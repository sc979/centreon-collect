#!/bin/sh

set -e
set -x

# Check arguments.
if [ "$#" -lt 1 ] ; then
  echo "USAGE: $0 <6|7>"
  exit 1
fi
CENTOS_VERSION="$1"

# Pull mon-ppe image.
docker pull ci.int.centreon.com:5000/mon-middleware:centos$CENTOS_VERSION

# Check that phantomjs is running.
export PHANTOMJS_RUNNING=1
nc -w 0 localhost 4444 || export PHANTOMJS_RUNNING=0 || true
if [ "$PHANTOMJS_RUNNING" -ne 1 ] ; then
  screen -d -m phantomjs --webdriver=4444
fi

# Copy public key of private key used in container.
cp centreon-build/jobs/middleware/public.asc centreon-imp-portal-api

# Run acceptance tests.
export CENTREON_MIDDLEWARE_IMAGE=ci.int.centreon.com:5000/mon-middleware:centos$CENTOS_VERSION
rm -rf xunit-reports
mkdir xunit-reports
cd centreon-imp-portal-api
composer install
composer update
ls features/*.feature | parallel /opt/behat/vendor/bin/behat --strict --format=junit --out="../xunit-reports/{/.}" "{}"
