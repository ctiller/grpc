//
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
//

#include <grpc/support/port_platform.h>

#include "src/core/ext/filters/rbac/rbac_service_config_parser.h"

#include <cstdint>
#include <map>
#include <string>

#include "absl/memory/memory.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/optional.h"

#include "src/core/lib/channel/channel_args.h"
#include "src/core/lib/json/json_args.h"
#include "src/core/lib/json/json_object_loader.h"
#include "src/core/lib/matchers/matchers.h"

namespace grpc_core {

namespace {

struct RbacConfig {
  struct RbacPolicy {
    struct Rules {
      struct Policy {
        struct CidrRange {
          Rbac::CidrRange cidr_range;

          static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
            // All fields handled in JsonPostLoad().
            static const auto* loader = JsonObjectLoader<CidrRange>().Finish();
            return loader;
          }

          void JsonPostLoad(const Json& json, const JsonArgs& args,
                            ValidationErrors* errors) {
            auto address_prefix = LoadJsonObjectField<std::string>(
                json.object_value(), args, "addressPrefix", errors);
            auto prefix_len = LoadJsonObjectField<uint32_t>(
                json.object_value(), args, "prefixLen", errors,
                /*required=*/false);
            cidr_range = Rbac::CidrRange(address_prefix.value_or(""),
                                         prefix_len.value_or(0));
          }
        };

        struct SafeRegexMatch {
          std::string regex;

          static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
            static const auto* loader =
                JsonObjectLoader<SafeRegexMatch>()
                    .Field("regex", &SafeRegexMatch::regex)
                    .Finish();
            return loader;
          }
        };

        struct HeaderMatch {
          struct RangeMatch {
            int64_t start;
            int64_t end;

            static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
              static const auto* loader =
                  JsonObjectLoader<RangeMatch>()
                      .Field("start", &RangeMatch::start)
                      .Field("end", &RangeMatch::end)
                      .Finish();
              return loader;
            }
          };

          HeaderMatcher matcher;

          static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
            // All fields handled in JsonPostLoad().
            static const auto* loader =
                JsonObjectLoader<HeaderMatch>().Finish();
            return loader;
          }

          void JsonPostLoad(const Json& json, const JsonArgs& args,
                            ValidationErrors* errors) {
            const size_t original_error_size = errors->size();
            std::string name = LoadJsonObjectField<std::string>(
                                   json.object_value(), args, "name", errors)
                                   .value_or("");
            bool invert_match =
                LoadJsonObjectField<bool>(json.object_value(), args,
                                          "invertMatch", errors,
                                          /*required=*/false)
                    .value_or(false);
            auto set_header_matcher =
                [&](absl::StatusOr<HeaderMatcher> header_matcher) {
                  if (header_matcher.ok()) {
                    matcher = *header_matcher;
                  } else {
                    errors->AddError(header_matcher.status().message());
                  }
                };
            auto check_match = [&](absl::string_view field_name,
                                   HeaderMatcher::Type type) {
              auto match = LoadJsonObjectField<std::string>(
                  json.object_value(), args, field_name, errors,
                  /*required=*/false);
              if (match.has_value()) {
                set_header_matcher(HeaderMatcher::Create(
                    name, type, *match, 0, 0, false, invert_match));
                return true;
              }
              return false;
            };
            if (check_match("exactMatch", HeaderMatcher::Type::kExact) ||
                check_match("prefixMatch", HeaderMatcher::Type::kPrefix) ||
                check_match("suffixMatch", HeaderMatcher::Type::kSuffix) ||
                check_match("containsMatch", HeaderMatcher::Type::kContains)) {
              return;
            }
            auto present_match = LoadJsonObjectField<bool>(
                json.object_value(), args, "presentMatch", errors,
                /*required=*/false);
            if (present_match.has_value()) {
              set_header_matcher(
                  HeaderMatcher::Create(name, HeaderMatcher::Type::kPresent, "",
                                        0, 0, *present_match, invert_match));
              return;
            }
            auto regex_match = LoadJsonObjectField<SafeRegexMatch>(
                json.object_value(), args, "safeRegexMatch", errors,
                /*required=*/false);
            if (regex_match.has_value()) {
              set_header_matcher(HeaderMatcher::Create(
                  name, HeaderMatcher::Type::kSafeRegex, regex_match->regex, 0,
                  0, false, invert_match));
              return;
            }
            auto range_match = LoadJsonObjectField<RangeMatch>(
                json.object_value(), args, "rangeMatch", errors,
                /*required=*/false);
            if (range_match.has_value()) {
              set_header_matcher(HeaderMatcher::Create(
                  name, HeaderMatcher::Type::kRange, "", range_match->start,
                  range_match->end, invert_match));
              return;
            }
            if (errors->size() == original_error_size) {
              errors->AddError("no valid matcher found");
            }
          }
        };

        struct StringMatch {
          StringMatcher matcher;

          static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
            // All fields handled in JsonPostLoad().
            static const auto* loader =
                JsonObjectLoader<StringMatch>().Finish();
            return loader;
          }

          void JsonPostLoad(const Json& json, const JsonArgs& args,
                            ValidationErrors* errors) {
            const size_t original_error_size = errors->size();
            bool ignore_case =
                LoadJsonObjectField<bool>(json.object_value(), args,
                                          "ignoreCase", errors,
                                          /*required=*/false)
                    .value_or(false);
            auto set_string_matcher =
                [&](absl::StatusOr<StringMatcher> string_matcher) {
                  if (string_matcher.ok()) {
                    matcher = *string_matcher;
                  } else {
                    errors->AddError(string_matcher.status().message());
                  }
                };
            auto check_match = [&](absl::string_view field_name,
                                   StringMatcher::Type type) {
              auto match = LoadJsonObjectField<std::string>(
                  json.object_value(), args, field_name, errors,
                  /*required=*/false);
              if (match.has_value()) {
                set_string_matcher(
                    StringMatcher::Create(type, *match, ignore_case));
                return true;
              }
              return false;
            };
            if (check_match("exact", StringMatcher::Type::kExact) ||
                check_match("prefix", StringMatcher::Type::kPrefix) ||
                check_match("suffix", StringMatcher::Type::kSuffix) ||
                check_match("contains", StringMatcher::Type::kContains)) {
              return;
            }
            auto regex_match = LoadJsonObjectField<SafeRegexMatch>(
                json.object_value(), args, "safeRegex", errors,
                /*required=*/false);
            if (regex_match.has_value()) {
              set_string_matcher(
                  StringMatcher::Create(StringMatcher::Type::kSafeRegex,
                                        regex_match->regex, ignore_case));
              return;
            }
            if (errors->size() == original_error_size) {
              errors->AddError("no valid matcher found");
            }
          }
        };

        struct PathMatch {
          StringMatch path;

          static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
            static const auto* loader = JsonObjectLoader<PathMatch>()
                                            .Field("path", &PathMatch::path)
                                            .Finish();
            return loader;
          }
        };

        struct Metadata {
          bool invert = false;

          static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
            static const auto* loader =
                JsonObjectLoader<Metadata>()
                    .OptionalField("invert", &Metadata::invert)
                    .Finish();
            return loader;
          }
        };

        struct Permission {
          struct PermissionList {
            std::vector<Permission> rules;

            static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
              static const auto* loader =
                  JsonObjectLoader<PermissionList>()
                      .Field("rules", &PermissionList::rules)
                      .Finish();
              return loader;
            }
          };

          Rbac::Permission permission;

          // Work around for MSVC bug
          // https://developercommunity.visualstudio.com/t/C2280-when-modifying-a-vector-containing/377449
          Permission() = default;
          Permission(const Permission&) = delete;
          Permission(Permission&& other) noexcept = default;

          static std::vector<std::unique_ptr<Rbac::Permission>>
          MakeRbacPermissionList(std::vector<Permission>&& permission_list) {
            std::vector<std::unique_ptr<Rbac::Permission>> permissions;
            for (auto& rule : permission_list) {
              permissions.emplace_back(absl::make_unique<Rbac::Permission>(
                  std::move(rule.permission)));
            }
            return permissions;
          };

          static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
            // All fields handled in JsonPostLoad().
            static const auto* loader = JsonObjectLoader<Permission>().Finish();
            return loader;
          }

          void JsonPostLoad(const Json& json, const JsonArgs& args,
                            ValidationErrors* errors) {
            const size_t original_error_size = errors->size();
            auto any = LoadJsonObjectField<bool>(
                json.object_value(), args, "any", errors, /*required=*/false);
            if (any.has_value()) {
              permission = Rbac::Permission::MakeAnyPermission();
              return;
            }
            auto header = LoadJsonObjectField<HeaderMatch>(
                json.object_value(), args, "header", errors,
                /*required=*/false);
            if (header.has_value()) {
              permission =
                  Rbac::Permission::MakeHeaderPermission(header->matcher);
              return;
            }
            auto url_path = LoadJsonObjectField<PathMatch>(
                json.object_value(), args, "urlPath", errors,
                /*required=*/false);
            if (url_path.has_value()) {
              permission =
                  Rbac::Permission::MakePathPermission(url_path->path.matcher);
              return;
            }
            auto destination_ip = LoadJsonObjectField<CidrRange>(
                json.object_value(), args, "destinationIp", errors,
                /*required=*/false);
            if (destination_ip.has_value()) {
              permission = Rbac::Permission::MakeDestIpPermission(
                  std::move(destination_ip->cidr_range));
              return;
            }
            auto destination_port = LoadJsonObjectField<uint32_t>(
                json.object_value(), args, "destinationPort", errors,
                /*required=*/false);
            if (destination_port.has_value()) {
              permission =
                  Rbac::Permission::MakeDestPortPermission(*destination_port);
              return;
            }
            auto metadata = LoadJsonObjectField<Metadata>(
                json.object_value(), args, "metadata", errors,
                /*required=*/false);
            if (metadata.has_value()) {
              permission =
                  Rbac::Permission::MakeMetadataPermission(metadata->invert);
              return;
            }
            auto requested_server_name = LoadJsonObjectField<StringMatch>(
                json.object_value(), args, "requestedServerName", errors,
                /*required=*/false);
            if (requested_server_name.has_value()) {
              permission = Rbac::Permission::MakeReqServerNamePermission(
                  requested_server_name->matcher);
              return;
            }
            auto rules = LoadJsonObjectField<PermissionList>(
                json.object_value(), args, "andRules", errors,
                /*required=*/false);
            if (rules.has_value()) {
              permission = Rbac::Permission::MakeAndPermission(
                  MakeRbacPermissionList(std::move(rules->rules)));
              return;
            }
            rules = LoadJsonObjectField<PermissionList>(json.object_value(),
                                                        args, "orRules", errors,
                                                        /*required=*/false);
            if (rules.has_value()) {
              permission = Rbac::Permission::MakeOrPermission(
                  MakeRbacPermissionList(std::move(rules->rules)));
              return;
            }
            auto not_rule = LoadJsonObjectField<Permission>(
                json.object_value(), args, "notRule", errors,
                /*required=*/false);
            if (not_rule.has_value()) {
              permission = Rbac::Permission::MakeNotPermission(
                  std::move(not_rule->permission));
              return;
            }
            if (errors->size() == original_error_size) {
              errors->AddError("no valid rule found");
            }
          }
        };

        struct Principal {
          struct PrincipalList {
            std::vector<Principal> ids;

            static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
              static const auto* loader = JsonObjectLoader<PrincipalList>()
                                              .Field("ids", &PrincipalList::ids)
                                              .Finish();
              return loader;
            }
          };

          struct Authenticated {
            absl::optional<StringMatch> principal_name;

            static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
              static const auto* loader =
                  JsonObjectLoader<Authenticated>()
                      .OptionalField("principalName",
                                     &Authenticated::principal_name)
                      .Finish();
              return loader;
            }
          };

          Rbac::Principal principal;

          // Work around for MSVC bug
          // https://developercommunity.visualstudio.com/t/C2280-when-modifying-a-vector-containing/377449
          Principal() = default;
          Principal(const Principal&) = delete;
          Principal(Principal&& other) noexcept = default;

          static std::vector<std::unique_ptr<Rbac::Principal>>
          MakeRbacPrincipalList(std::vector<Principal>&& principal_list) {
            std::vector<std::unique_ptr<Rbac::Principal>> principals;
            for (auto& id : principal_list) {
              principals.emplace_back(
                  absl::make_unique<Rbac::Principal>(std::move(id.principal)));
            }
            return principals;
          }

          static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
            // All fields handled in JsonPostLoad().
            static const auto* loader = JsonObjectLoader<Principal>().Finish();
            return loader;
          }

          void JsonPostLoad(const Json& json, const JsonArgs& args,
                            ValidationErrors* errors) {
            const size_t original_error_size = errors->size();
            auto any = LoadJsonObjectField<bool>(
                json.object_value(), args, "any", errors, /*required=*/false);
            if (any.has_value()) {
              principal = Rbac::Principal::MakeAnyPrincipal();
              return;
            }
            auto authenticated = LoadJsonObjectField<Authenticated>(
                json.object_value(), args, "authenticated", errors,
                /*required=*/false);
            if (authenticated.has_value()) {
              if (authenticated->principal_name.has_value()) {
                principal = Rbac::Principal::MakeAuthenticatedPrincipal(
                    std::move(authenticated->principal_name->matcher));
              } else {
                // No principalName found. Match for all users.
                principal = Rbac::Principal::MakeAnyPrincipal();
              }
              return;
            }
            auto cidr_range = LoadJsonObjectField<CidrRange>(
                json.object_value(), args, "sourceIp", errors,
                /*required=*/false);
            if (cidr_range.has_value()) {
              principal = Rbac::Principal::MakeSourceIpPrincipal(
                  std::move(cidr_range->cidr_range));
              return;
            }
            cidr_range = LoadJsonObjectField<CidrRange>(
                json.object_value(), args, "directRemoteIp", errors,
                /*required=*/false);
            if (cidr_range.has_value()) {
              principal = Rbac::Principal::MakeDirectRemoteIpPrincipal(
                  std::move(cidr_range->cidr_range));
              return;
            }
            cidr_range = LoadJsonObjectField<CidrRange>(
                json.object_value(), args, "remoteIp", errors,
                /*required=*/false);
            if (cidr_range.has_value()) {
              principal = Rbac::Principal::MakeRemoteIpPrincipal(
                  std::move(cidr_range->cidr_range));
              return;
            }
            auto header = LoadJsonObjectField<HeaderMatch>(
                json.object_value(), args, "header", errors,
                /*required=*/false);
            if (header.has_value()) {
              principal = Rbac::Principal::MakeHeaderPrincipal(header->matcher);
              return;
            }
            auto url_path = LoadJsonObjectField<PathMatch>(
                json.object_value(), args, "urlPath", errors,
                /*required=*/false);
            if (url_path.has_value()) {
              principal =
                  Rbac::Principal::MakePathPrincipal(url_path->path.matcher);
              return;
            }
            auto metadata = LoadJsonObjectField<Metadata>(
                json.object_value(), args, "metadata", errors,
                /*required=*/false);
            if (metadata.has_value()) {
              principal =
                  Rbac::Principal::MakeMetadataPrincipal(metadata->invert);
              return;
            }
            auto ids = LoadJsonObjectField<PrincipalList>(
                json.object_value(), args, "andIds", errors,
                /*required=*/false);
            if (ids.has_value()) {
              principal = Rbac::Principal::MakeAndPrincipal(
                  MakeRbacPrincipalList(std::move(ids->ids)));
              return;
            }
            ids = LoadJsonObjectField<PrincipalList>(json.object_value(), args,
                                                     "orIds", errors,
                                                     /*required=*/false);
            if (ids.has_value()) {
              principal = Rbac::Principal::MakeOrPrincipal(
                  MakeRbacPrincipalList(std::move(ids->ids)));
              return;
            }
            auto not_rule = LoadJsonObjectField<Principal>(
                json.object_value(), args, "notId", errors,
                /*required=*/false);
            if (not_rule.has_value()) {
              principal = Rbac::Principal::MakeNotPrincipal(
                  std::move(not_rule->principal));
              return;
            }
            if (errors->size() == original_error_size) {
              errors->AddError("no valid id found");
            }
          }
        };

        std::vector<Permission> permissions;
        std::vector<Principal> principals;

        static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
          static const auto* loader =
              JsonObjectLoader<Policy>()
                  .Field("permissions", &Policy::permissions)
                  .Field("principals", &Policy::principals)
                  .Finish();
          return loader;
        }

        Rbac::Policy TakeAsRbacPolicy() {
          Rbac::Policy policy;
          policy.permissions = Rbac::Permission::MakeOrPermission(
              Permission::MakeRbacPermissionList(std::move(permissions)));
          policy.principals = Rbac::Principal::MakeOrPrincipal(
              Principal::MakeRbacPrincipalList(std::move(principals)));
          return policy;
        }
      };

      int action;
      std::map<std::string, Policy> policies;

      static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
        static const auto* loader =
            JsonObjectLoader<Rules>()
                .Field("action", &Rules::action)
                .OptionalField("policies", &Rules::policies)
                .Finish();
        return loader;
      }

      void JsonPostLoad(const Json&, const JsonArgs&,
                        ValidationErrors* errors) {
        // Validate action field.
        auto rbac_action = static_cast<Rbac::Action>(action);
        if (rbac_action != Rbac::Action::kAllow &&
            rbac_action != Rbac::Action::kDeny) {
          ValidationErrors::ScopedField field(errors, ".action");
          errors->AddError("unknown action");
        }
      }

      Rbac TakeAsRbac() {
        Rbac rbac;
        rbac.action = static_cast<Rbac::Action>(action);
        for (auto& p : policies) {
          rbac.policies.emplace(p.first, p.second.TakeAsRbacPolicy());
        }
        return rbac;
      }
    };

    absl::optional<Rules> rules;

    static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
      static const auto* loader =
          JsonObjectLoader<RbacPolicy>()
              .OptionalField("rules", &RbacPolicy::rules)
              .Finish();
      return loader;
    }

    Rbac TakeAsRbac() {
      if (!rules.has_value()) {
        // No enforcing to be applied. An empty deny policy with an empty map
        // is equivalent to no enforcing.
        return Rbac(Rbac::Action::kDeny, {});
      }
      return rules->TakeAsRbac();
    }
  };

  std::vector<RbacPolicy> rbac_policies;

  static const JsonLoaderInterface* JsonLoader(const JsonArgs&) {
    static const auto* loader =
        JsonObjectLoader<RbacConfig>()
            .Field("rbacPolicy", &RbacConfig::rbac_policies)
            .Finish();
    return loader;
  }

  std::vector<Rbac> TakeAsRbacList() {
    std::vector<Rbac> rbac_list;
    for (auto& rbac_policy : rbac_policies) {
      rbac_list.emplace_back(rbac_policy.TakeAsRbac());
    }
    return rbac_list;
  }
};

}  // namespace

std::unique_ptr<ServiceConfigParser::ParsedConfig>
RbacServiceConfigParser::ParsePerMethodParams(const ChannelArgs& args,
                                              const Json& json,
                                              ValidationErrors* errors) {
  // Only parse rbac policy if the channel arg is present
  if (!args.GetBool(GRPC_ARG_PARSE_RBAC_METHOD_CONFIG).value_or(false)) {
    return nullptr;
  }
  auto rbac_config = LoadFromJson<RbacConfig>(json, JsonArgs(), errors);
  std::vector<Rbac> rbac_policies = rbac_config.TakeAsRbacList();
  if (rbac_policies.empty()) return nullptr;
  return absl::make_unique<RbacMethodParsedConfig>(std::move(rbac_policies));
}

void RbacServiceConfigParser::Register(CoreConfiguration::Builder* builder) {
  builder->service_config_parser()->RegisterParser(
      absl::make_unique<RbacServiceConfigParser>());
}

size_t RbacServiceConfigParser::ParserIndex() {
  return CoreConfiguration::Get().service_config_parser().GetParserIndex(
      parser_name());
}

}  // namespace grpc_core
