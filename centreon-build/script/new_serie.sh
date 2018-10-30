#!/bin/sh

# Check arguments.
if [ -z "$1" -o -z "$2" ] ; then
  echo "USAGE:     $0 <OLDSERIE> <NEWSERIE>"
  echo "  example: $0 18.10 19.04"
  exit 1
fi

# Copy and tweak serie directories.
for olddir in `find . -type d -name $1` ; do
  newdir=`dirname $olddir`/$2
  cp -r "$olddir" "$newdir"
  find "$newdir" -type f | xargs sed -i -e "s#$1#$2#g"
  git add "$newdir"
done
