//
//
// Copyright 2015 gRPC authors.
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
//
//

#include <grpc/support/port_platform.h>

#include "src/core/lib/channel/channel_args.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/match.h"
#include "absl/strings/str_join.h"

#include <grpc/impl/channel_arg_names.h>
#include <grpc/support/alloc.h>
#include <grpc/support/log.h>
#include <grpc/support/string_util.h>

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/gpr/useful.h"

namespace grpc_core {

namespace {
const ChannelArgs::IntKey kMinimalStackKey = ChannelArgs::IntKey::Register(
    GRPC_ARG_MINIMAL_STACK, ChannelArgs::KeyOptions{});
}

ChannelArgs::Pointer::Pointer(void* p, const grpc_arg_pointer_vtable* vtable)
    : p_(p), vtable_(vtable == nullptr ? EmptyVTable() : vtable) {}

ChannelArgs::Pointer::Pointer(const Pointer& other)
    : p_(other.vtable_->copy(other.p_)), vtable_(other.vtable_) {}

ChannelArgs::Pointer::Pointer(Pointer&& other) noexcept
    : p_(other.p_), vtable_(other.vtable_) {
  other.p_ = nullptr;
  other.vtable_ = EmptyVTable();
}

const grpc_arg_pointer_vtable* ChannelArgs::Pointer::EmptyVTable() {
  static const grpc_arg_pointer_vtable vtable = {
      // copy
      [](void* p) { return p; },
      // destroy
      [](void*) {},
      // cmp
      [](void* p1, void* p2) -> int { return QsortCompare(p1, p2); },
  };
  return &vtable;
}

ChannelArgs::ChannelArgs() = default;
ChannelArgs::~ChannelArgs() = default;
ChannelArgs::ChannelArgs(const ChannelArgs& other) = default;
ChannelArgs& ChannelArgs::operator=(const ChannelArgs& other) = default;
ChannelArgs::ChannelArgs(ChannelArgs&& other) noexcept = default;
ChannelArgs& ChannelArgs::operator=(ChannelArgs&& other) noexcept = default;

bool ChannelArgs::Contains(IntKey name) const {
  return bins_[name.bin()].ints.Lookup(name) != nullptr;
}

bool ChannelArgs::Contains(StringKey name) const {
  return bins_[name.bin()].strings.Lookup(name) != nullptr;
}

bool ChannelArgs::Contains(PointerKey name) const {
  return bins_[name.bin()].pointers.Lookup(name) != nullptr;
}

bool ChannelArgs::Bin::operator<(const ChannelArgs::Bin& other) const {
  if (ints < other.ints) return true;
  if (other.ints < ints) return false;
  if (strings < other.strings) return true;
  if (other.strings < strings) return false;
  return pointers < other.pointers;
}

bool ChannelArgs::Bin::operator==(const ChannelArgs::Bin& other) const {
  return ints == other.ints && strings == other.strings &&
         pointers == other.pointers;
}

bool ChannelArgs::operator<(const ChannelArgs& other) const {
  for (size_t i = 0; i < kBins; i++) {
    if (bins_[i] < other.bins_[i]) return true;
    if (other.bins_[i] < bins_[i]) return false;
  }
  return false;
}

bool ChannelArgs::operator==(const ChannelArgs& other) const {
  for (size_t i = 0; i < kBins; i++) {
    if (bins_[i] != other.bins_[i]) return false;
  }
  return true;
}

bool ChannelArgs::operator!=(const ChannelArgs& other) const {
  return !(*this == other);
}

bool ChannelArgs::WantMinimalStack() const {
  return GetBool(kMinimalStackKey).value_or(false);
}

ChannelArgs::ChannelArgs(Bins bins) : bins_(std::move(bins)) {}

ChannelArgs ChannelArgs::Set(grpc_arg arg, bool override) const {
  switch (arg.type) {
    case GRPC_ARG_INTEGER: {
      auto key = IntKey::LookupRegistered(arg.key);
      if (!key.has_value()) {
        gpr_log(GPR_ERROR, "Ignoring unknown integer arg key '%s'", arg.key);
        return *this;
      }
      return override ? Set(*key, arg.value.integer)
                      : SetIfUnset(*key, arg.value.integer);
    }
    case GRPC_ARG_STRING: {
      auto key = StringKey::LookupRegistered(arg.key);
      if (!key.has_value()) {
        gpr_log(GPR_ERROR, "Ignoring unknown string arg key '%s'", arg.key);
        return *this;
      }
      if (arg.value.string != nullptr) {
        return override ? Set(*key, arg.value.string)
                        : SetIfUnset(*key, arg.value.string);
      } else if (override) {
        return Remove(*key);
      }
    }
    case GRPC_ARG_POINTER: {
      auto key = PointerKey::LookupRegistered(arg.key);
      if (!key.has_value()) {
        gpr_log(GPR_ERROR, "Ignoring unknown string arg key '%s'", arg.key);
        return *this;
      }
      Pointer p(arg.value.pointer.vtable->copy(arg.value.pointer.p),
                arg.value.pointer.vtable);
      return override ? Set(*key, std::move(p))
                      : SetIfUnset(*key, std::move(p));
    }
  }
  GPR_UNREACHABLE_CODE(return ChannelArgs());
}

ChannelArgs ChannelArgs::FromC(const grpc_channel_args* args) {
  ChannelArgs result;
  if (args != nullptr) {
    for (size_t i = 0; i < args->num_args; i++) {
      result = result.Set(args->args[i]);
    }
  }
  return result;
}

/*
grpc_arg ChannelArgs::Value::MakeCArg(const char* name) const {
  char* c_name = const_cast<char*>(name);
  if (rep_.c_vtable() == &int_vtable_) {
    return grpc_channel_arg_integer_create(
        c_name, reinterpret_cast<intptr_t>(rep_.c_pointer()));
  }
  if (rep_.c_vtable() == &string_vtable_) {
    return grpc_channel_arg_string_create(
        c_name, const_cast<char*>(
                    static_cast<RefCountedString*>(rep_.c_pointer())->c_str()));
  }
  return grpc_channel_arg_pointer_create(c_name, rep_.c_pointer(),
                                         rep_.c_vtable());
}

ChannelArgs::CPtr ChannelArgs::ToC() const {
  std::vector<grpc_arg> c_args;
  args_.ForEach(
      [&c_args](const RefCountedStringValue& key, const Value& value) {
        c_args.push_back(value.MakeCArg(key.c_str()));
      });
  return CPtr(static_cast<const grpc_channel_args*>(
      grpc_channel_args_copy_and_add(nullptr, c_args.data(), c_args.size())));
}
*/

ChannelArgs ChannelArgs::Set(PointerKey name, Pointer value) const {
  auto* p = bins_[name.bin()].pointers.Lookup(name);
  if (p != nullptr && *p == value) return *this;  // already have this value
  Bins new_bins = bins_;
  new_bins[name.bin()].pointers =
      new_bins[name.bin()].pointers.Add(name, value);
  return ChannelArgs(std::move(new_bins));
}

ChannelArgs ChannelArgs::Set(IntKey name, int value) const {
  auto* p = bins_[name.bin()].ints.Lookup(name);
  if (p != nullptr && *p == value) return *this;  // already have this value
  Bins new_bins = bins_;
  new_bins[name.bin()].ints = new_bins[name.bin()].ints.Add(name, value);
  return ChannelArgs(std::move(new_bins));
}

ChannelArgs ChannelArgs::Set(StringKey name, absl::string_view value) const {
  return Set(name, std::string(value));
}

ChannelArgs ChannelArgs::Set(StringKey name, const char* value) const {
  return Set(name, std::string(value));
}

ChannelArgs ChannelArgs::Set(StringKey name, std::string value) const {
  auto* p = bins_[name.bin()].strings.Lookup(name);
  if (p != nullptr && p->as_string_view() == value) {
    return *this;  // already have this value
  }
  Bins new_bins = bins_;
  new_bins[name.bin()].strings =
      new_bins[name.bin()].strings.Add(name, RefCountedStringValue(value));
  return ChannelArgs(std::move(new_bins));
}

ChannelArgs ChannelArgs::Remove(IntKey name) const {
  if (bins_[name.bin()].ints.Lookup(name) == nullptr) return *this;
  Bins new_bins = bins_;
  new_bins[name.bin()].ints = new_bins[name.bin()].ints.Remove(name);
  return ChannelArgs(std::move(new_bins));
}

ChannelArgs ChannelArgs::Remove(StringKey name) const {
  if (bins_[name.bin()].strings.Lookup(name) == nullptr) return *this;
  Bins new_bins = bins_;
  new_bins[name.bin()].strings = new_bins[name.bin()].strings.Remove(name);
  return ChannelArgs(std::move(new_bins));
}

ChannelArgs ChannelArgs::Remove(PointerKey name) const {
  if (bins_[name.bin()].pointers.Lookup(name) == nullptr) return *this;
  Bins new_bins = bins_;
  new_bins[name.bin()].pointers = new_bins[name.bin()].pointers.Remove(name);
  return ChannelArgs(std::move(new_bins));
}

absl::optional<int> ChannelArgs::GetInt(IntKey name) const {
  auto* v = bins_[name.bin()].ints.Lookup(name);
  if (v == nullptr) return absl::nullopt;
  return *v;
}

absl::optional<Duration> ChannelArgs::GetDurationFromIntMillis(
    IntKey name) const {
  auto ms = GetInt(name);
  if (!ms.has_value()) return absl::nullopt;
  if (*ms == INT_MAX) return Duration::Infinity();
  if (*ms == INT_MIN) return Duration::NegativeInfinity();
  return Duration::Milliseconds(*ms);
}

absl::optional<absl::string_view> ChannelArgs::GetString(StringKey name) const {
  auto* v = bins_[name.bin()].strings.Lookup(name);
  if (v == nullptr) return absl::nullopt;
  return v->as_string_view();
}

absl::optional<std::string> ChannelArgs::GetOwnedString(StringKey name) const {
  absl::optional<absl::string_view> v = GetString(name);
  if (!v.has_value()) return absl::nullopt;
  return std::string(*v);
}

void* ChannelArgs::GetVoidPointer(PointerKey name) const {
  auto* v = bins_[name.bin()].pointers.Lookup(name);
  if (v == nullptr) return nullptr;
  return v->c_pointer();
}

absl::optional<bool> ChannelArgs::GetBool(IntKey name) const {
  auto v = GetInt(name);
  if (!v.has_value()) return absl::nullopt;
  switch (*v) {
    case 0:
      return false;
    case 1:
      return true;
    default:
      gpr_log(GPR_ERROR, "%s treated as bool but set to %d (assuming true)",
              std::string(name.name()).c_str(), *v);
      return true;
  }
}

/*
std::string ChannelArgs::Value::ToString() const {
  if (rep_.c_vtable() == &int_vtable_) {
    return std::to_string(reinterpret_cast<intptr_t>(rep_.c_pointer()));
  }
  if (rep_.c_vtable() == &string_vtable_) {
    return std::string(
        static_cast<RefCountedString*>(rep_.c_pointer())->as_string_view());
  }
  return absl::StrFormat("%p", rep_.c_pointer());
}

std::string ChannelArgs::ToString() const {
  std::vector<std::string> arg_strings;
  args_.ForEach(
      [&arg_strings](const RefCountedStringValue& key, const Value& value) {
        arg_strings.push_back(
            absl::StrCat(key.as_string_view(), "=", value.ToString()));
      });
  return absl::StrCat("{", absl::StrJoin(arg_strings, ", "), "}");
}
*/

ChannelArgs ChannelArgs::UnionWith(ChannelArgs other) const {
  ChannelArgs out;
  for (size_t i = 0; i < kBins; i++) {
    out.bins_[i].ints = bins_[i].ints.UnionWith(other.bins_[i].ints);
    out.bins_[i].strings = bins_[i].strings.UnionWith(other.bins_[i].strings);
    out.bins_[i].pointers =
        bins_[i].pointers.UnionWith(other.bins_[i].pointers);
  }
  return out;
}

void ChannelArgs::ChannelArgsDeleter::operator()(
    const grpc_channel_args* p) const {
  grpc_channel_args_destroy(p);
}

std::ostream& operator<<(std::ostream& out, const ChannelArgs& args) {
  return out << args.ToString();
}

}  // namespace grpc_core

static grpc_arg copy_arg(const grpc_arg* src) {
  grpc_arg dst;
  dst.type = src->type;
  dst.key = gpr_strdup(src->key);
  switch (dst.type) {
    case GRPC_ARG_STRING:
      dst.value.string = gpr_strdup(src->value.string);
      break;
    case GRPC_ARG_INTEGER:
      dst.value.integer = src->value.integer;
      break;
    case GRPC_ARG_POINTER:
      dst.value.pointer = src->value.pointer;
      dst.value.pointer.p =
          src->value.pointer.vtable->copy(src->value.pointer.p);
      break;
  }
  return dst;
}

grpc_channel_args* grpc_channel_args_copy_and_add(const grpc_channel_args* src,
                                                  const grpc_arg* to_add,
                                                  size_t num_to_add) {
  return grpc_channel_args_copy_and_add_and_remove(src, nullptr, 0, to_add,
                                                   num_to_add);
}

grpc_channel_args* grpc_channel_args_copy_and_remove(
    const grpc_channel_args* src, const char** to_remove,
    size_t num_to_remove) {
  return grpc_channel_args_copy_and_add_and_remove(src, to_remove,
                                                   num_to_remove, nullptr, 0);
}

static bool should_remove_arg(const grpc_arg* arg, const char** to_remove,
                              size_t num_to_remove) {
  for (size_t i = 0; i < num_to_remove; ++i) {
    if (strcmp(arg->key, to_remove[i]) == 0) return true;
  }
  return false;
}

grpc_channel_args* grpc_channel_args_copy_and_add_and_remove(
    const grpc_channel_args* src, const char** to_remove, size_t num_to_remove,
    const grpc_arg* to_add, size_t num_to_add) {
  // Figure out how many args we'll be copying.
  size_t num_args_to_copy = 0;
  if (src != nullptr) {
    for (size_t i = 0; i < src->num_args; ++i) {
      if (!should_remove_arg(&src->args[i], to_remove, num_to_remove)) {
        ++num_args_to_copy;
      }
    }
  }
  // Create result.
  grpc_channel_args* dst =
      static_cast<grpc_channel_args*>(gpr_malloc(sizeof(grpc_channel_args)));
  dst->num_args = num_args_to_copy + num_to_add;
  if (dst->num_args == 0) {
    dst->args = nullptr;
    return dst;
  }
  dst->args =
      static_cast<grpc_arg*>(gpr_malloc(sizeof(grpc_arg) * dst->num_args));
  // Copy args from src that are not being removed.
  size_t dst_idx = 0;
  if (src != nullptr) {
    for (size_t i = 0; i < src->num_args; ++i) {
      if (!should_remove_arg(&src->args[i], to_remove, num_to_remove)) {
        dst->args[dst_idx++] = copy_arg(&src->args[i]);
      }
    }
  }
  // Add args from to_add.
  for (size_t i = 0; i < num_to_add; ++i) {
    dst->args[dst_idx++] = copy_arg(&to_add[i]);
  }
  GPR_ASSERT(dst_idx == dst->num_args);
  return dst;
}

grpc_channel_args* grpc_channel_args_copy(const grpc_channel_args* src) {
  return grpc_channel_args_copy_and_add(src, nullptr, 0);
}

grpc_channel_args* grpc_channel_args_union(const grpc_channel_args* a,
                                           const grpc_channel_args* b) {
  if (a == nullptr) return grpc_channel_args_copy(b);
  if (b == nullptr) return grpc_channel_args_copy(a);
  const size_t max_out = (a->num_args + b->num_args);
  grpc_arg* uniques =
      static_cast<grpc_arg*>(gpr_malloc(sizeof(*uniques) * max_out));
  for (size_t i = 0; i < a->num_args; ++i) uniques[i] = a->args[i];

  size_t uniques_idx = a->num_args;
  for (size_t i = 0; i < b->num_args; ++i) {
    const char* b_key = b->args[i].key;
    if (grpc_channel_args_find(a, b_key) == nullptr) {  // not found
      uniques[uniques_idx++] = b->args[i];
    }
  }
  grpc_channel_args* result =
      grpc_channel_args_copy_and_add(nullptr, uniques, uniques_idx);
  gpr_free(uniques);
  return result;
}

static int cmp_arg(const grpc_arg* a, const grpc_arg* b) {
  int c = grpc_core::QsortCompare(a->type, b->type);
  if (c != 0) return c;
  c = strcmp(a->key, b->key);
  if (c != 0) return c;
  switch (a->type) {
    case GRPC_ARG_STRING:
      return strcmp(a->value.string, b->value.string);
    case GRPC_ARG_INTEGER:
      return grpc_core::QsortCompare(a->value.integer, b->value.integer);
    case GRPC_ARG_POINTER:
      return grpc_core::channel_args_detail::PointerCompare(
          a->value.pointer.p, a->value.pointer.vtable, b->value.pointer.p,
          b->value.pointer.vtable);
  }
  GPR_UNREACHABLE_CODE(return 0);
}

// stabilizing comparison function: since channel_args ordering matters for
// keys with the same name, we need to preserve that ordering
static int cmp_key_stable(const void* ap, const void* bp) {
  const grpc_arg* const* a = static_cast<const grpc_arg* const*>(ap);
  const grpc_arg* const* b = static_cast<const grpc_arg* const*>(bp);
  int c = strcmp((*a)->key, (*b)->key);
  if (c == 0) c = grpc_core::QsortCompare(*a, *b);
  return c;
}

grpc_channel_args* grpc_channel_args_normalize(const grpc_channel_args* src) {
  grpc_arg** args =
      static_cast<grpc_arg**>(gpr_malloc(sizeof(grpc_arg*) * src->num_args));
  for (size_t i = 0; i < src->num_args; i++) {
    args[i] = &src->args[i];
  }
  if (src->num_args > 1) {
    qsort(args, src->num_args, sizeof(grpc_arg*), cmp_key_stable);
  }

  grpc_channel_args* b =
      static_cast<grpc_channel_args*>(gpr_malloc(sizeof(grpc_channel_args)));
  b->num_args = src->num_args;
  b->args = static_cast<grpc_arg*>(gpr_malloc(sizeof(grpc_arg) * b->num_args));
  for (size_t i = 0; i < src->num_args; i++) {
    b->args[i] = copy_arg(args[i]);
  }

  gpr_free(args);
  return b;
}

void grpc_channel_args_destroy(grpc_channel_args* a) {
  size_t i;
  if (!a) return;
  for (i = 0; i < a->num_args; i++) {
    switch (a->args[i].type) {
      case GRPC_ARG_STRING:
        gpr_free(a->args[i].value.string);
        break;
      case GRPC_ARG_INTEGER:
        break;
      case GRPC_ARG_POINTER:
        a->args[i].value.pointer.vtable->destroy(a->args[i].value.pointer.p);
        break;
    }
    gpr_free(a->args[i].key);
  }
  gpr_free(a->args);
  gpr_free(a);
}

int grpc_channel_args_compare(const grpc_channel_args* a,
                              const grpc_channel_args* b) {
  if (a == nullptr && b == nullptr) return 0;
  if (a == nullptr || b == nullptr) return a == nullptr ? -1 : 1;
  int c = grpc_core::QsortCompare(a->num_args, b->num_args);
  if (c != 0) return c;
  for (size_t i = 0; i < a->num_args; i++) {
    c = cmp_arg(&a->args[i], &b->args[i]);
    if (c != 0) return c;
  }
  return 0;
}

const grpc_arg* grpc_channel_args_find(const grpc_channel_args* args,
                                       const char* name) {
  if (args != nullptr) {
    for (size_t i = 0; i < args->num_args; ++i) {
      if (strcmp(args->args[i].key, name) == 0) {
        return &args->args[i];
      }
    }
  }
  return nullptr;
}

int grpc_channel_arg_get_integer(const grpc_arg* arg,
                                 const grpc_integer_options options) {
  if (arg == nullptr) return options.default_value;
  if (arg->type != GRPC_ARG_INTEGER) {
    gpr_log(GPR_ERROR, "%s ignored: it must be an integer", arg->key);
    return options.default_value;
  }
  if (arg->value.integer < options.min_value) {
    gpr_log(GPR_ERROR, "%s ignored: it must be >= %d", arg->key,
            options.min_value);
    return options.default_value;
  }
  if (arg->value.integer > options.max_value) {
    gpr_log(GPR_ERROR, "%s ignored: it must be <= %d", arg->key,
            options.max_value);
    return options.default_value;
  }
  return arg->value.integer;
}

int grpc_channel_args_find_integer(const grpc_channel_args* args,
                                   const char* name,
                                   const grpc_integer_options options) {
  const grpc_arg* arg = grpc_channel_args_find(args, name);
  return grpc_channel_arg_get_integer(arg, options);
}

char* grpc_channel_arg_get_string(const grpc_arg* arg) {
  if (arg == nullptr) return nullptr;
  if (arg->type != GRPC_ARG_STRING) {
    gpr_log(GPR_ERROR, "%s ignored: it must be an string", arg->key);
    return nullptr;
  }
  return arg->value.string;
}

char* grpc_channel_args_find_string(const grpc_channel_args* args,
                                    const char* name) {
  const grpc_arg* arg = grpc_channel_args_find(args, name);
  return grpc_channel_arg_get_string(arg);
}

bool grpc_channel_arg_get_bool(const grpc_arg* arg, bool default_value) {
  if (arg == nullptr) return default_value;
  if (arg->type != GRPC_ARG_INTEGER) {
    gpr_log(GPR_ERROR, "%s ignored: it must be an integer", arg->key);
    return default_value;
  }
  switch (arg->value.integer) {
    case 0:
      return false;
    case 1:
      return true;
    default:
      gpr_log(GPR_ERROR, "%s treated as bool but set to %d (assuming true)",
              arg->key, arg->value.integer);
      return true;
  }
}

bool grpc_channel_args_find_bool(const grpc_channel_args* args,
                                 const char* name, bool default_value) {
  const grpc_arg* arg = grpc_channel_args_find(args, name);
  return grpc_channel_arg_get_bool(arg, default_value);
}

bool grpc_channel_args_want_minimal_stack(const grpc_channel_args* args) {
  return grpc_channel_arg_get_bool(
      grpc_channel_args_find(args, GRPC_ARG_MINIMAL_STACK), false);
}

grpc_arg grpc_channel_arg_string_create(char* name, char* value) {
  grpc_arg arg;
  arg.type = GRPC_ARG_STRING;
  arg.key = name;
  arg.value.string = value;
  return arg;
}

grpc_arg grpc_channel_arg_integer_create(char* name, int value) {
  grpc_arg arg;
  arg.type = GRPC_ARG_INTEGER;
  arg.key = name;
  arg.value.integer = value;
  return arg;
}

grpc_arg grpc_channel_arg_pointer_create(
    char* name, void* value, const grpc_arg_pointer_vtable* vtable) {
  grpc_arg arg;
  arg.type = GRPC_ARG_POINTER;
  arg.key = name;
  arg.value.pointer.p = value;
  arg.value.pointer.vtable = vtable;
  return arg;
}

std::string grpc_channel_args_string(const grpc_channel_args* args) {
  return grpc_core::ChannelArgs::FromC(args).ToString();
}

namespace grpc_core {
ChannelArgs ChannelArgsBuiltinPrecondition(const grpc_channel_args* src) {
  if (src == nullptr) return ChannelArgs();
  ChannelArgs output;
  std::map<absl::string_view, std::vector<absl::string_view>>
      concatenated_values;
  for (size_t i = 0; i < src->num_args; i++) {
    absl::string_view key = src->args[i].key;
    // User-agent strings were traditionally multi-valued and concatenated.
    // We preserve this behavior for backwards compatibility.
    if (key == GRPC_ARG_PRIMARY_USER_AGENT_STRING ||
        key == GRPC_ARG_SECONDARY_USER_AGENT_STRING) {
      if (src->args[i].type != GRPC_ARG_STRING) {
        gpr_log(GPR_ERROR, "Channel argument '%s' should be a string",
                std::string(key).c_str());
      } else {
        concatenated_values[key].push_back(src->args[i].value.string);
      }
      continue;
    } else if (absl::StartsWith(key, "grpc.internal.")) {
      continue;
    }
    // Traditional grpc_channel_args_find behavior was to pick the first
    // value.
    // For compatibility with existing users, we will do the same here.
    output = output.Set(src->args[i], false);
  }
  // Concatenate the concatenated values.
  for (const auto& concatenated_value : concatenated_values) {
    const auto key =
        *ChannelArgs::StringKey::LookupRegistered(concatenated_value.first);
    output = output.Set(key, absl::StrJoin(concatenated_value.second, " "));
  }
  return output;
}

}  // namespace grpc_core

namespace {
grpc_channel_args_client_channel_creation_mutator g_mutator = nullptr;
}  // namespace

void grpc_channel_args_set_client_channel_creation_mutator(
    grpc_channel_args_client_channel_creation_mutator cb) {
  GPR_DEBUG_ASSERT(g_mutator == nullptr);
  g_mutator = cb;
}
grpc_channel_args_client_channel_creation_mutator
grpc_channel_args_get_client_channel_creation_mutator() {
  return g_mutator;
}
