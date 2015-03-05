#!/bin/sh

set -ex

source $(dirname $0)/config.sh

gcloud compute instances delete benchmark1 benchmark2 benchmark3 \
  --zone $ZONE \
  --quiet
