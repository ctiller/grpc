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

#include "src/core/lib/promise/activity.h"

namespace grpc_core {

///////////////////////////////////////////////////////////////////////////////
// GLOBALS

thread_local Activity* Activity::g_current_activity_ = nullptr;

///////////////////////////////////////////////////////////////////////////////
// HELPER TYPES

// Weak handle to an Activity.
// Handle can persist while Activity goes away.
class Activity::Handle {
 public:
  explicit Handle(Activity* activity) : activity_(activity) {}

  // Ref the Handle (not the activity).
  void Ref() { refs_.fetch_add(1, std::memory_order_relaxed); }
  // Unref the Handle (not the activity).
  void Unref() {
    if (1 == refs_.fetch_sub(1, std::memory_order_acq_rel)) {
      delete this;
    }
  }

  // Activity is going away... drop its reference and sever the connection back.
  void DropActivity() {
    mu_.Lock();
    assert(activity_ != nullptr);
    activity_ = nullptr;
    mu_.Unlock();
    Unref();
  }

  // Activity needs to wake up (if it still exists!) - wake it up, and drop the
  // ref that was kept for this handle.
  void WakeupAndUnref() LOCKS_EXCLUDED(mu_, activity_->mu_) {
    mu_.Lock();
    // Note that activity refcount can drop to zero, but we could win the lock
    // against DropActivity, so we need to only increase activities refcount if
    // it is non-zero.
    if (activity_ && activity_->RefIfNonZero()) {
      Activity* activity = activity_;
      mu_.Unlock();
      // Activity still exists and we have a reference: wake it up, and then
      // drop the ref.
      activity->Wakeup();
      activity->Unref();
    } else {
      // Could not get the activity - it's either gone or going. No need to wake
      // it up!
      mu_.Unlock();
    }
    // Drop the ref to the handle (we have one ref = one wakeup semantics).
    Unref();
  }

 private:
  // Two initial refs: one for the waiter that caused instantiation, one for the
  // activity.
  std::atomic<size_t> refs_{2};
  absl::Mutex mu_ ACQUIRED_AFTER(activity_->mu_);
  Activity* activity_ GUARDED_BY(mu_);
};

///////////////////////////////////////////////////////////////////////////////
// ACTIVITY IMPLEMENTATION

bool Activity::RefIfNonZero() {
  auto value = refs_.load(std::memory_order_acquire);
  do {
    if (value == 0) {
      return false;
    }
  } while (!refs_.compare_exchange_weak(
      value, value + 1, std::memory_order_release, std::memory_order_relaxed));
  return true;
}

Activity::Handle* Activity::RefHandle() {
  if (handle_ == nullptr) {
    // No handle created yet - construct it and return it.
    handle_ = new Handle(this);
    return handle_;
  } else {
    // Already had to create a handle, ref & return it.
    handle_->Ref();
    return handle_;
  }
}

void Activity::DropHandle() {
  handle_->DropActivity();
  handle_ = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// WAITSET IMPLEMENTATION

WaitSet::WaitSet() = default;

WaitSet::~WaitSet() {
  for (auto* h : pending_) {
    h->WakeupAndUnref();
  }
}

Pending WaitSet::pending() {
  // Get a reffed handle to the current activity.
  Activity::Handle* h = Activity::g_current_activity_->RefHandle();
  // Insert it into our pending list.
  if (!pending_.insert(h).second) {
    // If it was already there, we can drop it immediately.
    h->Unref();
  }
  return kPending;
}

void WaitSet::WakeAllAndUnlock() {
  // Take the entire pending set.
  decltype(pending_) prior;
  prior.swap(pending_);
  // Unlock so we're not holding a mutex during wakeups
  mu_.Unlock();
  // Wakeup all the waiting activities!
  for (auto* h : prior) {
    h->WakeupAndUnref();
  }
}

///////////////////////////////////////////////////////////////////////////////
// SINGLEWAITER IMPLEMENTATION

SingleWaiter::SingleWaiter() : waiter_(nullptr) {}

SingleWaiter::~SingleWaiter() { Wakeup(waiter_); }

Pending SingleWaiter::pending() {
  assert(Activity::g_current_activity_);
  if (waiter_ == Activity::g_current_activity_) {
    return kPending;
  }
  if (waiter_ != nullptr) {
    waiter_->Unref();
  }
  waiter_ = Activity::g_current_activity_;
  waiter_->Ref();
  return kPending;
}

void SingleWaiter::WakeAndUnlock() {
  Activity* activity = waiter_;
  waiter_ = nullptr;
  mu_.Unlock();
  Wakeup(activity);
}

void SingleWaiter::Wakeup(Activity* activity) {
  if (activity == nullptr) {
    return;
  }
  activity->Wakeup();
  activity->Unref();
}

}  // namespace grpc_core
