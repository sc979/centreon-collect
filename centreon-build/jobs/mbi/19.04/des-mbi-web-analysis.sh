#!/bin/sh

set -e
set -x

. `dirname $0`/../../common.sh

# Project.
PROJECT=centreon-bi-server

# Retrieve copy of git repository.
curl -o "$PROJECT-git.tar.gz" "http://srvi-repo.int.centreon.com/sources/internal/mbi-web/$PROJECT-$VERSION-$RELEASE/$PROJECT-git.tar.gz"
rm -rf "$PROJECT"
tar xzf "$PROJECT-git.tar.gz"

# Copy reports and run analysis.
cd "$PROJECT"
if [ "$BUILD" '=' 'RELEASE' ] ; then
  sed -i -e 's/centreon-mbi-server-19.04/centreon-mbi-server-19.04-release/g' sonar-project.properties
  sed -i -e 's/Centreon MBI Server 19.04/Centreon MBI Server 19.04 (release)/g' sonar-project.properties
fi
echo "sonar.projectVersion=$VERSION" >> sonar-project.properties
sonar-scanner
