#!/bin/sh

set -e
set -x

. `dirname $0`/../../common.sh

# Project.
PROJECT=centreon-autodiscovery

# Check arguments.
if [ -z "$VERSION" -o -z "$RELEASE" ] ; then
  echo "You need to specify VERSION and RELEASE environment variables."
  exit 1
fi
if [ "$#" -lt 1 ] ; then
  echo "USAGE: $0 <centos7|...>"
  exit 1
fi
DISTRIB="$1"

# Pull base image.
WEB_IMAGE=ci.int.centreon.com:5000/mon-web-18.10:$DISTRIB
docker pull $WEB_IMAGE

# Prepare Dockerfiles.
rm -rf centreon-build-containers
cp -r `dirname $0`/../../../containers centreon-build-containers
cd centreon-build-containers
sed "s/@DISTRIB@/$DISTRIB/g" < autodisco/18.10/Dockerfile.in > autodisco/Dockerfile
sed "s#@PROJECT@#$PROJECT#g;s#@SUBDIR@#18.10/el7/x86_64/autodisco/$PROJECT-$VERSION-$RELEASE#g" < repo/centreon-internal.repo.in > repo/centreon-internal.repo

# Build image.
REGISTRY="ci.int.centreon.com:5000"
AUTODISCO_IMAGE="$REGISTRY/mon-autodisco-$VERSION-$RELEASE:$DISTRIB"
AUTODISCO_WIP_IMAGE="$REGISTRY/mon-autodisco-18.10-wip:$DISTRIB"
docker build --no-cache -t "$AUTODISCO_IMAGE" -f autodisco/Dockerfile .
docker push "$AUTODISCO_IMAGE"
docker tag "$AUTODISCO_IMAGE" "$AUTODISCO_WIP_IMAGE"
docker push "$AUTODISCO_WIP_IMAGE"
