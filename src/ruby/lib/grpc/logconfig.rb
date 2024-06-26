# Copyright 2015 gRPC authors.
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

# GRPC contains the General RPC module.
module GRPC
  def self.logger=(logger_obj)
    # Need a free variable here to keep value of logger_obj for logger closure
    @logger = logger_obj

    extend(
      Module.new do
        def logger
          @logger
        end
      end
    )
  end

  # DefaultLogger is a module included in GRPC if no other logging is set up for
  # it.  See ../spec/spec_helpers an example of where other logging is added.
  module DefaultLogger
    def logger
      LOGGER
    end

    private

    # NoopLogger implements the methods of Ruby's conventional logging interface
    # that are actually used internally within gRPC with a noop implementation.
    class NoopLogger
      def info(_ignored)
      end

      def debug(_ignored)
      end

      def warn(_ignored)
      end
    end

    LOGGER = NoopLogger.new
  end

  # Inject the noop #logger if no module-level logger method has been injected.
  extend DefaultLogger unless methods.include?(:logger)
end
