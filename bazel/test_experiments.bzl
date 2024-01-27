# Copyright 2023 gRPC authors.
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

# Auto generated by tools/codegen/core/gen_experiments.py

"""Dictionary of tags to experiments so we know when to test different experiments."""

TEST_EXPERIMENT_ENABLES = {
    "test_experiment_1": "test_experiment_1",
    "test_experiment_2": "test_experiment_2",
    "test_experiment_3": "test_experiment_3",
    "test_experiment_4": "test_experiment_4",
}

TEST_EXPERIMENT_POLLERS = [
]

TEST_EXPERIMENTS = {
    "windows": {
        "dbg": {
        },
        "off": {
            "experiments_tag_test": [
                "test_experiment_1",
            ],
        },
        "on": {
        },
    },
    "ios": {
        "dbg": {
        },
        "off": {
        },
        "on": {
        },
    },
    "posix": {
        "dbg": {
        },
        "off": {
        },
        "on": {
            "experiments_tag_test": [
                "test_experiment_1",
            ],
        },
    },
}
