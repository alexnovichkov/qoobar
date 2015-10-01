#!/bin/bash
path_to_qt="/Users/admin/Qt5.5.0/5.5/clang_64/bin/"

make  install
${path_to_qt}macdeployqt release/qoobar.app

cp -Rf mac_os/discid.framework release/qoobar.app/Contents/Frameworks
cp -Rf mac_os/Sparkle.framework release/qoobar.app/Contents/Frameworks

UTILITIES="shntool flac mac mp3gain enca vorbisgain metaflac replaygain mppdec aacgain wvgain"
for util in $UTILITIES
do
	cp -f mac_os/${util} release/qoobar.app/Contents/Resources
done
cp -f mac_os/*.dylib release/qoobar.app/Contents/Resources

# installing help
cp -f html/Qoobar.help release/qoobar.app/Contents/Resources
hiutil -C ...