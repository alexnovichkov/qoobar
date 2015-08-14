#!/bin/bash
make  install
#LANGUAGES="ru_RU"
#for lang in $LANGUAGES
#do
#  mkdir release/qoobar.app/Contents/Resources/${lang}
#done
PLUGINS="beatport autonumber discogs freedb gd3 musicbrainz playlists"
FRAMEWORKS="QtNetwork QtGui QtCore QtConcurrent QtWidgets"
path_to_qt="/Users/ab/QtSDK/Desktop/Qt/4.8.0/gcc/lib/"
#path_to_qt="/Users/admin/Qt5.2.0/5.2.0/clang_64/lib/"
version=4
for plugin in $PLUGINS
do
  echo "Processing ${plugin}"
  for framework in $FRAMEWORKS
  do
    framework_path="${framework}.framework/Versions/${version}/${framework}"
    echo "For framework ${framework}"
    install_name_tool -change ${path_to_qt}${framework_path} @executable_path/../Frameworks/${framework_path} release/qoobar.app/Contents/PlugIns/lib${plugin}.dylib
  done
done
