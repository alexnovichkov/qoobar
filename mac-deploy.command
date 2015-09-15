#!/bin/bash
make  install
#LANGUAGES="ru_RU"
#for lang in $LANGUAGES
#do
#  mkdir release/qoobar.app/Contents/Resources/${lang}
#done
PLUGINS="beatport autonumber discogs freedb gd3 musicbrainz playlists"
FRAMEWORKS="QtNetwork QtGui QtCore QtConcurrent QtWidgets QtMacExtras QtPrintSupport QtDBus"
IMAGEPLUGINS="libqgif libqicns libqjpeg libqico"
#path_to_qt="/Users/ab/QtSDK/Desktop/Qt/4.8.0/gcc/lib/"
path_to_qt="/Users/admin/Qt5.5.0/5.5/clang_64/lib/"
version=5
for plugin in $PLUGINS
do
  echo "Processing ${plugin}"
  for framework in $FRAMEWORKS
  do
    framework_path="${framework}.framework/Versions/${version}/${framework}"
    echo "For framework ${framework}"
    install_name_tool -change @rpath/${framework_path} @executable_path/../Frameworks/${framework_path} release/qoobar.app/Contents/PlugIns/lib${plugin}.dylib
  done
done

for framework in $FRAMEWORKS
do
  framework_path="${framework}.framework/Versions/${version}/${framework}"
  full_path="@executable_path/../Frameworks/${framework_path}"
  echo "Changing executable paths"
  install_name_tool -id ${full_path} release/qoobar.app/Contents/Frameworks/${framework_path}
  install_name_tool -change @rpath/${framework_path} ${full_path} release/qoobar.app/Contents/MacOS/qoobar

  echo "Changing frameworks paths"
  for framework1 in $FRAMEWORKS
  do
  	framework1_path="${framework1}.framework/Versions/${version}/${framework1}"
    install_name_tool -change @rpath/${framework1_path} @executable_path/../Frameworks/${framework1_path} release/qoobar.app/Contents/Frameworks/${framework_path}
  done

  echo "Changing Qt plugins paths"
  install_name_tool -change @rpath/${framework_path} ${full_path} release/qoobar.app/Contents/PlugIns/platforms/libqcocoa.dylib
  for imageplugin in $IMAGEPLUGINS
  do
    install_name_tool -change @rpath/${framework_path} ${full_path} release/qoobar.app/Contents/PlugIns/imageformats/${imageplugin}.dylib
  done
done
