// Copyright 2021 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GRPC_CORE_LIB_PROMISE_ACTIVITY_H
#define GRPC_CORE_LIB_PROMISE_ACTIVITY_H

#include <functional>
#include "absl/container/flat_hash_set.h"
#include "absl/container/inlined_vector.h"
#include "absl/status/status.h"
#include "absl/synchronization/mutex.h"
#include "src/core/lib/promise/adaptor.h"
#include "src/core/lib/promise/context.h"
#include "src/core/lib/promise/poll.h"

namespace grpc_core {

// A Wakeable object is used by queues to wake activities.
class Wakeable {
 public:
  // Wake up the underlying activity.
  // After calling, this Wakeable cannot be used again.
  virtual void Wakeup() = 0;
  // Drop this wakeable without waking up the underlying activity.
  virtual void Drop() = 0;
};

// An owning reference to a Wakeable.
// This type is non-copyable but movable.
class Waker {
 public:
  explicit Waker(Wakeable* wakeable) : wakeable_(wakeable) {}
  Waker() : wakeable_(&unwakeable_) {}
  ~Waker() { wakeable_->Drop(); }
  Waker(const Waker&) = delete;
  Waker& operator=(const Waker&) = delete;
  Waker(Waker&& other) noexcept : wakeable_(other.wakeable_) {
    other.wakeable_ = &unwakeable_;
  }
  Waker& operator=(Waker&& other) noexcept {
    std::swap(wakeable_, other.wakeable_);
    return *this;
  }

  // Wake the underlying activity.
  void Wakeup() {
    wakeable_->Wakeup();
    wakeable_ = &unwakeable_;
  }

  template <typename H>
  friend H AbslHashValue(H h, const Waker& w) {
    return H::combine(std::move(h), w.wakeable_);
  }

  bool operator==(const Waker& other) const noexcept {
    return wakeable_ == other.wakeable_;
  }

 private:
  class Unwakeable final : public Wakeable {
   public:
    void Wakeup() final { abort(); }
    void Drop() final {}
  };

  Wakeable* wakeable_;
  static Unwakeable unwakeable_;
};

// An Activity tracks execution of a single promise.
// It executes the promise under a mutex.
// When the promise stalls, it registers the containing activity to be woken up
// later.
// The activity takes a callback, which will be called exactly once with the
// result of execution.
// Activity execution may be cancelled by simply deleting the activity. In such
// a case, if execution had not already finished, the done callback would be
// called with absl::CancelledError().
// Activity also takes a CallbackScheduler instance on which to schedule
// callbacks to itself in a lock-clean environment.
class Activity : private Wakeable {
 public:
  // Cancel execution of the underlying promise.
  virtual void Cancel() LOCKS_EXCLUDED(mu_) = 0;

  // Destroy the Activity - used for the type alias ActivityPtr.
  struct Deleter {
    void operator()(Activity* activity) {
      activity->Cancel();
      activity->Unref();
    }
  };

  // Fetch the size of the implementation of this activity.
  virtual size_t Size() = 0;

  // Wakeup the current threads activity - will force a subsequent poll after
  // the one that's running.
  static void WakeupCurrent() { current()->got_wakeup_during_run_ = true; }

  // Return the current activity.
  // Additionally:
  // - assert that there is a current activity (and catch bugs if there's not)
  // - indicate to thread safety analysis that the current activity is indeed
  //   locked
  // - back up that assertation with a runtime check in debug builds (it's
  //   prohibitively expensive in non-debug builds)
  static Activity* current() ABSL_ASSERT_EXCLUSIVE_LOCK(current()->mu_) {
#ifndef NDEBUG
    assert(g_current_activity_);
    if (g_current_activity_ != nullptr) {
      g_current_activity_->mu_.AssertHeld();
    }
#endif
    return g_current_activity_;
  }

  // Produce an activity-owning Waker. The produced waker will keep the activity
  // alive until it's awoken or dropped.
  Waker MakeOwningWaker() {
    Ref();
    return Waker(this);
  }

  // Produce a non-owning Waker. The waker will own a small heap allocated weak
  // pointer to this activity. This is more suitable for wakeups that may not be
  // delivered until long after the activity should be destroyed.
  Waker MakeNonOwningWaker() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);

 protected:
  inline virtual ~Activity() {
    if (handle_) {
      DropHandle();
    }
  }

  // All promise execution occurs under this mutex.
  absl::Mutex mu_;

  // Check if this activity is the current activity executing on the current
  // thread.
  bool is_current() const { return this == g_current_activity_; }
  // Check if there is an activity executing on the current thread.
  static bool have_current() { return g_current_activity_ != nullptr; }
  // Check if we got an internal wakeup since the last time this function was
  // called.
  bool got_wakeup() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    return absl::exchange(got_wakeup_during_run_, false);
  }

  // Set the current activity at construction, clean it up at destruction.
  class ScopedActivity {
   public:
    explicit ScopedActivity(Activity* activity) {
      assert(g_current_activity_ == nullptr);
      g_current_activity_ = activity;
    }
    ~ScopedActivity() { g_current_activity_ = nullptr; }
    ScopedActivity(const ScopedActivity&) = delete;
    ScopedActivity& operator=(const ScopedActivity&) = delete;
  };

  // Implementors of Wakeable::Wakeup should call this after the wakeup has
  // completed.
  void WakeupComplete() { Unref(); }

 private:
  class Handle;

  void Ref() { refs_.fetch_add(1, std::memory_order_relaxed); }
  void Unref() {
    if (1 == refs_.fetch_sub(1, std::memory_order_acq_rel)) {
      delete this;
    }
  }

  // Return a Handle instance with a ref so that it can be stored waiting for
  // some wakeup.
  Handle* RefHandle() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_);
  // If our refcount is non-zero, ref and return true.
  // Otherwise, return false.
  bool RefIfNonZero();
  // Drop the (proved existing) wait handle.
  void DropHandle() EXCLUSIVE_LOCKS_REQUIRED(mu_);

  // Current refcount.
  std::atomic<uint32_t> refs_{1};
  // If wakeup is called during Promise polling, we raise this flag and repoll
  // until things settle out.
  bool got_wakeup_during_run_ ABSL_GUARDED_BY(mu_) = false;
  // Handle for long waits. Allows a very small weak pointer type object to
  // queue for wakeups while Activity may be deleted earlier.
  Handle* handle_ ABSL_GUARDED_BY(mu_) = nullptr;
  // Set during RunLoop to the Activity that's executing.
  // Being set implies that mu_ is held.
  static thread_local Activity* g_current_activity_;
};

// Owned pointer to one Activity.
using ActivityPtr = std::unique_ptr<Activity, Activity::Deleter>;

namespace activity_detail {

template <typename Context>
class ContextHolder {
 public:
  explicit ContextHolder(Context value) : value_(std::move(value)) {}
  Context* GetContext() { return &value_; }

 private:
  Context value_;
};

template <typename Context>
class ContextHolder<Context*> {
 public:
  explicit ContextHolder(Context* value) : value_(value) {}
  Context* GetContext() { return value_; }

 private:
  Context* value_;
};

template <typename... Contexts>
class EnterContexts : public promise_detail::Context<Contexts>... {
 public:
  explicit EnterContexts(Contexts*... contexts)
      : promise_detail::Context<Contexts>(contexts)... {}
};

// Implementation details for an Activity of an arbitrary type of promise.
template <class Factory, class CallbackScheduler, class OnDone,
          typename... Contexts>
class PromiseActivity final
    : public Activity,
      private activity_detail::ContextHolder<Contexts>... {
 public:
  PromiseActivity(Factory promise_factory, CallbackScheduler callback_scheduler,
                  OnDone on_done, Contexts... contexts)
      : Activity(),
        ContextHolder<Contexts>(std::move(contexts))...,
        callback_scheduler_(std::move(callback_scheduler)),
        on_done_(std::move(on_done)) {
    // Lock, construct an initial promise from the factory, and step it.
    // This may hit a waiter, which could expose our this pointer to other
    // threads, meaning we do need to hold this mutex even though we're still
    // constructing.
    mu_.Lock();
    auto status = Start(std::move(promise_factory));
    mu_.Unlock();
    // We may complete immediately.
    if (status.has_value()) {
      on_done_(std::move(*status));
    }
  }

  ~PromiseActivity() override {
    // We shouldn't destruct without calling Cancel() first, and that must get
    // us to be done_, so we assume that and have no logic to destruct the
    // promise here.
    assert(done_);
  }

  size_t Size() override { return sizeof(*this); }

  void Cancel() final {
    bool was_done;
    {
      absl::MutexLock lock(&mu_);
      // Check if we were done, and flag done.
      was_done = done_;
      if (!done_) MarkDone();
    }
    // If we were not done, then call the on_done callback.
    if (!was_done) {
      on_done_(absl::CancelledError());
    }
  }

 private:
  // Wakeup this activity. Arrange to poll the activity again at a convenient
  // time: this could be inline if it's deemed safe, or it could be by passing
  // the activity to an external threadpool to run. If the activity is already
  // running on this thread, a note is taken of such and the activity is
  // repolled if it doesn't complete.
  void Wakeup() final {
    // If there's no active activity, we can just run inline.
    if (!Activity::have_current()) {
      Step();
      WakeupComplete();
      return;
    }
    // If there is an active activity, but hey it's us, flag that and we'll loop
    // in RunLoop (that's calling from above here!).
    if (Activity::is_current()) {
      WakeupCurrent();
      WakeupComplete();
      return;
    }
    // Can't safely run, so ask to run later.
    callback_scheduler_([this]() {
      this->Step();
      this->WakeupComplete();
    });
  }

  // Drop a wakeup
  void Drop() final { this->WakeupComplete(); }

  // Notification that we're no longer executing - it's ok to destruct the
  // promise.
  void MarkDone() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    assert(!done_);
    done_ = true;
    Destruct(&promise_holder_.promise);
  }

  // In response to Wakeup, run the Promise state machine again until it
  // settles. Then check for completion, and if we have completed, call on_done.
  void Step() ABSL_LOCKS_EXCLUDED(mu_) {
    // Poll the promise until things settle out under a lock.
    mu_.Lock();
    if (done_) {
      // We might get some spurious wakeups after finishing.
      mu_.Unlock();
      return;
    }
    auto status = RunStep();
    mu_.Unlock();
    if (status.has_value()) {
      on_done_(std::move(*status));
    }
  }

  // The main body of a step: set the current activity, and any contexts, and
  // then run the main polling loop. Contained in a function by itself in order
  // to keep the scoping rules a little easier in Step().
  absl::optional<absl::Status> RunStep() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    ScopedActivity scoped_activity(this);
    EnterContexts<Contexts...> contexts(
        static_cast<ContextHolder<Contexts>*>(this)->GetContext()...);
    return StepLoop();
  }

  // Similarly to RunStep, but additionally construct the promise from a promise
  // factory before entering the main loop. Called once from the constructor.
  absl::optional<absl::Status> Start(Factory promise_factory)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    ScopedActivity scoped_activity(this);
    EnterContexts<Contexts...> contexts(
        static_cast<ContextHolder<Contexts>*>(this)->GetContext()...);
    Construct(&promise_holder_.promise, promise_factory());
    return StepLoop();
  }

  // Until there are no wakeups from within and the promise is incomplete: poll
  // the promise.
  absl::optional<absl::Status> StepLoop() ABSL_EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    assert(is_current());
    do {
      // Run the promise.
      assert(!done_);
      auto r = promise_holder_.promise();
      if (auto* status = r.get_ready()) {
        // If complete, destroy the promise, flag done, and exit this loop.
        MarkDone();
        return IntoStatus(status);
      }
      // Continue looping til no wakeups occur.
    } while (got_wakeup());
    return {};
  }

  using Promise = decltype(std::declval<Factory>()());
  // We wrap the promise in a union to allow control over the construction
  // simultaneously with annotating mutex requirements and noting that the
  // promise contained may not use any memory.
  union PromiseHolder {
    PromiseHolder() {}
    ~PromiseHolder() {}
    [[no_unique_address]] Promise promise;
  };
  [[no_unique_address]] PromiseHolder promise_holder_ ABSL_GUARDED_BY(mu_);
  // Schedule callbacks on some external executor.
  [[no_unique_address]] CallbackScheduler callback_scheduler_;
  // Callback on completion of the promise.
  [[no_unique_address]] OnDone on_done_;
  // Has execution completed?
  [[no_unique_address]] bool done_ ABSL_GUARDED_BY(mu_) = false;
};

}  // namespace activity_detail

// Given a functor that returns a promise (a promise factory), a callback for
// completion, and a callback scheduler, construct an activity.
template <typename Factory, typename CallbackScheduler, typename OnDone,
          typename... Contexts>
ActivityPtr MakeActivity(Factory promise_factory,
                         CallbackScheduler callback_scheduler, OnDone on_done,
                         Contexts... contexts) {
  return ActivityPtr(
      new activity_detail::PromiseActivity<Factory, CallbackScheduler, OnDone,
                                           Contexts...>(
          std::move(promise_factory), std::move(callback_scheduler),
          std::move(on_done), std::move(contexts)...));
}

// Helper type that can be used to enqueue many Activities waiting for some
// external state.
// Typically the external state should be guarded by mu_, and a call to
// WakeAllAndUnlock should be made when the state changes.
// Promises should bottom out polling inside pending(), which will register for
// wakeup and return kPending.
// Queues handles to Activities, and not Activities themselves, meaning that if
// an Activity is destroyed prior to wakeup we end up holding only a small
// amount of memory (around 16 bytes + malloc overhead) until the next wakeup
// occurs.
class WaitSet final {
  using WakerSet = absl::flat_hash_set<Waker>;

 public:
  // Register for wakeup, return kPending. If state is not ready to proceed,
  // Promises should bottom out here.
  Pending AddPending(Waker waker) {
    pending_.emplace(std::move(waker));
    return kPending;
  }

  class WakeupSet {
   public:
    void Wakeup() {
      while (!wakeup_.empty()) {
        wakeup_.extract(wakeup_.begin()).value().Wakeup();
      }
    }

   private:
    friend class WaitSet;
    explicit WakeupSet(WakerSet&& wakeup)
        : wakeup_(std::forward<WakerSet>(wakeup)) {}
    WakerSet wakeup_;
  };

  [[nodiscard]] WakeupSet TakeWakeupSet() {
    return WakeupSet(std::move(pending_));
  }

 private:
  // Handles to activities that need to be awoken.
  WakerSet pending_;
};

// Helper type to track wakeups between objects in the same activity.
// Can be fairly fast as no ref counting or locking needs to occur.
class IntraActivityWaiter {
 public:
  // Register for wakeup, return kPending. If state is not ready to proceed,
  // Promises should bottom out here.
  Pending pending() {
    waiting_ = true;
    return kPending;
  }
  // Wake the activity
  void Wake() {
    if (waiting_) {
      waiting_ = false;
      Activity::WakeupCurrent();
    }
  }

 private:
  bool waiting_ = false;
};

// A callback scheduler that simply crashes
struct NoCallbackScheduler {
  template <typename F>
  void operator()(F) {
    abort();
  }
};

}  // namespace grpc_core

#endif