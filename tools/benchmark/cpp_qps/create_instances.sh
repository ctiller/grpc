#!/bin/sh

set -ex

cd $(dirname $0)

source ./config.sh

gcloud compute instances create \
  benchmark1 benchmark2 benchmark3 \
  --image container-vm \
  --zone $ZONE \
  --machine-type $MACHINE \
  --quiet

# wtf
sleep 30

BRANCH=`git rev-parse --abbrev-ref HEAD`

for b in benchmark1 benchmark2 benchmark3
do
  gcloud compute ssh $b --zone $ZONE \
    --command \
      "sudo apt-get update &&
       sudo apt-get install -y \
       	 g++ make autoconf unzip libtool \
       	 libgtest-dev libgflags-dev \
       && git clone https://github.com/ctiller/grpc.git grpc \
       && cd grpc \
       && git checkout $BRANCH \
       && git submodule update --init \
       && make CONFIG=opt qps_worker -j 3" &
done

wait
