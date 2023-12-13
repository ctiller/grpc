# Copyright 2023 The gRPC Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
This file is generated by generate_dockerimage_current_versions_bzl.sh
It makes the info from testing docker image *.current_version files
accessible to bazel builds.
"""

DOCKERIMAGE_CURRENT_VERSIONS = {
    "third_party/rake-compiler-dock/rake_aarch64-linux.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rake_aarch64-linux@sha256:a015b83950b891a61b43cbfcdac5bcdfd62d34ac58652302b3e25633274e942a",
    "third_party/rake-compiler-dock/rake_arm64-darwin.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rake_arm64-darwin@sha256:5e207456cc0cdd67f308023b5cbc6932a6db56e58a537862292e76c8ac2e5f90",
    "third_party/rake-compiler-dock/rake_x64-mingw-ucrt.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rake_x64-mingw-ucrt@sha256:4b60f82c8425db2b8fb430833175538aa362cf61d7f88c9c68cbf5970bf8b92c",
    "third_party/rake-compiler-dock/rake_x64-mingw32.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rake_x64-mingw32@sha256:eab102dc22d88124c78444f35b3a24bb45bce379b147cca0c5b64662ca41a82c",
    "third_party/rake-compiler-dock/rake_x86-linux.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rake_x86-linux@sha256:c1221bb41bb36ecc2b998431aabdffacec8bacea03dde2c6bf717a71a25975e2",
    "third_party/rake-compiler-dock/rake_x86-mingw32.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rake_x86-mingw32@sha256:909ae31711de69a129bac3b2ed568ccc771f467ec3c712cf2cb142c3cdd6a154",
    "third_party/rake-compiler-dock/rake_x86_64-darwin.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rake_x86_64-darwin@sha256:44865f928d61f97369903673785fcbe0f6a823a60efcd045c025ca9c5579bcee",
    "third_party/rake-compiler-dock/rake_x86_64-linux.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rake_x86_64-linux@sha256:91603592860c33da1773711b9422b7ffc8ecb69892d112082be17093f2995095",
    "tools/dockerfile/distribtest/cpp_debian10_aarch64_cross_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cpp_debian10_aarch64_cross_x64@sha256:15eeafcd816cb32a0d44da22f654749352a92fec9626dc028b39948897d5bea3",
    "tools/dockerfile/distribtest/cpp_debian10_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cpp_debian10_x64@sha256:904e3db8521697768f94aa08230063b474246184e126f74a41b98a6f4aaf6a49",
    "tools/dockerfile/distribtest/csharp_alpine_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/csharp_alpine_x64@sha256:d018105349fcabdc3aa0649c1381d840c613df6b442a53a751d7dc839a80d429",
    "tools/dockerfile/distribtest/csharp_centos7_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/csharp_centos7_x64@sha256:ec715dd5fbd621789e7598c8d4ac346a7b4037b0cc83fbb29990dc8e4c1f1a13",
    "tools/dockerfile/distribtest/csharp_debian10_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/csharp_debian10_x64@sha256:8c3838e731da70566adc6f989f2c29351fdb2f629e8797928699fff24b3a0938",
    "tools/dockerfile/distribtest/csharp_dotnet31_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/csharp_dotnet31_x64@sha256:fee52df6064ff84bc9af644c2ea17ab579de3401e3a167d0d43383c24f0d500f",
    "tools/dockerfile/distribtest/csharp_dotnet5_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/csharp_dotnet5_x64@sha256:408425cd74bb8b79a3b09a64ea6c54f6cdc0e757a3469f31effc017a7187e442",
    "tools/dockerfile/distribtest/csharp_ubuntu2204_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/csharp_ubuntu2204_x64@sha256:d77c2dd1c94049e178dbc5cd3bc47199270c12ce3f1f2558b46e978707c63563",
    "tools/dockerfile/distribtest/php7_debian10_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/php7_debian10_x64@sha256:e760a60f2dce2dada571d9b07447a9f99ffeeb366a309dbbb5dc0a43991c22dc",
    "tools/dockerfile/distribtest/python_alpine_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_alpine_x64@sha256:699ac7b86199406fa27e88f30a1c623ef34ac33f6d9330fd13a6f6457ee4e19f",
    "tools/dockerfile/distribtest/python_arch_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_arch_x64@sha256:2c1adadeb010e107132cf5137f32a2d18727796631245b110cc74f69c07502e1",
    "tools/dockerfile/distribtest/python_buster_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_buster_x64@sha256:e501dc8e2f4ab9cd4382974759a879a27c065c8fed5327f538764298fc5c4972",
    "tools/dockerfile/distribtest/python_buster_x86.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_buster_x86@sha256:185fbb174525d67b6146f4d233c804c589b0b57d783bb1bf95bc47cfe792754e",
    "tools/dockerfile/distribtest/python_centos7_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_centos7_x64@sha256:39afaa687cb8516eef1621ed789326fdde2014fd3c81d11a1ded72f2d5285fe1",
    "tools/dockerfile/distribtest/python_dev_alpine3.7_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_alpine3.7_x64@sha256:7c08f67211a49eb72ad08c29de5c64a914c066d9c1670b712e717571b8d5c7e2",
    "tools/dockerfile/distribtest/python_dev_arch_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_arch_x64@sha256:29f179ef2083ee6addd57e90f58781fdc1cb5dc3dd3e228da1af38785b921f35",
    "tools/dockerfile/distribtest/python_dev_buster_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_buster_x64@sha256:e30d6efdeac24e5136cc169d503a239df22147bfb121d27feb1f87d58a8fe64e",
    "tools/dockerfile/distribtest/python_dev_buster_x86.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_buster_x86@sha256:179146fd5d5cc15846c6bf0284c2e261f383caf944559d2d9f7a5af0e0f7152d",
    "tools/dockerfile/distribtest/python_dev_centos7_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_centos7_x64@sha256:e6e9a1b23a0a543050db91e17d621aa899bad04308adaf961c11fa88ba941a95",
    "tools/dockerfile/distribtest/python_dev_fedora36_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_fedora36_x64@sha256:d10ea0c54ecaa861b67942b4adc69178585cd071c9d0c8997fa274a362beea55",
    "tools/dockerfile/distribtest/python_dev_ubuntu1804_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_ubuntu1804_x64@sha256:157a89cd6d0e69b89ac1975e0314aade556a35aafbaa5fe9f9890f90321d6c89",
    "tools/dockerfile/distribtest/python_dev_ubuntu2004_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_ubuntu2004_x64@sha256:91f0d88c43ec52ecd63f99acb424c88ff9a67fa046fae207a75e99bee37eef11",
    "tools/dockerfile/distribtest/python_dev_ubuntu2204_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_dev_ubuntu2204_x64@sha256:9e6c9ddc738afcd73fcf2de27b22fdc22a74f1e90e214345838373d9c65ea215",
    "tools/dockerfile/distribtest/python_fedora36_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_fedora36_x64@sha256:85b2d2fbbcfc1b995ce3916dcf8240c29dbc72f91bd47f04187c2db008570ba4",
    "tools/dockerfile/distribtest/python_opensuse_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_opensuse_x64@sha256:da52566b078d10e537aa219e59641731a57e5dc7d17d6737f5e5a7d447acf5cc",
    "tools/dockerfile/distribtest/python_python38_buster_aarch64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_python38_buster_aarch64@sha256:487b9af2ad1459ee2630694e61074d4ac525d4f90b2bdb026dbf6f77fb3e9878",
    "tools/dockerfile/distribtest/python_ubuntu1804_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_ubuntu1804_x64@sha256:edcd5f342d77ad9129cc0a0e6988b47b144815e7a93091d5b45e850111eefbcf",
    "tools/dockerfile/distribtest/python_ubuntu2004_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_ubuntu2004_x64@sha256:342e9dc23b674ad256b220745745be818708a1baa25a2690f0d4f777e28a22a3",
    "tools/dockerfile/distribtest/python_ubuntu2204_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_ubuntu2204_x64@sha256:b6ca497348741615406d1d571cf5042008da934c4f708877aa06a66b5dc3036c",
    "tools/dockerfile/distribtest/ruby_centos7_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/ruby_centos7_x64@sha256:4d529b984b78ca179086f7f9b416605e2d9a96ca0a28a71f4421bb5ffdc18f96",
    "tools/dockerfile/distribtest/ruby_debian10_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/ruby_debian10_x64@sha256:1298c39c950b2a48261555b6cff1ae66230a5020f100d3b381759285f0caf84e",
    "tools/dockerfile/distribtest/ruby_debian10_x64_ruby_2_7.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/ruby_debian10_x64_ruby_2_7@sha256:5ee26ad3abe2683c9a8ee03987ab0ae63f50793c3d3f5e4be6e6cbacb4556fcf",
    "tools/dockerfile/distribtest/ruby_debian10_x64_ruby_3_0.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/ruby_debian10_x64_ruby_3_0@sha256:9190da90a2a95eca1370cef64dcba7ddee9f59cc7487093da6711c1280a0b0f9",
    "tools/dockerfile/distribtest/ruby_ubuntu1804_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/ruby_ubuntu1804_x64@sha256:d38b3dd34cffc027e9a1bf82bc7ace75b8a9829c2d04d5cf7cc8323655edd27a",
    "tools/dockerfile/distribtest/ruby_ubuntu2204_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/ruby_ubuntu2204_x64@sha256:1c74c312f8a4ab37e629732a35daa3056e12136b90f37540cdf9fa11303c7eb8",
    "tools/dockerfile/grpc_artifact_centos6_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_centos6_x64@sha256:3285047265ea2b7c5d4df4c769b2d05f56288d947c75e16d27ae2dee693f791b",
    "tools/dockerfile/grpc_artifact_centos6_x86.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_centos6_x86@sha256:19783239da92208f0f39cf563529cd02e889920497ef81c60d20391fa998af62",
    "tools/dockerfile/grpc_artifact_protoc_aarch64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_protoc_aarch64@sha256:1a3957f32e81259e6f3c602bd67feb132ebc5a5f23e9cb0bf63ba34b91185982",
    "tools/dockerfile/grpc_artifact_python_linux_armv7.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_python_linux_armv7@sha256:f109d6c22cadb053f6843a66ee827d74f34d6cbf75a32f455a9da099ed1bdc9c",
    "tools/dockerfile/grpc_artifact_python_manylinux2014_aarch64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_python_manylinux2014_aarch64@sha256:ef3ae7991763a499b7a0317fd628d1c9d1eef7931fb160361d230f2009febdc3",
    "tools/dockerfile/grpc_artifact_python_manylinux2014_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_python_manylinux2014_x64@sha256:0725226dfb1f707ed3cb2d7cafb2376c33fa89d58577bf80e1d76a0c3bc92c64",
    "tools/dockerfile/grpc_artifact_python_manylinux2014_x86.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_python_manylinux2014_x86@sha256:8d00dc61594f4fa05ada5c38d55627f139a73cd3a6e4fbc0907a866f2a5f5b17",
    "tools/dockerfile/grpc_artifact_python_musllinux_1_1_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_python_musllinux_1_1_x64@sha256:a53a196823862af50ec05d0c5092f765438b488772d1bf53f431cbc98233403d",
    "tools/dockerfile/grpc_artifact_python_musllinux_1_1_x86.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_artifact_python_musllinux_1_1_x86@sha256:3b0892b4d7c751781fe7947d785837cb5a9c9768519066a4e755838acdfd3ad6",
    "tools/dockerfile/interoptest/grpc_interop_aspnetcore.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_aspnetcore@sha256:8e2e732e78724a8382c340dca72e7653c5f82c251a3110fa2874cc00ba538878",
    "tools/dockerfile/interoptest/grpc_interop_cxx.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_cxx@sha256:5a0b1690e6eb63d0904786da9daf6e9a66c901dc7b73d97b4652c05473401790",
    "tools/dockerfile/interoptest/grpc_interop_dart.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_dart@sha256:5e335005b27709f0882c5723affafa55094bd27a0cda7ce91c718deed157f2bb",
    "tools/dockerfile/interoptest/grpc_interop_go.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_go@sha256:889e7ff34399a5e16af87940d1eaa239e56da307f7faca3f8f1d28379c2e3df3",
    "tools/dockerfile/interoptest/grpc_interop_go1.11.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_go1.11@sha256:29cde59287843a3208c0cabeaf430cf813846a738c8a1b9692e68b54bbbdcc2d",
    "tools/dockerfile/interoptest/grpc_interop_go1.16.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_go1.16@sha256:d5b2b0c02e7a8196fea704196a8221994983c22eece2ac2324e095e8972a957f",
    "tools/dockerfile/interoptest/grpc_interop_go1.19.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_go1.19@sha256:889e7ff34399a5e16af87940d1eaa239e56da307f7faca3f8f1d28379c2e3df3",
    "tools/dockerfile/interoptest/grpc_interop_go1.8.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_go1.8@sha256:7830a301b37539252c592b9cd7fa30a6142d0afc717a05fc8d2b82c74fb45efe",
    "tools/dockerfile/interoptest/grpc_interop_http2.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_http2@sha256:e3f247d8038374848fadf7215b841e3575c0b2a4217feb503a79b8004b164c5a",
    "tools/dockerfile/interoptest/grpc_interop_java.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_java@sha256:d9210764071662ba2f377dafcaff4b743f41e4dff1876dd47df7b1c6950f88af",
    "tools/dockerfile/interoptest/grpc_interop_node.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_node@sha256:92f946fc9ff71d79bf1f1a0dff2b2eb38b51d5ff27a77a92fe317a776d64a3ef",
    "tools/dockerfile/interoptest/grpc_interop_nodepurejs.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_nodepurejs@sha256:76925722a5cce232e2e0fa459a5119e47606318af6c77a4a973ca4e7da2e1a9d",
    "tools/dockerfile/interoptest/grpc_interop_php7.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_php7@sha256:9342ff81689c37d9e79fd6abcc08bf310eb48174e83bd3bfce39d225c02f0d4e",
    "tools/dockerfile/interoptest/grpc_interop_python.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_python@sha256:1f55faacfb4be587e6d26b05561e79bf3e17fe81c69a990e8aeca4257081c9ac",
    "tools/dockerfile/interoptest/grpc_interop_pythonasyncio.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_pythonasyncio@sha256:47127a7863097b436613885a8866a2ef055470452838ceebb31f692ac88ac1d1",
    "tools/dockerfile/interoptest/grpc_interop_ruby.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/grpc_interop_ruby@sha256:7b044d6848f82234dba81b38d8eca220b608f830f93b42932df59ed6fe20b24d",
    "tools/dockerfile/interoptest/lb_interop_fake_servers.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/lb_interop_fake_servers@sha256:b89a51dd9147e1293f50ee64dd719fce5929ca7894d3770a3d80dbdecb99fd52",
    "tools/dockerfile/test/android_ndk.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/android_ndk@sha256:64ffc5d1e117172ca4dda89720087616830996181192de25fe10e03a88f0b3e5",
    "tools/dockerfile/test/bazel.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/bazel@sha256:32bde2dcb2087f2a32afab59e4dfedf7e8c76a52c69881f63a239d311f0e5ecf",
    "tools/dockerfile/test/bazel_arm64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/bazel_arm64@sha256:3b087387c44dee405c1b80d6ff50994e6d8e90a4ef67cc94b4291f1a29c0ef41",
    "tools/dockerfile/test/binder_transport_apk.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/binder_transport_apk@sha256:bf60a187cd2ce1abe8b4f32ae6479040a72ca6aa789cd5ab509f60ceb37a41f9",
    "tools/dockerfile/test/csharp_debian11_arm64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/csharp_debian11_arm64@sha256:4d4bc5f15e03f3d3d8fd889670ecde2c66a2e4d2dd9db80733c05c1d90c8a248",
    "tools/dockerfile/test/csharp_debian11_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/csharp_debian11_x64@sha256:b2e5c47d986312ea0850e2f2e696b45d23ee0aabceea161d31e28559e19ec4a5",
    "tools/dockerfile/test/cxx_alpine_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_alpine_x64@sha256:f2019edf9f2afd5042567f11afb1aa78a789fc9acdcce5ee0c14cc11f6830ed7",
    "tools/dockerfile/test/cxx_clang_16_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_clang_16_x64@sha256:7559d2743aa03e3247e6c80b412aec06b56a1a3aaa45ac174c07ea94afe62e1d",
    "tools/dockerfile/test/cxx_clang_6_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_clang_6_x64@sha256:8e9ddd6c0f3d04c1bf9370cc59712a4e5883f68f307643a9b6dcb2dbd678b579",
    "tools/dockerfile/test/cxx_debian11_openssl102_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_debian11_openssl102_x64@sha256:3bae65e56c756c491dfc4b02cff554ae3c4edd9d366d0d05d27e429b59a01b0f",
    "tools/dockerfile/test/cxx_debian11_openssl111_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_debian11_openssl111_x64@sha256:63f2cd5ae453aa2a850fc59b9ec8e8a865e63d9f99a2b796de65669f3bf21275",
    "tools/dockerfile/test/cxx_debian11_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_debian11_x64@sha256:ecd1fbc04423ac9c667a3fbd985a9530e9a4387db372e22229a4d9d77034f4c3",
    "tools/dockerfile/test/cxx_debian11_x86.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_debian11_x86@sha256:cba4b92ff05bc51c0668bfce696d4a826728f4c61163b6d6107685cd83098b15",
    "tools/dockerfile/test/cxx_debian12_openssl309_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_debian12_openssl309_x64@sha256:a3f1242283eb5988e4aefede8af64db6f4bfea0d2875610cf55560abeb60563e",
    "tools/dockerfile/test/cxx_gcc_12_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_gcc_12_x64@sha256:bbdfe66f27b964f9bfd526646b94a19d904fea52bdb244f32fd4355cc8c4551f",
    "tools/dockerfile/test/cxx_gcc_8_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/cxx_gcc_8_x64@sha256:a12780fc969d6856211bb80d85de22198bcf0b2f325d1f2e5cc2d779b41ee919",
    "tools/dockerfile/test/php73_zts_debian11_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/php73_zts_debian11_x64@sha256:bc221d435086d92b2482020214ee70814c569273f8d0cb577a0247e82d598a5f",
    "tools/dockerfile/test/php7_debian11_arm64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/php7_debian11_arm64@sha256:7ee21f253a2ddd255f4f6779cd19818eec6524e78b0bf0a7765339e4aa7347c3",
    "tools/dockerfile/test/php7_debian11_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/php7_debian11_x64@sha256:302c06c5dbffb97dd5540d758a8ce849269527bb7d1c3885af0b956f8f33c49e",
    "tools/dockerfile/test/python_alpine_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_alpine_x64@sha256:75fa424f2dae683422a5875d64911d9abf06c31e944401d240666d06f83de573",
    "tools/dockerfile/test/python_debian11_default_arm64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_debian11_default_arm64@sha256:fccca33a655c7aa89dd7ebd9492cbcc1f636bd2a004cd939d1982cfce3d68326",
    "tools/dockerfile/test/python_debian11_default_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/python_debian11_default_x64@sha256:51107bfe5f9ef2c9d9c7a6554a16c4335a6d48f81a47cf6176fb36eca0605f02",
    "tools/dockerfile/test/rbe_ubuntu2004.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/rbe_ubuntu2004@sha256:d3951aeadf43e3bee6adc5b86d26cdaf0b9d1b5baf790d7b2530d1c197adc9f8",
    "tools/dockerfile/test/ruby_debian11_arm64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/ruby_debian11_arm64@sha256:7e77cf17e2e8657f4cc23ac9f93630bf13213fff961799e0f16dae17cd45cf6d",
    "tools/dockerfile/test/ruby_debian11_x64.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/ruby_debian11_x64@sha256:e4cb502caccf2db733268ce2ddc951fda8a9df2f7f53d6b74523c33d40c83006",
    "tools/dockerfile/test/sanity.current_version": "docker://us-docker.pkg.dev/grpc-testing/testing-images-public/sanity@sha256:f1f5ca30491698f793cae75d5989cb53887d159800aef662cf7e6c3df6f86a88",
}
