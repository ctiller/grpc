config_setting(
    name = "darwin",
    values = {"cpu": "darwin"},
)

# Android is not officially supported through C++.
# This just helps with the build for now.
config_setting(
    name = "android",
    values = {
      "crosstool_top": "//external:android/crosstool",
    },
)

# iOS is not officially supported through C++.
# This just helps with the build for now.
config_setting(
    name = "ios_x86_64",
    values = {"cpu": "ios_x86_64"},
)
config_setting(
    name = "ios_armv7",
    values = {"cpu": "ios_armv7"},
)
config_setting(
    name = "ios_armv7s",
    values = {"cpu": "ios_armv7s"},
)
config_setting(
    name = "ios_arm64",
    values = {"cpu": "ios_arm64"},
)

cc_library(
    name = "ares",
    srcs = [
        "cares/ares__close_sockets.c",
        "cares/ares__get_hostent.c",
        "cares/ares__read_line.c",
        "cares/ares__timeval.c",
        "cares/ares_cancel.c",
        "cares/ares_create_query.c",
        "cares/ares_data.c",
        "cares/ares_destroy.c",
        "cares/ares_expand_name.c",
        "cares/ares_expand_string.c",
        "cares/ares_fds.c",
        "cares/ares_free_hostent.c",
        "cares/ares_free_string.c",
        "cares/ares_getenv.c",
        "cares/ares_gethostbyaddr.c",
        "cares/ares_gethostbyname.c",
        "cares/ares_getnameinfo.c",
        "cares/ares_getopt.c",
        "cares/ares_getsock.c",
        "cares/ares_init.c",
        "cares/ares_library_init.c",
        "cares/ares_llist.c",
        "cares/ares_mkquery.c",
        "cares/ares_nowarn.c",
        "cares/ares_options.c",
        "cares/ares_parse_a_reply.c",
        "cares/ares_parse_aaaa_reply.c",
        "cares/ares_parse_mx_reply.c",
        "cares/ares_parse_naptr_reply.c",
        "cares/ares_parse_ns_reply.c",
        "cares/ares_parse_ptr_reply.c",
        "cares/ares_parse_soa_reply.c",
        "cares/ares_parse_srv_reply.c",
        "cares/ares_parse_txt_reply.c",
        "cares/ares_platform.c",
        "cares/ares_process.c",
        "cares/ares_query.c",
        "cares/ares_search.c",
        "cares/ares_send.c",
        "cares/ares_strcasecmp.c",
        "cares/ares_strdup.c",
        "cares/ares_strerror.c",
        "cares/ares_timeout.c",
        "cares/ares_version.c",
        "cares/ares_writev.c",
        "cares/bitncmp.c",
        "cares/inet_net_pton.c",
        "cares/inet_ntop.c",
        "cares/windows_port.c",
    ],
    hdrs = [
        "ares_build.h",
        "cares/ares.h",
        "cares/ares_data.h",
        "cares/ares_dns.h",
        "cares/ares_getenv.h",
        "cares/ares_getopt.h",
        "cares/ares_inet_net_pton.h",
        "cares/ares_iphlpapi.h",
        "cares/ares_ipv6.h",
        "cares/ares_library_init.h",
        "cares/ares_llist.h",
        "cares/ares_nowarn.h",
        "cares/ares_platform.h",
        "cares/ares_private.h",
        "cares/ares_rules.h",
        "cares/ares_setup.h",
        "cares/ares_strcasecmp.h",
        "cares/ares_strdup.h",
        "cares/ares_version.h",
        "cares/bitncmp.h",
        "cares/config-win32.h",
        "cares/nameser.h",
        "cares/setup_once.h",
    ] + select({
        ":ios_x86_64": ["config_darwin/ares_config.h"],
        ":ios_armv7": ["config_darwin/ares_config.h"],
        ":ios_armv7s": ["config_darwin/ares_config.h"],
        ":ios_arm64": ["config_darwin/ares_config.h"],
        ":darwin": ["config_darwin/ares_config.h"],
        ":android": ["config_android/ares_config.h"],
        "//conditions:default": ["config_linux/ares_config.h"],
    }),
    includes = [
        ".",
        "cares"
    ] + select({
        ":ios_x86_64": ["config_darwin"],
        ":ios_armv7": ["config_darwin"],
        ":ios_armv7s": ["config_darwin"],
        ":ios_arm64": ["config_darwin"],
        ":darwin": ["config_darwin"],
        ":android": ["config_android"],
        "//conditions:default": ["config_linux"],
    }),
    linkstatic = 1,
    visibility = [
        "//visibility:public",
    ],
    copts = [
        "-D_GNU_SOURCE",
        "-D_HAS_EXCEPTIONS=0",
        "-DNOMINMAX",
        "-DHAVE_CONFIG_H",
    ],
)
