#!/bin/sh

sed "s/@CENTOS_VERSION@/6/g" < centreon-build/containers/build-dependencies.Dockerfile.in > centreon-build/containers/build-dependencies.centos6.Dockerfile
sed "s/@VERSION@/3.4/g;s/@DISTRIB@/el6/g" < centreon-build/containers/repo/centreon-internal.repo.in > centreon-build/containers/repo/centreon-internal.repo
docker build --no-cache -t ci.int.centreon.com:5000/mon-build-dependencies:centos6 -f centreon-build/containers/build-dependencies.centos6.Dockerfile centreon-build/containers
docker push ci.int.centreon.com:5000/mon-build-dependencies:centos6
