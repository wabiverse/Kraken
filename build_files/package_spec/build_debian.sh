#!/bin/sh
# Builds a debian package from SVN source.
#
# For paralelle builds use:
#  DEB_BUILD_OPTIONS="parallel=5" sh build_files/package_spec/build_debian.sh

# this needs to run in the root dir.
cd $(dirname $0)/../../
rm -rf debian
cp -a build_files/package_spec/debian .


# Get values from covah to use in debian/changelog.
# value may be formatted: 35042:35051M
COVAH_REVISION=$(svnversion | cut -d: -f2 | tr -dc 0-9)

covah_version=$(grep COVAH_VERSION source/covah/covakernel/CKE_version.h | tr -dc 0-9)
covah_version_char=$(sed -ne 's/.*COVAH_VERSION.*\([a-z]\)$/\1/p' source/covah/covakernel/CKE_version.h)
COVAH_VERSION=$(expr $covah_version / 100).$(expr $covah_version % 100)

# map the version a -> 1, to conform to debian naming convention
# not to be confused with covah's internal subversions
if [ "$covah_version_char" ]; then
    COVAH_VERSION=${COVAH_VERSION}.$(expr index abcdefghijklmnopqrstuvwxyz $covah_version_char)
fi

DEB_VERSION=${COVAH_VERSION}

# update debian/changelog
dch -b -v $DEB_VERSION "New upstream SVN snapshot."


# run the rules makefile
rm -rf get-orig-source
debian/rules get-orig-source SVN_URL=git@github.com:ForgeXYZ/Forge-SDK.git
mv *.gz ../

# build the package
debuild -i -us -uc -b


# remove temp dir
rm -rf debian
