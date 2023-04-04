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
extern const char test_root_cert[] = {
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x42, 0x45, 0x47, 0x49, 0x4e, 0x20, 0x43,
    0x45, 0x52, 0x54, 0x49, 0x46, 0x49, 0x43, 0x41, 0x54, 0x45, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x0a, 0x4d, 0x49, 0x49, 0x44, 0x57, 0x6a, 0x43, 0x43,
    0x41, 0x6b, 0x4b, 0x67, 0x41, 0x77, 0x49, 0x42, 0x41, 0x67, 0x49, 0x55,
    0x57, 0x72, 0x50, 0x30, 0x56, 0x76, 0x48, 0x63, 0x79, 0x2b, 0x4c, 0x50,
    0x36, 0x55, 0x75, 0x59, 0x4e, 0x74, 0x69, 0x4c, 0x39, 0x67, 0x42, 0x68,
    0x44, 0x35, 0x6f, 0x77, 0x44, 0x51, 0x59, 0x4a, 0x4b, 0x6f, 0x5a, 0x49,
    0x68, 0x76, 0x63, 0x4e, 0x41, 0x51, 0x45, 0x4c, 0x0a, 0x42, 0x51, 0x41,
    0x77, 0x56, 0x6a, 0x45, 0x4c, 0x4d, 0x41, 0x6b, 0x47, 0x41, 0x31, 0x55,
    0x45, 0x42, 0x68, 0x4d, 0x43, 0x51, 0x56, 0x55, 0x78, 0x45, 0x7a, 0x41,
    0x52, 0x42, 0x67, 0x4e, 0x56, 0x42, 0x41, 0x67, 0x4d, 0x43, 0x6c, 0x4e,
    0x76, 0x62, 0x57, 0x55, 0x74, 0x55, 0x33, 0x52, 0x68, 0x64, 0x47, 0x55,
    0x78, 0x49, 0x54, 0x41, 0x66, 0x42, 0x67, 0x4e, 0x56, 0x42, 0x41, 0x6f,
    0x4d, 0x0a, 0x47, 0x45, 0x6c, 0x75, 0x64, 0x47, 0x56, 0x79, 0x62, 0x6d,
    0x56, 0x30, 0x49, 0x46, 0x64, 0x70, 0x5a, 0x47, 0x64, 0x70, 0x64, 0x48,
    0x4d, 0x67, 0x55, 0x48, 0x52, 0x35, 0x49, 0x45, 0x78, 0x30, 0x5a, 0x44,
    0x45, 0x50, 0x4d, 0x41, 0x30, 0x47, 0x41, 0x31, 0x55, 0x45, 0x41, 0x77,
    0x77, 0x47, 0x64, 0x47, 0x56, 0x7a, 0x64, 0x47, 0x4e, 0x68, 0x4d, 0x42,
    0x34, 0x58, 0x44, 0x54, 0x49, 0x77, 0x0a, 0x4d, 0x44, 0x4d, 0x78, 0x4e,
    0x7a, 0x45, 0x34, 0x4e, 0x54, 0x6b, 0x31, 0x4d, 0x56, 0x6f, 0x58, 0x44,
    0x54, 0x4d, 0x77, 0x4d, 0x44, 0x4d, 0x78, 0x4e, 0x54, 0x45, 0x34, 0x4e,
    0x54, 0x6b, 0x31, 0x4d, 0x56, 0x6f, 0x77, 0x56, 0x6a, 0x45, 0x4c, 0x4d,
    0x41, 0x6b, 0x47, 0x41, 0x31, 0x55, 0x45, 0x42, 0x68, 0x4d, 0x43, 0x51,
    0x56, 0x55, 0x78, 0x45, 0x7a, 0x41, 0x52, 0x42, 0x67, 0x4e, 0x56, 0x0a,
    0x42, 0x41, 0x67, 0x4d, 0x43, 0x6c, 0x4e, 0x76, 0x62, 0x57, 0x55, 0x74,
    0x55, 0x33, 0x52, 0x68, 0x64, 0x47, 0x55, 0x78, 0x49, 0x54, 0x41, 0x66,
    0x42, 0x67, 0x4e, 0x56, 0x42, 0x41, 0x6f, 0x4d, 0x47, 0x45, 0x6c, 0x75,
    0x64, 0x47, 0x56, 0x79, 0x62, 0x6d, 0x56, 0x30, 0x49, 0x46, 0x64, 0x70,
    0x5a, 0x47, 0x64, 0x70, 0x64, 0x48, 0x4d, 0x67, 0x55, 0x48, 0x52, 0x35,
    0x49, 0x45, 0x78, 0x30, 0x0a, 0x5a, 0x44, 0x45, 0x50, 0x4d, 0x41, 0x30,
    0x47, 0x41, 0x31, 0x55, 0x45, 0x41, 0x77, 0x77, 0x47, 0x64, 0x47, 0x56,
    0x7a, 0x64, 0x47, 0x4e, 0x68, 0x4d, 0x49, 0x49, 0x42, 0x49, 0x6a, 0x41,
    0x4e, 0x42, 0x67, 0x6b, 0x71, 0x68, 0x6b, 0x69, 0x47, 0x39, 0x77, 0x30,
    0x42, 0x41, 0x51, 0x45, 0x46, 0x41, 0x41, 0x4f, 0x43, 0x41, 0x51, 0x38,
    0x41, 0x4d, 0x49, 0x49, 0x42, 0x43, 0x67, 0x4b, 0x43, 0x0a, 0x41, 0x51,
    0x45, 0x41, 0x73, 0x47, 0x4c, 0x30, 0x6f, 0x58, 0x66, 0x6c, 0x46, 0x30,
    0x4c, 0x7a, 0x6f, 0x4d, 0x2b, 0x42, 0x68, 0x2b, 0x71, 0x55, 0x55, 0x39,
    0x79, 0x68, 0x71, 0x7a, 0x77, 0x32, 0x77, 0x38, 0x4f, 0x4f, 0x58, 0x35,
    0x6d, 0x75, 0x2f, 0x69, 0x4e, 0x43, 0x79, 0x55, 0x4f, 0x42, 0x72, 0x71,
    0x61, 0x48, 0x69, 0x37, 0x6d, 0x47, 0x48, 0x78, 0x37, 0x33, 0x47, 0x44,
    0x30, 0x31, 0x0a, 0x64, 0x69, 0x4e, 0x7a, 0x43, 0x7a, 0x76, 0x6c, 0x63,
    0x51, 0x71, 0x64, 0x4e, 0x49, 0x48, 0x36, 0x4e, 0x51, 0x53, 0x4c, 0x37,
    0x44, 0x54, 0x70, 0x42, 0x6a, 0x63, 0x61, 0x36, 0x36, 0x6a, 0x59, 0x54,
    0x39, 0x75, 0x37, 0x33, 0x76, 0x5a, 0x65, 0x32, 0x4d, 0x44, 0x72, 0x72,
    0x31, 0x6e, 0x56, 0x62, 0x75, 0x4c, 0x76, 0x66, 0x75, 0x39, 0x38, 0x35,
    0x30, 0x63, 0x64, 0x78, 0x69, 0x55, 0x4f, 0x0a, 0x49, 0x6e, 0x76, 0x35,
    0x78, 0x66, 0x38, 0x2b, 0x73, 0x54, 0x48, 0x47, 0x30, 0x43, 0x2b, 0x61,
    0x2b, 0x56, 0x41, 0x76, 0x4d, 0x68, 0x73, 0x4c, 0x69, 0x52, 0x6a, 0x73,
    0x71, 0x2b, 0x6c, 0x58, 0x4b, 0x52, 0x4a, 0x79, 0x6b, 0x35, 0x7a, 0x6b,
    0x62, 0x62, 0x73, 0x45, 0x54, 0x79, 0x62, 0x71, 0x70, 0x78, 0x6f, 0x4a,
    0x2b, 0x4b, 0x37, 0x43, 0x6f, 0x53, 0x79, 0x33, 0x79, 0x63, 0x2f, 0x6b,
    0x0a, 0x51, 0x49, 0x59, 0x33, 0x54, 0x69, 0x70, 0x77, 0x45, 0x74, 0x77,
    0x6b, 0x4b, 0x50, 0x34, 0x68, 0x7a, 0x79, 0x6f, 0x36, 0x4b, 0x69, 0x47,
    0x64, 0x2f, 0x44, 0x50, 0x65, 0x78, 0x69, 0x65, 0x34, 0x6e, 0x42, 0x55,
    0x49, 0x6e, 0x4e, 0x33, 0x62, 0x53, 0x31, 0x42, 0x55, 0x65, 0x4e, 0x5a,
    0x35, 0x7a, 0x65, 0x61, 0x49, 0x43, 0x32, 0x65, 0x67, 0x33, 0x62, 0x6b,
    0x65, 0x65, 0x57, 0x37, 0x63, 0x0a, 0x71, 0x54, 0x35, 0x35, 0x62, 0x2b,
    0x59, 0x65, 0x6e, 0x36, 0x43, 0x78, 0x59, 0x30, 0x54, 0x45, 0x6b, 0x7a,
    0x42, 0x4b, 0x36, 0x41, 0x4b, 0x74, 0x2f, 0x57, 0x55, 0x69, 0x61, 0x6c,
    0x4b, 0x4d, 0x67, 0x54, 0x30, 0x77, 0x62, 0x54, 0x78, 0x52, 0x5a, 0x4f,
    0x37, 0x6b, 0x55, 0x43, 0x48, 0x33, 0x53, 0x71, 0x36, 0x65, 0x2f, 0x77,
    0x58, 0x65, 0x46, 0x64, 0x4a, 0x2b, 0x48, 0x76, 0x64, 0x56, 0x0a, 0x4c,
    0x50, 0x6c, 0x41, 0x67, 0x35, 0x54, 0x6e, 0x4d, 0x61, 0x4e, 0x70, 0x52,
    0x64, 0x51, 0x69, 0x68, 0x2f, 0x38, 0x6e, 0x52, 0x46, 0x70, 0x73, 0x64,
    0x77, 0x49, 0x44, 0x41, 0x51, 0x41, 0x42, 0x6f, 0x79, 0x41, 0x77, 0x48,
    0x6a, 0x41, 0x4d, 0x42, 0x67, 0x4e, 0x56, 0x48, 0x52, 0x4d, 0x45, 0x42,
    0x54, 0x41, 0x44, 0x41, 0x51, 0x48, 0x2f, 0x4d, 0x41, 0x34, 0x47, 0x41,
    0x31, 0x55, 0x64, 0x0a, 0x44, 0x77, 0x45, 0x42, 0x2f, 0x77, 0x51, 0x45,
    0x41, 0x77, 0x49, 0x43, 0x42, 0x44, 0x41, 0x4e, 0x42, 0x67, 0x6b, 0x71,
    0x68, 0x6b, 0x69, 0x47, 0x39, 0x77, 0x30, 0x42, 0x41, 0x51, 0x73, 0x46,
    0x41, 0x41, 0x4f, 0x43, 0x41, 0x51, 0x45, 0x41, 0x6b, 0x54, 0x72, 0x4b,
    0x5a, 0x6a, 0x42, 0x72, 0x4a, 0x58, 0x48, 0x70, 0x73, 0x2f, 0x48, 0x72,
    0x6a, 0x4e, 0x43, 0x46, 0x50, 0x62, 0x35, 0x61, 0x0a, 0x54, 0x48, 0x75,
    0x47, 0x50, 0x43, 0x53, 0x73, 0x65, 0x70, 0x65, 0x31, 0x77, 0x6b, 0x4b,
    0x64, 0x53, 0x70, 0x31, 0x68, 0x34, 0x48, 0x47, 0x52, 0x70, 0x4c, 0x6f,
    0x43, 0x67, 0x63, 0x4c, 0x79, 0x73, 0x43, 0x4a, 0x35, 0x68, 0x5a, 0x68,
    0x52, 0x70, 0x48, 0x6b, 0x52, 0x69, 0x68, 0x68, 0x65, 0x66, 0x2b, 0x72,
    0x46, 0x48, 0x45, 0x65, 0x36, 0x30, 0x55, 0x65, 0x50, 0x51, 0x4f, 0x33,
    0x53, 0x0a, 0x43, 0x56, 0x54, 0x74, 0x64, 0x4a, 0x42, 0x34, 0x43, 0x59,
    0x57, 0x70, 0x63, 0x4e, 0x79, 0x58, 0x4f, 0x64, 0x71, 0x65, 0x66, 0x72,
    0x62, 0x4a, 0x57, 0x35, 0x51, 0x4e, 0x6c, 0x6a, 0x78, 0x67, 0x69, 0x36,
    0x46, 0x68, 0x76, 0x73, 0x37, 0x4a, 0x4a, 0x6b, 0x42, 0x71, 0x64, 0x58,
    0x49, 0x6b, 0x57, 0x58, 0x74, 0x46, 0x6b, 0x32, 0x65, 0x52, 0x67, 0x4f,
    0x49, 0x50, 0x32, 0x45, 0x6f, 0x39, 0x0a, 0x2f, 0x4f, 0x48, 0x51, 0x48,
    0x6c, 0x59, 0x6e, 0x77, 0x5a, 0x46, 0x72, 0x6b, 0x36, 0x73, 0x70, 0x34,
    0x77, 0x50, 0x79, 0x52, 0x2b, 0x41, 0x39, 0x35, 0x53, 0x30, 0x74, 0x6f,
    0x5a, 0x42, 0x63, 0x79, 0x44, 0x56, 0x7a, 0x37, 0x75, 0x2b, 0x68, 0x4f,
    0x57, 0x30, 0x70, 0x47, 0x4b, 0x33, 0x77, 0x76, 0x69, 0x4f, 0x65, 0x39,
    0x6c, 0x76, 0x52, 0x67, 0x6a, 0x2f, 0x48, 0x33, 0x50, 0x77, 0x74, 0x0a,
    0x62, 0x65, 0x77, 0x62, 0x30, 0x6c, 0x2b, 0x4d, 0x68, 0x52, 0x69, 0x67,
    0x30, 0x2f, 0x44, 0x56, 0x48, 0x61, 0x6d, 0x79, 0x56, 0x78, 0x72, 0x44,
    0x52, 0x62, 0x71, 0x49, 0x6e, 0x55, 0x31, 0x2f, 0x47, 0x54, 0x4e, 0x43,
    0x77, 0x63, 0x5a, 0x6b, 0x58, 0x4b, 0x59, 0x46, 0x57, 0x53, 0x66, 0x39,
    0x32, 0x55, 0x2b, 0x6b, 0x49, 0x63, 0x54, 0x74, 0x68, 0x32, 0x34, 0x51,
    0x31, 0x67, 0x63, 0x77, 0x0a, 0x65, 0x5a, 0x69, 0x4c, 0x6c, 0x35, 0x46,
    0x66, 0x72, 0x57, 0x6f, 0x6b, 0x55, 0x4e, 0x79, 0x74, 0x46, 0x45, 0x6c,
    0x58, 0x6f, 0x62, 0x30, 0x56, 0x30, 0x61, 0x35, 0x2f, 0x6b, 0x62, 0x68,
    0x69, 0x4c, 0x63, 0x33, 0x79, 0x57, 0x6d, 0x76, 0x57, 0x71, 0x48, 0x54,
    0x70, 0x71, 0x43, 0x41, 0x4c, 0x62, 0x56, 0x79, 0x46, 0x2b, 0x72, 0x4b,
    0x4a, 0x6f, 0x32, 0x66, 0x35, 0x4b, 0x77, 0x3d, 0x3d, 0x0a, 0x2d, 0x2d,
    0x2d, 0x2d, 0x2d, 0x45, 0x4e, 0x44, 0x20, 0x43, 0x45, 0x52, 0x54, 0x49,
    0x46, 0x49, 0x43, 0x41, 0x54, 0x45, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x0a,
    0x00};
