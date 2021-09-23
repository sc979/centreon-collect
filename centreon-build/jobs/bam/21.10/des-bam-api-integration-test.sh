#!/bin/sh

SCRIPT_PATH=`dirname $0`

. $SCRIPT_PATH/../../common.sh

# Project.
PROJECT=centreon-bam-server

# Check arguments.
if [ -z "$VERSION" -o -z "$RELEASE" ] ; then
  echo "You need to specify VERSION and RELEASE environment variables."
  exit 1
fi
if [ "$#" -lt 2 ] ; then
  echo "USAGE: $0 <centos7|centos8> [feature]"
  exit 1
fi
DISTRIB="$1"

# Pull images.
BAM_IMAGE="$REGISTRY/des-bam-$VERSION-$RELEASE:$DISTRIB"
docker pull $BAM_IMAGE

# Get sources.
rm -rf "$PROJECT-$VERSION"
tar xzf "$PROJECT-$VERSION.tar.gz"

tar xzf vendor.tar.gz -C "$PROJECT-$VERSION"

# Prepare Docker Compose file.
sed 's#@WEB_IMAGE@#'$BAM_IMAGE'#g' < $SCRIPT_PATH/../../../containers/web/21.10/docker-compose-api.yml.in > "$PROJECT-$VERSION/docker-compose-bam.yml"

# Prepare Behat.yml
cd "$PROJECT-$VERSION"
alreadyset=`grep docker-compose-bam.yml < tests/api/behat.yml || true`
if [ -z "$alreadyset" ] ; then
  sed -i 's#    Centreon\\Test\\Behat\\Extensions\\ContainerExtension:#    Centreon\\Test\\Behat\\Extensions\\ContainerExtension:\n      log_directory: ../api-integration-test-logs\n      bam: docker-compose-bam.yml#g' tests/api/behat.yml
fi

# Run acceptance tests.
rm -rf ../xunit-reports
mkdir ../xunit-reports
rm -rf ../api-integration-test-logs
mkdir ../api-integration-test-logs

./vendor/bin/behat --config tests/api/behat.yml --format=pretty --out=std --format=junit --out="../xunit-reports" "$2"
