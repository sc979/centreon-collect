#!/bin/sh

set -e
set -x

export PROJECT='centreon-map-client'

# Check arguments.
if [ -z "$VERSION" ] ; then
  echo "You need to specify VERSION environment variable."
  exit 1
fi

#IFS='.' read -ra VERSION_TABLE <<< "$VERSION"

#export MAJOR=${VERSION_TABLE[0]}
#export MINOR=${VERSION_TABLE[1]}
#export BUGFIX=${VERSION_TABLE[2]}

export MAJOR=4
export MINOR=2
export BUGFIX=0

# Move artifacts to the stable directory.
SSH_REPO='ssh -o StrictHostKeyChecking=no ubuntu@srvi-repo.int.centreon.com'
$SSH_REPO mv "/srv/sources/map/testing/$PROJECT-$VERSION*" "/srv/sources/map/stable/"

# Upload installers to centreon download website
# BCO: /!\ This works only for version 4.2.X
# The url is specific for this version. We have to change it for new minor versions

BASE_INTERNAL_URL='/srv/sources/map/stable/$PROJECT-$VERSION/'
BASE_EXTERNAL_URL='s3://centreon-download/enterprises/centreon-map/centreon-map-4.2/centreon-map-4.2/9ae03a4457fa0ce578379a4e0c8b51f2/'

# Copy MacOS .tar.gz version
PRODUCT_NAME='Centreon-Map4.product-macosx.cocoa.x86_64.tar.gz'
$SSH_REPO aws s3 cp --acl public-read "$BASE_INTERNAL_URL/$PRODUCT_NAME" "$BASE_EXTERNAL_URL/$PRODUCT_NAME"

# Copy Windows .exe
PRODUCT_NAME='centreon-map4-desktop-client-$VERSION-SNAPSHOT-x86_64.exe'
EXTERNAL_PRODUCT_NAME='centreon-map4-desktop-client-$VERSION-x86_64.exe'
$SSH_REPO aws s3 cp --acl public-read "$BASE_INTERNAL_URL/$PRODUCT_NAME" "$BASE_EXTERNAL_URL/$EXTERNAL_PRODUCT_NAME"

# Copy Ubuntu .deb
PRODUCT_NAME='centreon-map4-desktop-client-$VERSION-SNAPSHOT-x86_64.deb'
EXTERNAL_PRODUCT_NAME='centreon-map4-desktop-client-$VERSION-x86_64.deb'
$SSH_REPO aws s3 cp --acl public-read "$BASE_INTERNAL_URL/$PRODUCT_NAME" "$BASE_EXTERNAL_URL/$EXTERNAL_PRODUCT_NAME"

# Copy p2 artifacts to remote server.
$SSH_REPO ssh -o StrictHostKeyChecking=no "map-repo@10.24.1.107" rm -rf "centreon-studio-repository/$MAJOR/$MINOR"
$SSH_REPO scp -r "/srv/p2/testing/$MAJOR/$MINOR" "map-repo@10.24.1.107:centreon-studio-repository/$MAJOR/$MINOR"

# Generate online documentation.
SSH_DOC="$SSH_REPO ssh -o StrictHostKeyChecking=no ubuntu@10.24.1.54"
$SSH_DOC "'source /srv/env/documentation/bin/activate ; /srv/prod/readthedocs.org/readthedocs/manage.py update_repos centreon-map-4 -V latest -p'"

# Synchronize RPMs.
$SSH_REPO /srv/scripts/sync-map.sh --confirm
