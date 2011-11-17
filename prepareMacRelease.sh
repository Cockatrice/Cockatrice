#!/bin/sh

FRAMEWORKS="Core Gui Multimedia Network Svg Xml Script"
DATE=`date '+%Y%m%d'`
#QTDIR="/Users/brukie/QtSDK/Desktop/Qt/474/gcc"
QTLIB="/usr/local/Qt4.7/lib"
QTPLUGINS="/usr/local/Qt4.7/plugins"
DIR=cockatrice_mac_$DATE
if [ -d $DIR ]; then echo "delete old dir first"; exit 1; fi

mkdir $DIR

cp -R cockatrice/cockatrice.app $DIR
cp -R oracle/oracle.app $DIR
mkdir $DIR/decks
mkdir $DIR/pics
mkdir $DIR/Frameworks
cp -R sounds zonebg $DIR
cp -R $QTPLUGINS $DIR/PlugIns
for f in $FRAMEWORKS; do
	cp -R $QTLIB/Qt"$f".framework $DIR/Frameworks
done
find $DIR/Frameworks -name '*debug*'|xargs -n 1 rm -f
find $DIR/Frameworks -name 'Headers'|xargs -n 1 rm -rf
find $DIR/PlugIns -name '*debug*'|xargs -n 1 rm -f


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
