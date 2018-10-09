#!/bin/sh

set -e
set -x

. `dirname $0`/../../common.sh

# Project.
PROJECT=centreon-bam

# Check arguments.
if [ -z "$VERSION" -o -z "$RELEASE" ] ; then
  echo "You need to specify VERSION and RELEASE environment variables."
  exit 1
fi
if [ "$#" -lt 1 ] ; then
  echo "USAGE: $0 <centos6|centos7|...>"
  exit 1
fi
DISTRIB="$1"
if [ "$DISTRIB" = "centos6" ] ; then
  CENTOS_VERSION=6
else
  CENTOS_VERSION=7
fi

# Pull base image.
WEB_IMAGE=ci.int.centreon.com:5000/mon-web-3.4:$DISTRIB
docker pull $WEB_IMAGE

# Prepare Dockerfiles.
rm -rf centreon-build-containers
cp -r `dirname $0`/../../../containers centreon-build-containers
cd centreon-build-containers
sed "s/@DISTRIB@/$DISTRIB/g" < bam/3.4/Dockerfile.in > bam/Dockerfile
sed "s#@PROJECT@#$PROJECT#g;s#@SUBDIR@#3.4/el$CENTOS_VERSION/noarch/bam/$PROJECT-$VERSION-$RELEASE#g" < repo/centreon-internal.repo.in > repo/centreon-internal.repo

# Build image.
REGISTRY="ci.int.centreon.com:5000"
BAM_IMAGE="$REGISTRY/des-bam-$VERSION-$RELEASE:$DISTRIB"
BAM_WIP_IMAGE="$REGISTRY/des-bam-3.4-wip:$DISTRIB"
docker build --no-cache -t "$BAM_IMAGE" -f bam/Dockerfile .
docker push "$BAM_IMAGE"
docker tag "$BAM_IMAGE" "$BAM_WIP_IMAGE"
docker push "$BAM_WIP_IMAGE"
