; Installer log helpers: CockLog, CockLogWrite and CockLogListing. Included
; before the first function that logs, because CockLog is a macro and is expanded
; at the point of use. Nothing here returns anything; CockLogWrite uses
; $LogMessage/$LogHandle, CockLogListing the $Log* match variables.

; Installer log
; Every run appends a step-by-step trace to a file so a failed upgrade on a
; user's machine can be diagnosed after the fact. Kept in $TEMP rather than in
; $INSTDIR because the previous version's uninstaller removes the whole install
; directory part-way through an upgrade. Read it back with:
;   notepad %TEMP%\Cockatrice-installer.log
!macro CockLog text
    Push "${text}"
    Call CockLogWrite
!macroend

Function CockLogWrite
    Pop $LogMessage
    FileOpen $LogHandle "$InstallLogPath" a
    ${If} $LogHandle == ""
        Return
    ${EndIf}
    ; FileOpen places the pointer at the start of the file in every mode, so
    ; 'a' alone would overwrite the log from the beginning on each line. Seek
    ; past the existing content first to actually append.
    FileSeek $LogHandle 0 END
    FileWrite $LogHandle "$LogMessage$\r$\n"
    FileClose $LogHandle
FunctionEnd

; Logs every file matching $INSTDIR\<mask>. Used to record the DLL inventory
; before and after the upgrade purge, which is what shows whether Qt6Gui.dll was
; on disk and what (if anything) survived the deletes.
!macro CockLogListing label mask
    Push "${label}"
    Push "$INSTDIR\${mask}"
    Call CockLogListing
!macroend

Function CockLogListing
    Pop $LogMatchPattern
    Pop $LogMatchLabel
    !insertmacro CockLog "listing [$LogMatchLabel] pattern=$LogMatchPattern"
    FindFirst $LogFindHandle $LogFindName "$LogMatchPattern"
    ${If} $LogFindHandle == ""
        !insertmacro CockLog "listing [$LogMatchLabel] (no matches)"
        Return
    ${EndIf}
    log_listing_loop:
        !insertmacro CockLog "listing [$LogMatchLabel] $LogFindName"
        ClearErrors
        FindNext $LogFindHandle $LogFindName
        IfErrors 0 log_listing_loop
    FindClose $LogFindHandle
FunctionEnd
