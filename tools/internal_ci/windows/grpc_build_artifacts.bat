@rem Copyright 2017 gRPC authors.
@rem
@rem Licensed under the Apache License, Version 2.0 (the "License");
@rem you may not use this file except in compliance with the License.
@rem You may obtain a copy of the License at
@rem
@rem     http://www.apache.org/licenses/LICENSE-2.0
@rem
@rem Unless required by applicable law or agreed to in writing, software
@rem distributed under the License is distributed on an "AS IS" BASIS,
@rem WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem See the License for the specific language governing permissions and
@rem limitations under the License.

@rem Avoid slow finalization after the script has exited.
@rem See the script's prologue for info on the correct invocation pattern.
IF "%cd%"=="T:\src" (
  call %~dp0\..\..\..\tools\internal_ci\helper_scripts\move_src_tree_and_respawn_itself.bat %0
  exit /b %errorlevel%
)

@rem Boringssl build no longer supports yasm
choco uninstall yasm -y --limit-output
choco install nasm -y --limit-output

@rem enter repo root
cd /d %~dp0\..\..\..

set PREPARE_BUILD_INSTALL_DEPS_PYTHON=true
call tools/internal_ci/helper_scripts/prepare_build_windows.bat || exit /b 1

python tools/run_tests/task_runner.py -f artifact windows -j 4
set RUNTESTS_EXITCODE=%errorlevel%

bash tools/internal_ci/helper_scripts/store_artifacts_from_moved_src_tree.sh

exit /b %RUNTESTS_EXITCODE%
