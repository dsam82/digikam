#! /bin/bash

# Script to bundle data using previously-built digiKam installation.
# and create a PKG file with Packages application (http://s.sudre.free.fr/Software/Packages/about.html)
# This script must be run as sudo
#
# Copyright (c) 2015,      Shanti, <listaccount at revenant dot org>
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-installer.full.log) 2>&1

#################################################################################################

echo "04-build-installer.sh : build digiKam bundle PKG."
echo "-------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksRunAsRoot
ChecksXCodeCLI
ChecksCPUCores
OsxCodeName
#RegisterRemoteServers

#################################################################################################

# Pathes rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:$ORIG_PATH

DKRELEASEID=`cat $ORIG_WD/data/RELEASEID.txt`

#################################################################################################
# Build icons-set ressource

echo -e "\n---------- Build icons-set ressource\n"

cd $ORIG_WD/icon-rcc

rm -f CMakeCache.txt > /dev/null

cp -f $ORIG_WD/../../../bootstrap.macports .

cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
      -DCMAKE_BUILD_TYPE=debug \
      -DCMAKE_COLOR_MAKEFILE=ON \
      -Wno-dev \
      .

make -j$CPU_CORES

cd $ORIG_WD

#################################################################################################
# Configurations

# Directory where this script is located (default - current directory)
BUILDDIR="$PWD"

# Directory where Packages project files are located
PROJECTDIR="$BUILDDIR/installer"

# Staging area where files to be packaged will be copied
TEMPROOT="$BUILDDIR/$INSTALL_PREFIX"

# Applications to be launched directly by user (create launch scripts)
KDE_MENU_APPS="\
digikam \
showfoto \
"

# Paths to search for applications above
KDE_APP_PATHS="\
Applications/KF5 \
"

# Other apps - non-MacOS binaries & libraries to be included with required dylibs
OTHER_APPS="\
Applications/KF5/digikam.app/Contents/MacOS/digikam \
Applications/KF5/showfoto.app/Contents/MacOS/showfoto \
lib/plugins/imageformats/*.so \
lib/plugins/digikam/bqm/*.so \
lib/plugins/digikam/generic/*.so \
lib/plugins/digikam/editor/*.so \
lib/mariadb/bin/mysql \
lib/mariadb/bin/mysqld \
lib/mariadb/bin/my_print_defaults \
lib/mariadb/bin/mysqladmin \
lib/mariadb/bin/mysqltest \
lib/mariadb/mysql/*.dylib \
lib/mariadb/plugin/*.so \
bin/kbuildsycoca5 \
libexec/qt5/plugins/imageformats/*.dylib \
libexec/qt5/plugins/sqldrivers/*.dylib \
libexec/qt5/plugins/printsupport/*.dylib \
libexec/qt5/plugins/platforms/*.dylib \
libexec/qt5/plugins/mediaservice/*.dylib \
libexec/qt5/plugins/iconengines/*.dylib \
libexec/qt5/plugins/audio/*.dylib \
libexec/qt5/plugins/position/*.dylib \
libexec/qt5/plugins/geoservices/*.dylib \
"

#lib/sane/*.so \

binaries="$OTHER_APPS"

# Additional Files/Directories - to be copied recursively but not checked for dependencies
OTHER_DIRS="\
lib/plugins \
lib/libgphoto2 \
lib/libgphoto2_port \
lib/mariadb \
share/mariadb \
etc/xdg \
"

#etc/sane.d \

# Additional Data Directories - to be copied recursively
OTHER_DATA="\
share/applications \
share/OpenCV \
share/k* \
share/lensfun \
share/mime \
Library/Application/ \
Marble.app/Contents/Resources/ \
Marble.app/Contents/MacOS/resources/ \
"

# Packaging tool paths
PACKAGESBUILD="/usr/local/bin/packagesbuild"
RECURSIVE_LIBRARY_LISTER="$BUILDDIR/rll.py"

echo "digiKam version: $DKRELEASEID"

# ./installer sub-dir must be writable by root
chmod 777 ${PROJECTDIR}

#################################################################################################
# Check if Packages CLI tools are installed

if [[ (! -f "$PACKAGESBUILD") ]] ; then
    echo "Packages CLI tool is not installed"
    echo "See http://s.sudre.free.fr/Software/Packages/about.html for details."
    exit 1
else
    echo "Check Packages CLI tool passed..."
fi

#################################################################################################
# Create temporary dir to build package contents

if [ -d "$TEMPROOT" ] ; then
    echo "---------- Removing temporary packaging directory $TEMPROOT"
    rm -rf "$TEMPROOT"
fi

echo "Creating $TEMPROOT"
mkdir -p "$TEMPROOT/Applications/digiKam"

#################################################################################################
# Prepare applications for MacOS

echo "---------- Preparing Applications for MacOS"

for app in $KDE_MENU_APPS ; do
    echo "  $app"

    # Look for application

    for searchpath in $KDE_APP_PATHS ; do

        # Copy the application if it is found (create directory if necessary)

        if [ -d "$INSTALL_PREFIX/$searchpath/$app.app" ] ; then
            echo "    Found $app in $INSTALL_PATH/$searchpath"

        # Create destination directory if necessary and copy app

        if [ ! -d "$TEMPROOT/$searchpath" ] ; then 
            echo "    Creating $TEMPROOT/$searchpath"
            mkdir -p "$TEMPROOT/$searchpath"
        fi

        echo "    Copying $app"
        cp -pr "$INSTALL_PREFIX/$searchpath/$app.app" "$TEMPROOT/$searchpath/"

        # Add executable to list of binaries for which we need to collect dependencies for

        binaries="$binaries $searchpath/$app.app/Contents/MacOS/$app"

        # If application is to be run by user, create Applescript launcher to
        # load kbuildsycoca5. Set DYLD_IMAGE_SUFFIX if built with debug variant

        if [[ $KDE_MENU_APPS == *"$app"* ]] ; then
            echo "    Creating launcher script for $app"

            # Debug variant needs DYLD_IMAGE_SUFFIX="_debug set at runtime

            if [[ $DK_DEBUG = 1 ]] ; then
                DYLD_ENV_CMD="DYLD_IMAGE_SUFFIX=_debug "
            else
                DYLD_ENV_CMD=""
            fi

            # ------ Create application launcher script
            # Partially derived from https://discussions.apple.com/thread/3934912 and
            # http://stackoverflow.com/questions/16064957/how-to-check-in-applescript-if-an-app-is-running-without-launching-it-via-osa
            # and https://discussions.apple.com/thread/4059113

            cat << EOF | osacompile -o "$TEMPROOT/Applications/digiKam/$app.app"
#!/usr/bin/osascript

log "Running $DYLD_ENV_CMD $INSTALL_PREFIX/bin/kbuildsycoca5"
do shell script "$DYLD_ENV_CMD $INSTALL_PREFIX/bin/kbuildsycoca5"

do shell script "$DYLD_ENV_CMD open $INSTALL_PREFIX/$searchpath/$app.app"
EOF
                # ------ End application launcher script

                # Get application icon for launcher. If no icon file matches pattern app_SRCS.icns, grab the first icon

                if [ -f "$INSTALL_PREFIX/$searchpath/$app.app/Contents/Resources/${app}_SRCS.icns" ] ; then
                    echo "    Found icon for $app launcher"
                    cp -p "$INSTALL_PREFIX/$searchpath/$app.app/Contents/Resources/${app}_SRCS.icns" "$TEMPROOT/Applications/digiKam/$app.app/Contents/Resources/applet.icns"
                else
                    for icon in "$INSTALL_PREFIX/$searchpath/$app.app/"Contents/Resources/*.icns ; do
                        echo "    Using icon for $app launcher: $icon"
                        cp -p "$icon" "$TEMPROOT/Applications/digiKam/$app.app/Contents/Resources/applet.icns"
                        break
                    done
                fi

                chmod 755 "$TEMPROOT/Applications/digiKam/$app.app"
            fi

            # Don't keep looking through search paths once we've found the app
            break
        fi

    done

done

#################################################################################################
# Collect dylib dependencies for all binaries,
# then copy them to the staging area (creating directories as required)

echo "---------- Collecting dependencies for applications, binaries, and libraries:"

cd "$INSTALL_PREFIX"

"$RECURSIVE_LIBRARY_LISTER" $binaries | sort -u | \
while read lib ; do
    lib="`echo $lib | sed "s:$INSTALL_PREFIX/::"`"

    if [ ! -e "$TEMPROOT/$lib" ] ; then
        dir="${lib%/*}"

        if [ ! -d "$TEMPROOT/$dir" ] ; then
            echo "  Creating $TEMPROOT/$dir"
            mkdir -p "$TEMPROOT/$dir"
        fi

        echo "  $lib"
        cp -aH "$INSTALL_PREFIX/$lib" "$TEMPROOT/$dir/"
    fi
done

#################################################################################################
# Copy non-binary files and directories, creating parent directories if needed

echo "---------- Copying binary files..."

for path in $OTHER_APPS ; do
    dir="${path%/*}"

    if [ ! -d "$TEMPROOT/$dir" ] ; then
        echo "  Creating $TEMPROOT/$dir"
        mkdir -p "$TEMPROOT/$dir"
    fi

    echo "  Copying $path"
    cp -a "$INSTALL_PREFIX/$path" "$TEMPROOT/$dir/"
done

echo "---------- Copying directory contents..."

for path in $OTHER_DIRS ; do
    dir="${path%/*}"

    if [ ! -d "$TEMPROOT/$dir" ] ; then
        echo "  Creating $TEMPROOT/$dir"
        mkdir -p "$TEMPROOT/$dir"
    fi

    echo "   Copying $path"
    cp -a "$INSTALL_PREFIX/$path" "$TEMPROOT/$dir/"
done

echo "---------- Copying data files..."

# Special case with data dirs. QStandardPaths::GenericDataLocation was patched everywhere
# in source code by QStandardPaths::AppDataLocation
for path in $OTHER_DATA ; do
    echo "   Copying $path"
    cp -a "$INSTALL_PREFIX/$path" "$TEMPROOT/Applications/KF5/digikam.app/Contents/Resources/"
done

echo "---------- Copying i18n..."

i18nprefix=$INSTALL_PREFIX/share/
cd $i18nprefix

FILES=$(cat $ORIG_WD/logs/build-extralibs.full.log | grep "$INSTALL_PREFIX/share/locale/" | cut -d' ' -f3  | awk '{sub("'"$i18nprefix"'","")}1')

for FILE in $FILES ; do
    rsync -R "./$FILE" "$TEMPROOT/Applications/KF5/digikam.app/Contents/Resources/"
done

FILES=$(cat $ORIG_WD/logs/build-digikam.full.log | grep "$INSTALL_PREFIX/share/locale/" | cut -d' ' -f3  | awk '{sub("'"$i18nprefix"'","")}1')

for FILE in $FILES ; do
    rsync -R "./$FILE" "$TEMPROOT/Applications/KF5/digikam.app/Contents/Resources/"
done

# Showfoto resources dir must be merged with digiKam.
cp -a "$TEMPROOT/Applications/KF5/showfoto.app/Contents/Resources/" "$TEMPROOT/Applications/KF5/digikam.app/Contents/Resources/"
rm -rf "$TEMPROOT/Applications/KF5/showfoto.app/Contents/Resources"

# A symbolic link to install path where is installed digiKam resources will be used for Showfoto.
ln -s "$INSTALL_PREFIX/Applications/KF5/digikam.app/Contents/Resources" "$TEMPROOT/Applications/KF5/showfoto.app/Contents/Resources"

cd "$ORIG_WD"

#################################################################################################
# Move digiKam and KF5 run-time plugins to the right place

cp -a $TEMPROOT/lib/plugins $TEMPROOT/libexec/qt5/
rm -rf $TEMPROOT/lib/plugins

#################################################################################################
# Create package pre-install script

echo "---------- Create package pre-install script"

# Delete /Applications entries, delete existing installation

cat << EOF > "$PROJECTDIR/preinstall"
#!/bin/bash

if [ -d /Applications/digiKam ] ; then
    echo "Removing digikam from Applications folder"
    rm -r /Applications/digiKam
fi

if [ -d "$INSTALL_PREFIX" ] ; then
    echo "Removing $INSTALL_PREFIX"
    rm -rf "$INSTALL_PREFIX"
fi
EOF

# Pre-install script need to be executable

chmod 755 "$PROJECTDIR/preinstall"

#################################################################################################
# Create package post-install script

echo "---------- Create package post-install script"

# Creates Applications menu icons

cat << EOF > "$PROJECTDIR/postinstall"
#!/bin/bash

[[ ! -d /Applications/digiKam ]] && mkdir "/Applications/digiKam"

for app in $INSTALL_PREFIX/Applications/digiKam/*.app ; do
    ln -s "\$app" /Applications/digiKam/\${app##*/}
done
EOF

# Post-install script need to be executable

chmod 755 "$PROJECTDIR/postinstall"

#################################################################################################
# Copy icons-set resource files.

cp $ORIG_WD/icon-rcc/breeze.rcc $TEMPROOT/Applications/KF5/digikam.app/Contents/Resources/
cp $ORIG_WD/icon-rcc/breeze-dark.rcc $TEMPROOT/Applications/KF5/digikam.app/Contents/Resources/

#################################################################################################
# Cleanup symbols in binary files to free space.

echo -e "\n---------- Strip symbols in binary files\n"

if [[ $DK_DEBUG = 1 ]] ; then
    find $TEMPROOT -name "*.so"    | grep -Ev '(digikam|showfoto|exiv2)' | xargs strip -SXx
    find $TEMPROOT -name "*.dylib" | grep -Ev '(digikam|showfoto|exiv2)' | xargs strip -SXx
else
    find $TEMPROOT -name "*.so"    | xargs strip -SXx
    find $TEMPROOT -name "*.dylib" | xargs strip -SXx
fi

#################################################################################################
# Relocate binary files
# For details, see these urls:
# https://stackoverflow.com/questions/9263256/can-you-please-help-me-understand-how-mach-o-libraries-work-in-mac-os-x
# https://matthew-brett.github.io/docosx/mac_runtime_link.html

# Not yet finalized !
if [ ]; then

echo -e "\n---------- Relocate binary files"

# relocate dynamic libraries with rpath

DYLIBFILES=(`find $TEMPROOT/lib -name "*.dylib"`)

RelocateBinaries DYLIBFILES[@]

# relocate library executables and system objects files with rpath.
# This include all binary files with extension as all Qt libraries.

LIBEXECFILES=(`find $TEMPROOT/libexec -type f -perm +ugo+x`)

RelocateBinaries LIBEXECFILES[@]

# relocate main executable with rpath.

MAINFILES="\
$TEMPROOT/Applications/KF5/digikam.app/Contents/MacOS/digikam \
$TEMPROOT/Applications/KF5/showfoto.app/Contents/MacOS/showfoto \
$TEMPROOT/bin/kbuildsycoca5 \
"

RelocateBinaries MAINFILES[@]

for APP in $MAINFILES ; do
    install_name_tool -add_rpath @executable_path/.. $APP
    install_name_tool -add_rpath @executable_path/../.. $APP
    install_name_tool -add_rpath @executable_path/../../.. $APP
    install_name_tool -add_rpath @executable_path/../../../.. $APP
    install_name_tool -add_rpath @executable_path/../../../../.. $APP
    install_name_tool -add_rpath @executable_path/../../../../../.. $APP
done

fi

#################################################################################################
# Build PKG file

echo "---------- Create MacOS package for digiKam $DKRELEASEID"

mkdir -p $ORIG_WD/bundle
rm -f $ORIG_WD/bundle/* || true

TARGET_INSTALLER=digiKam-$DKRELEASEID$DK_EPOCH-MacOS-x86-64$DEBUG_SUF.pkg
TARGET_PKG_FILE=$BUILDDIR/bundle/$TARGET_INSTALLER
echo -e "Target PKG file : $TARGET_PKG_FILE"

$PACKAGESBUILD -v "$PROJECTDIR/digikam.pkgproj" --package-version "$DKRELEASEID"

mv "$PROJECTDIR/build/digikam.pkg" "$TARGET_PKG_FILE"

#################################################################################################
# Show resume information and future instructions to host PKG file to remote server

echo -e "\n---------- Compute package checksums for digiKam $DKRELEASEID\n" >  $TARGET_PKG_FILE.sum
echo    "File       : $TARGET_PKG_FILE"                                     >> $TARGET_PKG_FILE.sum
echo -n "Size       : "                                                     >> $TARGET_PKG_FILE.sum
du -h "$TARGET_PKG_FILE"        | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum
echo -n "MD5 sum    : "                                                     >> $TARGET_PKG_FILE.sum
md5 -q "$TARGET_PKG_FILE"                                                   >> $TARGET_PKG_FILE.sum
echo -n "SHA1 sum   : "                                                     >> $TARGET_PKG_FILE.sum
shasum -a1 "$TARGET_PKG_FILE"   | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum
echo -n "SHA256 sum : "                                                     >> $TARGET_PKG_FILE.sum
shasum -a256 "$TARGET_PKG_FILE" | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum

# Checksums to post on Phabricator at release time.
shasum -a256 "$TARGET_PKG_FILE" > $BUILDDIR/bundle/sha256_release.sum

if [[ $DK_SIGN = 1 ]] ; then

    cat ~/.gnupg/dkorg-gpg-pwd.txt | gpg --batch --yes --passphrase-fd 0 -sabv "$TARGET_PKG_FILE"
    mv -f $TARGET_PKG_FILE.asc $TARGET_PKG_FILE.sig

    echo    "File       : $TARGET_PKG_FILE.sig"                                     >> $TARGET_PKG_FILE.sum
    echo -n "Size       : "                                                         >> $TARGET_PKG_FILE.sum
    du -h "$TARGET_PKG_FILE.sig"        | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum
    echo -n "MD5 sum    : "                                                         >> $TARGET_PKG_FILE.sum
    md5 -q "$TARGET_PKG_FILE.sig"                                                   >> $TARGET_PKG_FILE.sum
    echo -n "SHA1 sum   : "                                                         >> $TARGET_PKG_FILE.sum
    shasum -a1 "$TARGET_PKG_FILE.sig"   | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum
    echo -n "SHA256 sum : "                                                         >> $TARGET_PKG_FILE.sum
    shasum -a256 "$TARGET_PKG_FILE.sig" | { read first rest ; echo $first ; }       >> $TARGET_PKG_FILE.sum

    # Checksums to post on Phabricator at release time.
    shasum -a256 "$TARGET_PKG_FILE.sig" >> $BUILDDIR/bundle/sha256_release.sum

fi

cat $TARGET_PKG_FILE.sum

if [[ $DK_UPLOAD = 1 ]] ; then

    echo -e "---------- Cleanup older bundle Package files from files.kde.org repository \n"

    ssh $DK_UPLOADURL rm -f $DK_UPLOADDIR*-MacOS-x86-64*.pkg*

    echo -e "---------- Upload new bundle Package files to files.kde.org repository \n"

    rsync -r -v --progress -e ssh $BUILDDIR/bundle/$TARGET_INSTALLER     $DK_UPLOADURL:$DK_UPLOADDIR
    scp $BUILDDIR/bundle/$TARGET_INSTALLER.sum $DK_UPLOADURL:$DK_UPLOADDIR

    if [[ $DK_SIGN = 1 ]] ; then
        scp $BUILDDIR/bundle/$TARGET_INSTALLER.sig $DK_UPLOADURL:$DK_UPLOADDIR
    fi

else
    echo -e "\n------------------------------------------------------------------"
    curl https://download.kde.org/README_UPLOAD
    echo -e "------------------------------------------------------------------\n"
fi

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
