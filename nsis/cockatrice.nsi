!include "MUI2.nsh"
!include "FileFunc.nsh"

!define /date TIMESTAMP "%Y%m%d" 
!searchparse /file ../build/cockatrice/version_string.cpp '= "' VERSION '";'

Name "Cockatrice"
OutFile "cockatrice_win32_${TIMESTAMP}_git-${VERSION}.exe"
SetCompressor /SOLID lzma
InstallDir "$PROGRAMFILES\Cockatrice"

; set the Qt install dir here (and make sure you use the latest 4.8 version for packaging)
!define QTDIR "C:\Qt\4.8.6"

!define MUI_ABORTWARNING
!define MUI_WELCOMEFINISHPAGE_BITMAP "leftimage.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "leftimage.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "headerimage.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "headerimage.bmp"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of Cockatrice.$\r$\n$\r$\nClick Next to continue."
!define MUI_FINISHPAGE_RUN "$INSTDIR/oracle.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Run card database downloader now"
!define MUI_FINISHPAGE_RUN_PARAMETERS "-dlsets"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "Application" SecApplication
	SetShellVarContext all
	SetOutPath "$INSTDIR"
	File ..\build\cockatrice\Release\cockatrice.exe
	File ..\build\oracle\Release\oracle.exe
	File ..\doc\usermanual\Usermanual.pdf
	File ..\build\protobuf-2.5.0\protobuf-2.5.0\vsprojects\Release\libprotobuf.lib
	File "${QTDIR}\bin\QtCore4.dll"
	File "${QTDIR}\bin\QtGui4.dll"
	File "${QTDIR}\bin\QtNetwork4.dll"
	File "${QTDIR}\bin\QtSvg4.dll"
	File "${QTDIR}\bin\QtXml4.dll"
	File "${QTDIR}\bin\QtMultimedia4.dll"

	SetOutPath "$INSTDIR\zonebg"
	File /r ..\zonebg\*.*
	
	SetOutPath "$INSTDIR\plugins"
	SetOutPath "$INSTDIR\plugins\codecs"
	File "${QTDIR}\plugins\codecs\qcncodecs4.dll"
	File "${QTDIR}\plugins\codecs\qjpcodecs4.dll"
	File "${QTDIR}\plugins\codecs\qkrcodecs4.dll"
	File "${QTDIR}\plugins\codecs\qtwcodecs4.dll"
	SetOutPath "$INSTDIR\plugins\iconengines"
	File "${QTDIR}\plugins\iconengines\qsvgicon4.dll"
	SetOutPath "$INSTDIR\plugins\imageformats"
	File "${QTDIR}\plugins\imageformats\qjpeg4.dll"
	File "${QTDIR}\plugins\imageformats\qsvg4.dll"

	SetOutPath "$INSTDIR\sounds"
	File /r ..\sounds\*.*
	
	SetOutPath "$INSTDIR\translations"
	File /r ..\build\cockatrice\*.qm

	WriteUninstaller "$INSTDIR\uninstall.exe"
	${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice" "DisplayName" "Cockatrice"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice" "DisplayIcon" "$INSTDIR\cockatrice.exe"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice" "EstimatedSize" "$0"
SectionEnd

Section "Update configuration" SecUpdateConfig
	WriteRegStr HKCU "Software\Cockatrice\Cockatrice\paths" "carddatabase" "$INSTDIR\cards.xml"
	WriteRegStr HKCU "Software\Cockatrice\Cockatrice\paths" "decks" "$INSTDIR\decks"
	WriteRegStr HKCU "Software\Cockatrice\Cockatrice\paths" "pics" "$INSTDIR\pics"
	WriteRegStr HKCU "Software\Cockatrice\Cockatrice\sound" "path" "$INSTDIR\sounds"
SectionEnd

Section "Start menu item" SecStartMenu
	createDirectory "$SMPROGRAMS\Cockatrice"
	createShortCut "$SMPROGRAMS\Cockatrice\Cockatrice.lnk" "$INSTDIR\cockatrice.exe" '--debug-output'
	createShortCut "$SMPROGRAMS\Cockatrice\Oracle.lnk" "$INSTDIR\oracle.exe"
	createShortCut "$SMPROGRAMS\Cockatrice\Usermanual.lnk" "$INSTDIR\Usermanual.pdf"
SectionEnd

Section Uninstall
SetShellVarContext all
	RMDir /r "$INSTDIR\zonebg"
	RMDir /r "$INSTDIR\plugins"
	RMDir /r "$INSTDIR\sounds"
	RMDir /r "$INSTDIR\translations"
	Delete "$INSTDIR\uninstall.exe"
	Delete "$INSTDIR\cockatrice.exe"
	Delete "$INSTDIR\oracle.exe"
	Delete "$INSTDIR\Usermanual.pdf"
	Delete "$INSTDIR\libprotobuf.lib"
	Delete "$INSTDIR\QtCore4.dll"
	Delete "$INSTDIR\QtGui4.dll"
	Delete "$INSTDIR\QtNetwork4.dll"
	Delete "$INSTDIR\QtSvg4.dll"
	Delete "$INSTDIR\QtXml4.dll"
	Delete "$INSTDIR\QtMultimedia4.dll"
	RMDir "$INSTDIR"

	RMDir /r "$SMPROGRAMS\Cockatrice"

	DeleteRegKey HKCU "Software\Cockatrice"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice"
SectionEnd

LangString DESC_SecApplication ${LANG_ENGLISH} "Cockatrice program files"
LangString DESC_SecUpdateConfig ${LANG_ENGLISH} "Update the paths in the application settings according to the installation paths."
LangString DESC_SecStartMenu ${LANG_ENGLISH} "Create start menu items for Cockatrice and Oracle."
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SecApplication} $(DESC_SecApplication)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecUpdateConfig} $(DESC_SecUpdateConfig)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

