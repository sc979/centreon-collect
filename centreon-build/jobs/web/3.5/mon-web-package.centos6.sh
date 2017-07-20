# Create input and output directories for docker-rpm-builder.
rm -rf input
mkdir input
rm -rf output
mkdir output

# Retrieve sources.
mv "centreon-$VERSION.tar.gz" input/
cp -r packaging-centreon-web/rpm/centreon-dev.spectemplate input/
cp -r packaging-centreon-web/src/dev/* input/

# Build RPMs.
docker-rpm-builder dir --sign-with `dirname $0`/../../ces.key ci.int.centreon.com:5000/mon-build-dependencies:centos6 input output

# Publish RPMs.
put_internal_rpms "3.5" "el6" "noarch" "web" "$PROJECT-$VERSION-$RELEASE" output/noarch/*.rpm
