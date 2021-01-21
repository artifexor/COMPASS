export ARCH=x86_64
export QT_SELECT=5

cd /app/workspace/compass/
mkdir -p appimage/appdir/bin/

cp /usr/bin/compass_client appimage/appdir/bin/
mkdir -p appimage/appdir/lib/
cp /usr/lib/libcompass.a appimage/appdir/lib/

#cp -r /usr/lib64/osgPlugins-3.4.1 appimage/appdir/lib/
cp -r /usr/lib/osgPlugins-3.6.5 appimage/appdir/lib/
cp /usr/lib64/libosgEarth* appimage/appdir/lib/
cp /usr/lib64/osgdb_* appimage/appdir/lib/

#chrpath -r '$ORIGIN' appimage/appdir/lib/*
#chrpath -r '$ORIGIN/osgPlugins-3.4.1' appimage/appdir/lib/osgPlugins-3.4.1/*

mkdir -p appimage/appdir/compass/
cp -r data appimage/appdir/compass/
cp -r conf appimage/appdir/compass/

mkdir -p appimage/appdir/lib

/app/tools/linuxdeployqt-continuous-x86_64.AppImage --appimage-extract-and-run appimage/appdir/compass.desktop -appimage -bundle-non-qt-libs -verbose=2 -extra-plugins=iconengines,platformthemes/libqgtk3.so
#-qmake=/usr/lib/x86_64-linux-gnu/qt5/bin/qmake  -show-exclude-libs

cd /app/workspace/compass/docker
