line=$(head -n 1 src/qoobar_app/version.h)
tmp=${line#*\"}   # remove prefix ending in "\""
VERSION=${tmp%\\*}

do_debuild () {
  debuild -S -sa
  #cd ..
  #dput ppa:aleksej-novichkov/ppa "qoobar_$VERSION_source.changes"
}

work_dir=qoobar-$VERSION
echo $work_dir

# cleanup
cleanup () {
   if [ -d "qoobar-$VERSION" ]; then
     rm -rf "qoobar-$VERSION"
   fi
}

trap cleanup EXIT

cleanup

git clone https://github.com/alexnovichkov/qoobar.git "$work_dir"
cd $work_dir

qmake INSTALL_ROOT=/usr
make clean

# reading last line of file
# tag=$( tail -n 1 history.txt )

for ubuntu in impish focal bionic ; do
   rm -rf debian

   DEBFULLNAME="Alex Novichkov" dh_make --single -c gpl3 --native -e aleksej.novichkov@gmail.com

   #edit 
   cp -f debian1/control-qt5 debian/control
   cp -f debian1/copyright debian
   if [ "$ubuntu" = "bionic" ]; then
      echo 11 > debian/compat
      cp -f debian1/control-bionic debian/control
   fi
   rm debian/qoobar.doc-base.EX
   rm debian/post*
   rm debian/pre*
   cp -f debian1/qoobar.doc-base debian
   rm debian/manpage*
   rm debian/qoobar.cron*
   rm debian/README.source
   rm debian/README.Debian
   rm debian/salsa*

   # wait to complete editing debian folder
   echo "Now edit files in the debian folder:"
   echo "Edit changelog and README"
   echo "Then press y to continue"
   echo "Press n to abort"
   read -n 1 answer
   if [ "$answer" = "y" ]; then
      do_debuild
   else
      exit 1
   fi
done