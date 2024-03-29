#! /bin/bash
set -x
set -e

line=$(head -n 1 src/qoobar_app/version.h)
tmp=${line#*\"}   # remove prefix ending in "\""
VERSION=${tmp%\\*}

echo $VERSION

# VERSION=1.7.0

mkdir -p ~/deployment/qoobar

DEPLOY_DIR=$(mktemp -d -p ~/deployment/qoobar appimage-build-XXXXXX)

# make sure to clean up build dir, even if errors occur
cleanup () {
    if [ -d "$DEPLOY_DIR" ]; then
        rm -rf "$DEPLOY_DIR"
    fi
    # remove not-needed linuxdeployqt-5-x86_64.AppImage
    rm -f linuxdeployqt-5-x86_64.AppImage*
}

trap cleanup EXIT

# switch to src dir
pushd ~/build/qoobar

# configure build files with qmake
qmake INSTALL_ROOT="$DEPLOY_DIR"/usr

# build project and install files into DEPLOY_DIR
make -j$(nproc)
make install INSTALL_ROOT="$DEPLOY_DIR"/usr

# now, build AppImage using linuxdeployqt
# download linuxdeployqt 
wget https://github.com/probonopd/linuxdeployqt/releases/download/5/linuxdeployqt-5-x86_64.AppImage

# make executable
chmod +x linuxdeployqt*.AppImage

# build AppImage
./linuxdeployqt-5-x86_64.AppImage "$DEPLOY_DIR"/usr/share/applications/qoobar.desktop -appimage -extra-plugins=iconengines,platformthemes/libqgtk3.so

# move built AppImage back into original CWD
mv Qoobar*.AppImage ~/deployment/qoobar/Qoobar-"$VERSION".AppImage

