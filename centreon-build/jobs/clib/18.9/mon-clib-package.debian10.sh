# Prepare build directory.
rm -rf debuildir
mkdir debuildir
cp "${PROJECT}-${VERSION}.tar.gz" "debuildir/${PROJECT}_${VERSION}.orig.tar.gz"
cd debuildir
tar xzf "${PROJECT}_${VERSION}.orig.tar.gz"
cd ..
cp -r `dirname $0`/../../../packaging/clib/debian "debuildir/${PROJECT}-${VERSION}/"
sed -e "s/@VERSION@/${VERSION}/g" -e "s/@RELEASE@/${RELEASE}/g" < "debuildir/${PROJECT}-${VERSION}/debian/changelog.in" > "debuildir/${PROJECT}-${VERSION}/debian/changelog"

# Launch debuild.
containerid=`docker create ci.int.centreon.com:5000/mon-build-dependencies-18.9:debian10 sh -c "cd /usr/local/src/debuildir/${PROJECT}-${VERSION} && debuild -us -uc -i"`
docker cp debuildir "$containerid:/usr/local/src/debuildir"
docker start -a "$containerid"

# Send package to repository.
docker cp "$containerid:/usr/local/src/debuildir/${PROJECT}_${VERSION}-${RELEASE}_amd64.deb" .
put_internal_debs "18.9" "buster" *.deb

# Stop container.
docker stop "$containerid"
docker rm "$containerid"
