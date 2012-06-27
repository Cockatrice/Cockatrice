#!/bin/sh

FRAMEWORKS="Core Gui Multimedia Network Svg Xml"
DATE=`date '+%Y%m%d'`
#QTDIR="/Users/brukie/QtSDK/Desktop/Qt/474/gcc"
QTLIB="/Users/brukie/qt_leopard/lib"
QTPLUGINS="/Users/brukie/qt_leopard/plugins"
PROTOBUF="/Users/brukie/protobuf_leopard"
DIR=cockatrice_mac_$DATE
if [ -d $DIR ]; then echo "delete old dir first"; exit 1; fi

mkdir $DIR

cp -R build/cockatrice/cockatrice.app $DIR
cp -R build/oracle/oracle.app $DIR
mkdir $DIR/Frameworks
cp -R sounds zonebg $DIR
mkdir $DIR/translations
cp build/cockatrice/*.qm $DIR/translations
cp -R $QTPLUGINS $DIR/PlugIns
for f in $FRAMEWORKS; do
	cp -R $QTLIB/Qt"$f".framework $DIR/Frameworks
done
find $DIR/Frameworks -name '*debug*'|xargs -n 1 rm -f
find $DIR/Frameworks -name 'Headers'|xargs -n 1 rm -rf
find $DIR/PlugIns -name '*debug*'|xargs -n 1 rm -f
cp $PROTOBUF/lib/libprotobuf.7.dylib $DIR/Frameworks

cd $DIR
for f in $FRAMEWORKS; do
	echo "Framework $f"
	echo "step 1"
	install_name_tool -id @executable_path/../../../Frameworks/Qt"$f".framework/Versions/4/Qt"$f" Frameworks/Qt"$f".framework/Versions/4/Qt"$f"
	for g in $FRAMEWORKS; do
		install_name_tool -change $QTLIB/Qt"$f".framework/Versions/4/Qt"$f" @executable_path/../../../Frameworks/Qt"$f".framework/Versions/4/Qt"$f" Frameworks/Qt"$g".framework/Versions/4/Qt"$g"
	done
	echo "step 2"
	for g in cockatrice oracle; do
		install_name_tool -change $QTLIB/Qt"$f".framework/Versions/4/Qt"$f" @executable_path/../../../Frameworks/Qt"$f".framework/Versions/4/Qt"$f" "$g".app/Contents/MacOS/$g
	done
	echo "step 3"
	for g in `find . -name '*.dylib'`; do
		install_name_tool -change $QTLIB/Qt"$f".framework/Versions/4/Qt"$f" @executable_path/../../../Frameworks/Qt"$f".framework/Versions/4/Qt"$f" "$g"
	done
done

install_name_tool -id @executable_path/../../../libprotobuf.7.dylib Frameworks/libprotobuf.7.dylib
install_name_tool -change $PROTOBUF/lib/libprotobuf.7.dylib @executable_path/../../../Frameworks/libprotobuf.7.dylib cockatrice.app/Contents/MacOS/cockatrice

cd ..

VOL=cockatrice_mac_${DATE}
DMG="tmp-$VOL.dmg"

SIZE=`du -sk $DIR | cut -f1`
SIZE=$((${SIZE}/1000+10))
hdiutil create "$DMG" -megabytes ${SIZE} -ov -type UDIF -fs HFS+ -volname "$VOL"

hdid "$DMG"
cp -R "${DIR}" "/Volumes/$VOL"
hdiutil detach "/Volumes/$VOL"
#osascript -e "tell application "Finder" to eject disk "$VOL"" && 

# convert to compressed image, delete temp image
rm -f "${VOL}.dmg"
hdiutil convert "$DMG" -format UDZO -o "${VOL}.dmg"
rm -f "$DMG"
