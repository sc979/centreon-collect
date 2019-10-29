#!/bin/sh

set -e
set -x

# Install cross-compiler and armhf dependencies.
dpkg --add-architecture armhf
apt-get update
xargs apt-get install < /tmp/build-dependencies.txt
apt-get autoremove --purge libgnutls28-dev:amd64
