#!/bin/sh

set -e
set -x

. `dirname $0`/../../common.sh

# Project.
PROJECT=centreon-map

# Copy reports and run analysis.
cd "$PROJECT-server-$VERSION"
if [ "$BUILD" '=' 'RELEASE' ] ; then
  sed -i -e 's/centreon-map-server-19.04/centreon-map-server-19.04-release/g' sonar-project.properties
  sed -i -e 's/Centreon Map Server 19.04/Centreon Map Server 19.04 (release)/g' sonar-project.properties
fi
echo "sonar.projectVersion=$VERSION" >> sonar-project.properties
sonar-scanner
