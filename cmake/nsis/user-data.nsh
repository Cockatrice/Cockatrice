; User data backup and restore around the previous version's uninstaller.
;
; Kept apart from the upgrade orchestration in the template because all of it is
; about the previous version's uninstaller: it waits for that uninstaller's
; payload to disappear, removes what it left behind and appends its log, and it
; copies the user's data out of $OldInstallDir before the uninstaller runs and
; puts it back afterwards. The reasoning for the shared %TEMP% backup directory
; and for the copy-not-move relocation is in the comment above each function.
;
; Registers: $OldUninstallExit/$OldUninstallExe/$OldPayloadRemoved for the
; leftover handling, $DataBackupDir, $DataBackupDone, $DataBackupForDir and the
; $OldInstallDir/$INSTDIR pair for the backup. Removing leftovers aborts the
; install when it cannot free the files; the backup and restore never do.

; Waits for the previous installation's payload to be gone, so the sections below never
; write into a directory an old uninstaller is still emptying.
;
; ExecWait only waits for as long as the uninstaller runs in place: an NSIS uninstaller
; that did not get a usable _?= copies itself to %TEMP%, starts that copy and exits, so
; ExecWait returns at once while the copy keeps deleting - and it finishes after this
; window has closed, removing the version that was just installed. Waiting for
; cockatrice.exe to disappear covers both cases, and stops with a message instead of
;
; When the payload is still there after the wait, $OldUninstallExit decides: a non-zero
; code means the uninstaller was cancelled or failed, and the install aborts saying so.
; A zero code means it finished but skipped files someone was holding (NSIS Delete does
; that silently), so RemoveOldPayloadLeftovers frees and removes them, and the install
; only aborts - naming the holders - when even that fails.
; installing over a directory that is still being emptied.
Function WaitForOldPayloadToDisappear
${If} $OldInstallDir == ""
    Return
${EndIf}
; The registered uninstaller could not be run at all (missing, or ExecWait could not
; start it): nothing is going to empty the directory, so waiting is pointless and
; aborting as "cancelled" was wrong. Remove the previous payload the same way the
; skipped-files recovery below does, and abort - naming why - only if that fails.
${If} $OldUninstallExit == "missing"
${OrIf} $OldUninstallExit == "launch-failed"
    ${IfNot} ${FileExists} "$OldInstallDir\*.*"
        !insertmacro CockLog "old uninstall: uninstaller $OldUninstallExit and $OldInstallDir is already gone"
        Return
    ${EndIf}
    !insertmacro CockLog "old uninstall: uninstaller $OldUninstallExit ('$OldUninstallString'); removing $OldInstallDir payload directly"
    Call AppendUninstallerLog
    Call RemoveOldPayloadLeftovers
    ${If} $OldPayloadRemoved != "1"
        StrCpy $AbortReason \
            "The previous version in $OldInstallDir could not be removed: its uninstaller ($OldUninstallExe) could not be run ($OldUninstallExit), and removing the files directly failed.$\r$\n$\r$\nStill in use by: $HolderOutput$\r$\n$\r$\nClose anything still using those files and run this installer again."
        Call RestoreUserDataAfterUninstall
        Call AbortInstall
    ${EndIf}
    !insertmacro CockLog "old uninstall: continuing after removing the payload of an uninstaller that could not run"
    Return
${EndIf}
StrCpy $R2 0
old_uninstall_wait:
IfFileExists "$OldInstallDir\cockatrice.exe" 0 old_uninstall_gone
IntOp $R2 $R2 + 1
${If} $R2 > 60
    !insertmacro CockLog "old uninstall: $OldInstallDir still holds cockatrice.exe after 30 s (uninstaller exit=$OldUninstallExit)"
    ; The uninstaller's own account of what it skipped or why it stopped lives in its
    ; separate log; copy it here so the report sent from AbortInstall carries it.
    Call AppendUninstallerLog
    ${If} $OldUninstallExit != "0"
        ; Cancelled or failed with a non-zero exit code (an uninstaller that could not be
        ; started is handled above): the old version was not meant to be removed, so
        ; nothing is deleted on its behalf.
        StrCpy $AbortReason \
            "The previous version in $OldInstallDir is still there because its uninstaller was cancelled or failed (exit code '$OldUninstallExit').$\r$\n$\r$\nClose anything still using those files and run this installer again."
        ; The backup was taken before the uninstaller ran; an abort must not leave the
        ; only copy of the user data in %TEMP%, where the next run clears it.
        Call RestoreUserDataAfterUninstall
        Call AbortInstall
    ${EndIf}
    ; The uninstaller finished but skipped files - a running instance or a scanner holding
    ; them, which NSIS Delete passes over silently. Free them and remove what is left.
    Call RemoveOldPayloadLeftovers
    ; The removal reports success only when every payload executable is gone and the
    ; DLL/Plugins purge came back clean; cockatrice.exe alone would hide a locked
    ; oracle.exe or a stale DLL the new payload is about to be written over.
    ${If} $OldPayloadRemoved != "1"
        StrCpy $AbortReason \
            "The previous version in $OldInstallDir could not be fully removed after its uninstaller ran.$\r$\n$\r$\nStill in use by: $HolderOutput$\r$\n$\r$\nClose anything still using those files and run this installer again."
        Call RestoreUserDataAfterUninstall
        Call AbortInstall
    ${EndIf}
    ; The payload is gone and ExecWait already waited for the uninstaller; keep the
    ; short settle the other paths use, then hand back to the caller's restore.
    Sleep 1000
    !insertmacro CockLog "old uninstall: continuing after removing the leftovers the uninstaller skipped"
    Return
${EndIf}
Sleep 500
Goto old_uninstall_wait
old_uninstall_gone:
${If} $R2 > 0
    ; ExecWait returned while the old payload was still there: that uninstaller
    ; copied itself to %TEMP%, and the copy is still working through its file list.
    ; cockatrice.exe is gone before the rest of it, so let the copy settle before
    ; the sections below start writing files it would otherwise remove.
    Sleep 1000
    !insertmacro CockLog "old uninstall: ExecWait returned early, settled 1 s after $R2 * 500 ms"
${Else}
    !insertmacro CockLog "old uninstall: $OldInstallDir was already clear when ExecWait returned"
${EndIf}
Return
FunctionEnd

; Called when the old uninstaller exited successfully but left its payload behind, or
; could not be run at all (missing or not startable) and the payload is still there.
; Frees the files the same way the guard does (close, then offer to end), deletes what
; the uninstaller skipped, and on failure records which files survived and who holds
; them in $HolderOutput, for the abort message. Sets $OldPayloadRemoved to 1 only when
; every payload executable is gone and the DLL/Plugins purge is clean.
Function RemoveOldPayloadLeftovers
    !insertmacro CockLog "old uninstall: removing leftovers of $OldInstallDir"
    StrCpy $OldPayloadRemoved 0
    StrCpy $HolderOutput "nothing could be named"
    StrCpy $GuardDir $OldInstallDir
    Call GuardAppsNotRunning
    ; The purge helpers work on $INSTDIR; point them at the old directory for the
    ; duration (on /R the two are the same anyway).
    Push $INSTDIR
    StrCpy $INSTDIR $OldInstallDir
    StrCpy $R2 0
    rop_loop:
    SetFileAttributes "$INSTDIR\cockatrice.exe" NORMAL
    SetFileAttributes "$INSTDIR\oracle.exe" NORMAL
    SetFileAttributes "$INSTDIR\servatrice.exe" NORMAL
    Delete "$INSTDIR\cockatrice.exe"
    Delete "$INSTDIR\oracle.exe"
    Delete "$INSTDIR\servatrice.exe"
    Delete "$INSTDIR\qt.conf"
    Call RetryDllPurge
    ; $R0 is RetryDllPurge's verdict (1 = a DLL or the Plugins directory survived).
    ; Every payload executable has to be gone as well: a locked oracle.exe or
    ; servatrice.exe is just as fatal to the install that follows as cockatrice.exe.
    IfFileExists "$INSTDIR\cockatrice.exe" rop_retry
    IfFileExists "$INSTDIR\oracle.exe" rop_retry
    IfFileExists "$INSTDIR\servatrice.exe" rop_retry
    ${If} $R0 == 0
        Goto rop_done
    ${EndIf}
    rop_retry:
    IntOp $R2 $R2 + 1
    ${If} $R2 < 3
        !insertmacro CockLog "old uninstall: leftovers of $OldInstallDir survived removal attempt $R2, retrying in 2 s"
        Sleep 2000
        Goto rop_loop
    ${EndIf}
    !insertmacro CockLog "old uninstall: leftovers of $OldInstallDir still present after 3 removal attempts"
    !insertmacro CockLogListing "old-leftovers" "*.*"
    ; Per-file Windows codes for the DLLs plus the holder probe's line ($HolderOutput).
    Call LogStaleRuntimeDiagnostics
    Pop $INSTDIR
    Return
    rop_done:
    StrCpy $OldPayloadRemoved 1
    ; Only now that the recovery has succeeded: deleting the old uninstaller on a
    ; failed attempt would leave a directory whose UninstallString points at nothing,
    ; so no later run could remove it. WriteUninstaller recreates it on success anyway.
    SetFileAttributes "$INSTDIR\uninstall.exe" NORMAL
    Delete "$INSTDIR\uninstall.exe"
    !insertmacro CockLog "old uninstall: leftovers of $OldInstallDir removed"
    !insertmacro CockLogListing "old-leftovers-after-removal" "*.*"
    Pop $INSTDIR
FunctionEnd

; Copies the old uninstaller's log ($TEMP\Cockatrice-uninstaller.log) into this
; installer's log, so the report AbortInstall sends shows what the uninstaller did.
; Uninstallers older than this log simply have no file; that is logged too.
Function AppendUninstallerLog
    ClearErrors
    FileOpen $UninstLogHandle "$TEMP\Cockatrice-uninstaller.log" r
    ${If} ${Errors}
    ${OrIf} $UninstLogHandle == ""
        !insertmacro CockLog "uninstaller log: not found at $TEMP\Cockatrice-uninstaller.log"
        Return
    ${EndIf}
    !insertmacro CockLog "uninstaller log begin ----------------------------------------"
    aul_loop:
    ClearErrors
    FileRead $UninstLogHandle $UninstLogLine
    IfErrors aul_done
    ; Strip the line break FileRead keeps, so CockLog does not double it.
    aul_trim:
    StrCpy $R9 $UninstLogLine 1 -1
    ${If} $R9 == "$\n"
    ${OrIf} $R9 == "$\r"
        StrCpy $UninstLogLine $UninstLogLine -1
        Goto aul_trim
    ${EndIf}
    !insertmacro CockLog "uninstaller: $UninstLogLine"
    Goto aul_loop
    aul_done:
    FileClose $UninstLogHandle
    !insertmacro CockLog "uninstaller log end ------------------------------------------"
FunctionEnd

; BackupUserDataBeforeUninstall / RestoreUserDataAfterUninstall
;
; The uninstallers shipped in 3.1.0/3.1.1 run a blanket "RMDir /r $INSTDIR".
; For a portable install that deletes $INSTDIR\data (decks, settings, pictures)
; along with the payload, and for a normal install it deletes anything the user
; pointed into $INSTDIR. The fixed uninstaller preserves foreign files, but the
; destructive one is already on disk on affected machines, so stash a copy
; outside $INSTDIR before invoking it and put it back afterwards. Best effort
; only: a failed backup must never abort the install.
;
; The backup is left in %TEMP% afterwards on a best-effort basis only: the next
; installer run that takes a new backup wipes it first (see below), so it is not
; a durable second copy. Anything this backup does not cover - e.g. loose files
; with other extensions - relies on the client's startup safety backup
; (warnAboutDataPathsInProgramDir), which only exists after the first launch of
; the new client.
;
; The 32-bit and 64-bit uninstall blocks below share $TEMP\CockatriceDataBackup.
; When both are registered with different InstallLocations, backing up the second
; directory into the first one's backup would restore the first dir's files into
; the second one. BackupUserDataBeforeUninstall therefore starts over when
; $OldInstallDir changes: the previous pass already restored its own backup, so
; only its leftover %TEMP% copy is lost. RelocateRestoredDataToInstDir only sees
; the last pass, so with two different old dirs plus a new target dir the first
; dir's restored data stays in place instead of moving along - no loss, but the
; user has to copy it manually.
;
; The portable-data branch below is defensive: both callers (AutoUninstallIfNeeded
; on /R, componentsPagePre interactively) only run for non-portable installs, and
; portable installs never write the registry uninstall keys that trigger them.
; It fires only when $OldInstallDir nevertheless contains portable.dat - e.g. a
; stale registry InstallLocation pointing at a portable dir, or portable.dat added
; to a normal install dir. The loose-decks and install-subdirs branches are the
; live ones for normal installs.
Function BackupUserDataBeforeUninstall
; $OldInstallDir is the directory being uninstalled: on the /R path it equals
; $INSTDIR (updater passes /D=<appDir>), on the interactive path componentsPagePre
; sets it from the registry InstallLocation via $GuardDir, because $INSTDIR is
; still the default there (directory page comes later).
${If} $OldInstallDir == ""
    StrCpy $OldInstallDir $INSTDIR
${EndIf}
; %TEMP% survives across installer runs, so a previous run's safety copy would
; otherwise merge into this run: CopyFiles merges/overwrites, resurrecting files
; the user deleted since (e.g. deck A from run 1 reappearing alongside deck B
; from run 2) or restoring stale content after a failed copy. Wipe once per
; installer run - $DataBackupDir is empty until the first backup of this run,
; so the second call in sequence (32-bit then 64-bit uninstall) still merges.
${If} $DataBackupDir == ""
    RMDir /r "$TEMP\CockatriceDataBackup"
${EndIf}
; The 32-bit and 64-bit uninstall blocks share one backup dir. When the second
; block points at a different directory, merging would restore the first dir's
; files into the second one, so start over instead. The previous pass already
; restored its own backup; only its leftover %TEMP% copy is discarded.
; StrCmp is case-sensitive while Windows paths are not, so a casing-only
; difference triggers a harmless redundant re-backup of the same content.
${If} $DataBackupDir != ""
${AndIf} $DataBackupForDir != ""
${AndIf} $OldInstallDir != $DataBackupForDir
    !insertmacro CockLog "pre-uninstall backup: old dir changed ($DataBackupForDir -> $OldInstallDir), starting a fresh backup"
    RMDir /r "$TEMP\CockatriceDataBackup"
    StrCpy $DataBackupDir ""
    StrCpy $DataBackupDone ""
    StrCpy $LooseDecksBackupDone ""
    StrCpy $InstallSubdirsBackupDone ""
${EndIf}
${If} ${FileExists} "$OldInstallDir\portable.dat"
${AndIf} ${FileExists} "$OldInstallDir\data"
    StrCpy $DataBackupDir "$TEMP\CockatriceDataBackup"
    StrCpy $DataBackupDone "1"
    !insertmacro CockLog "pre-uninstall backup: copying $OldInstallDir\data (downloaded pics excluded)"
    DetailPrint "Backing up portable user data before removing previous version..."
    CreateDirectory "$DataBackupDir\data"
    ; Copy every top-level entry except pics (downloaded pictures are large and
    ; re-downloadable): copying everything first and deleting pics afterwards
    ; would duplicate gigabytes into %TEMP% just to throw them away.
    FindFirst $DataBackupFindHandle $DataBackupFindName "$OldInstallDir\data\*.*"
    ${If} $DataBackupFindHandle != ""
    backup_data_loop:
        ${If} $DataBackupFindName != "."
        ${AndIf} $DataBackupFindName != ".."
        ${AndIf} $DataBackupFindName != "pics"
            ${If} ${FileExists} "$OldInstallDir\data\$DataBackupFindName\*.*"
                CreateDirectory "$DataBackupDir\data\$DataBackupFindName"
                CopyFiles /SILENT "$OldInstallDir\data\$DataBackupFindName\*.*" "$DataBackupDir\data\$DataBackupFindName\"
            ${Else}
                CopyFiles /SILENT "$OldInstallDir\data\$DataBackupFindName" "$DataBackupDir\data\"
            ${EndIf}
        ${EndIf}
        ClearErrors
        FindNext $DataBackupFindHandle $DataBackupFindName
        IfErrors 0 backup_data_loop
        FindClose $DataBackupFindHandle
    ${EndIf}
    ; Keep user-made custom pics (data\pics\CUSTOM).
    ${If} ${FileExists} "$OldInstallDir\data\pics\CUSTOM"
        CreateDirectory "$DataBackupDir\data\pics\CUSTOM"
        CopyFiles /SILENT "$OldInstallDir\data\pics\CUSTOM\*.*" "$DataBackupDir\data\pics\CUSTOM\"
    ${EndIf}
    DetailPrint "Portable user data backup complete."
${EndIf}
; Deck extensions mirror DeckLoader::ACCEPTED_FILE_EXTENSIONS, minus *.txt: a
; blanket *.txt copy would also grab payload-adjacent text files, while the
; deck-specific extensions are unambiguous user content.
${If} ${FileExists} "$OldInstallDir\*.cod"
${OrIf} ${FileExists} "$OldInstallDir\*.dec"
${OrIf} ${FileExists} "$OldInstallDir\*.dek"
${OrIf} ${FileExists} "$OldInstallDir\*.mwDeck"
    ${If} $DataBackupDir == ""
        StrCpy $DataBackupDir "$TEMP\CockatriceDataBackup"
    ${EndIf}
    StrCpy $LooseDecksBackupDone "1"
    !insertmacro CockLog "pre-uninstall backup: copying loose deck files"
    DetailPrint "Backing up deck files stored in the install directory..."
    CreateDirectory "$DataBackupDir\loose_decks"
    CopyFiles /SILENT "$OldInstallDir\*.cod" "$DataBackupDir\loose_decks\"
    CopyFiles /SILENT "$OldInstallDir\*.dec" "$DataBackupDir\loose_decks\"
    CopyFiles /SILENT "$OldInstallDir\*.dek" "$DataBackupDir\loose_decks\"
    CopyFiles /SILENT "$OldInstallDir\*.mwDeck" "$DataBackupDir\loose_decks\"
${EndIf}
; Data folders below the top level (e.g. $OldInstallDir\decks) are deleted by the
; old "RMDir /r $INSTDIR" just like loose decks, and on a 3.1.1 -> new upgrade the
; client's startup backup has never run yet, so stash every top-level directory
; that is not part of the payload. Payload dirs stay in place: they are rewritten
; by the section below, and restoring them could put stale DLLs next to the new
; executables. "data" is skipped once the portable branch above already took it;
; without portable.dat it is user content like any other unknown dir.
FindFirst $DataBackupFindHandle $DataBackupFindName "$OldInstallDir\*.*"
${If} $DataBackupFindHandle != ""
backup_subdirs_loop:
    ${If} $DataBackupFindName != "."
    ${AndIf} $DataBackupFindName != ".."
    ${AndIf} $DataBackupFindName != "plugins"
    ${AndIf} $DataBackupFindName != "Plugins"
    ${AndIf} $DataBackupFindName != "translations"
    ${AndIf} $DataBackupFindName != "sounds"
    ${AndIf} $DataBackupFindName != "themes"
        ${If} $DataBackupFindName == "data"
        ${AndIf} $DataBackupDone == "1"
            Goto backup_subdirs_next
        ${EndIf}
        ${If} ${FileExists} "$OldInstallDir\$DataBackupFindName\*.*"
            ${If} $DataBackupDir == ""
                StrCpy $DataBackupDir "$TEMP\CockatriceDataBackup"
            ${EndIf}
            StrCpy $InstallSubdirsBackupDone "1"
            !insertmacro CockLog "pre-uninstall backup: copying subdir $OldInstallDir\$DataBackupFindName"
            DetailPrint "Backing up $DataBackupFindName stored in the install directory..."
            CreateDirectory "$DataBackupDir\install_subdirs\$DataBackupFindName"
            CopyFiles /SILENT "$OldInstallDir\$DataBackupFindName\*.*" "$DataBackupDir\install_subdirs\$DataBackupFindName\"
        ${EndIf}
    ${EndIf}
backup_subdirs_next:
    ClearErrors
    FindNext $DataBackupFindHandle $DataBackupFindName
    IfErrors 0 backup_subdirs_loop
    FindClose $DataBackupFindHandle
${EndIf}
; Remember which directory the current backup belongs to, so a later pass for a
; different directory starts over instead of merging (see above).
${If} $DataBackupDir != ""
    StrCpy $DataBackupForDir $OldInstallDir
${EndIf}
FunctionEnd

Function RestoreUserDataAfterUninstall
StrCmp $DataBackupDir "" done
${If} $OldInstallDir == ""
    StrCpy $OldInstallDir $INSTDIR
${EndIf}
${If} $DataBackupDone == "1"
${AndIf} ${FileExists} "$DataBackupDir\data"
    ${If} ${FileExists} "$OldInstallDir\data"
        ; Existence-only: a partially-deleted data dir (locked files during the old
        ; uninstall) looks the same as a survived one, so the backup is always kept.
        ; Merge the backup back over it to repair partial deletes; CopyFiles only
        ; adds/overwrites, so a fully-survived dir is unaffected.
        !insertmacro CockLog "post-uninstall restore: $OldInstallDir\data still exists, merging backup and keeping it"
        CopyFiles /SILENT "$DataBackupDir\data\*.*" "$OldInstallDir\data\"
        DetailPrint "Previous user data is still in place; pre-uninstall backup merged back and kept at $DataBackupDir\data."
    ${Else}
        !insertmacro CockLog "post-uninstall restore: restoring $OldInstallDir\data"
        CreateDirectory "$OldInstallDir\data"
        CopyFiles /SILENT "$DataBackupDir\data\*.*" "$OldInstallDir\data\"
        DetailPrint "Restored portable user data from pre-uninstall backup."
    ${EndIf}
${EndIf}
${If} $LooseDecksBackupDone == "1"
${AndIf} ${FileExists} "$DataBackupDir\loose_decks"
    !insertmacro CockLog "post-uninstall restore: restoring loose deck files"
    CreateDirectory "$OldInstallDir"
    CopyFiles /SILENT "$DataBackupDir\loose_decks\*.cod" "$OldInstallDir\"
    CopyFiles /SILENT "$DataBackupDir\loose_decks\*.dec" "$OldInstallDir\"
    CopyFiles /SILENT "$DataBackupDir\loose_decks\*.dek" "$OldInstallDir\"
    CopyFiles /SILENT "$DataBackupDir\loose_decks\*.mwDeck" "$OldInstallDir\"
    DetailPrint "Restored deck files stored in the install directory."
${EndIf}
${If} $InstallSubdirsBackupDone == "1"
${AndIf} ${FileExists} "$DataBackupDir\install_subdirs"
    !insertmacro CockLog "post-uninstall restore: restoring install subdirectories"
    CreateDirectory "$OldInstallDir"
    FindFirst $DataBackupFindHandle $DataBackupFindName "$DataBackupDir\install_subdirs\*.*"
    ${If} $DataBackupFindHandle != ""
    restore_subdirs_loop:
        ${If} $DataBackupFindName != "."
        ${AndIf} $DataBackupFindName != ".."
        ${AndIf} ${FileExists} "$DataBackupDir\install_subdirs\$DataBackupFindName\*.*"
            !insertmacro CockLog "post-uninstall restore: restoring subdir $OldInstallDir\$DataBackupFindName"
            CreateDirectory "$OldInstallDir\$DataBackupFindName"
            CopyFiles /SILENT "$DataBackupDir\install_subdirs\$DataBackupFindName\*.*" "$OldInstallDir\$DataBackupFindName\"
        ${EndIf}
        ClearErrors
        FindNext $DataBackupFindHandle $DataBackupFindName
        IfErrors 0 restore_subdirs_loop
        FindClose $DataBackupFindHandle
    ${EndIf}
    DetailPrint "Restored folders stored in the install directory."
${EndIf}
; Runs once per uninstall block, and there are two (32-bit, 64-bit): show the
; notice a single time per installer run, covering both loose decks and
; restored subdirectories.
${IfNot} ${Silent}
${AndIf} $LooseDeckNoticeShown != "1"
    ${If} $LooseDecksBackupDone == "1"
    ${OrIf} $InstallSubdirsBackupDone == "1"
        StrCpy $LooseDeckNoticeShown "1"
        MessageBox MB_OK|MB_ICONINFORMATION "Your decks or data folders were stored inside the Cockatrice install folder ($OldInstallDir), which the previous version's uninstaller deletes.$\n$\nThey have been restored from a backup to that folder (loose deck files ending in .txt are not backed up - move those manually). If you install to a different folder, they are moved there automatically. Please move them out of the install folder afterwards (Settings -> General -> Paths) so a future uninstall cannot delete them again."
    ${EndIf}
${EndIf}
done:
FunctionEnd

; RelocateRestoredDataToInstDir
;
; componentsPagePre backs up and restores around the old uninstaller before the
; directory page, so $OldInstallDir (registry InstallLocation) is authoritative
; there while $INSTDIR is still the default. When the user then picks a different
; target directory, the restored user data would otherwise stay orphaned in the
; old directory while the new install starts empty. Copy it across when they differ
; (a copy, not a move: the source is user data and is deliberately left in place).
; Only the last uninstall pass's $OldInstallDir is relocated (see the shared-backup
; comment above); with two different old dirs the first one's data stays restored
; in place instead of moving along.
; StrCmp is case-sensitive while Windows paths are not; a casing-only difference
; triggers a harmless redundant copy onto itself.
Function RelocateRestoredDataToInstDir
StrCmp $DataBackupDir "" done
StrCmp $OldInstallDir "" done
StrCmp $OldInstallDir $INSTDIR done
${If} $DataBackupDone == "1"
${AndIf} ${FileExists} "$OldInstallDir\data"
    !insertmacro CockLog "relocate: copying restored data $OldInstallDir\data -> $INSTDIR\data"
    CreateDirectory "$INSTDIR\data"
    CopyFiles /SILENT "$OldInstallDir\data\*.*" "$INSTDIR\data\"
    DetailPrint "Copied restored user data to the new install folder."
${EndIf}
${If} $LooseDecksBackupDone == "1"
    ${If} ${FileExists} "$OldInstallDir\*.cod"
    ${OrIf} ${FileExists} "$OldInstallDir\*.dec"
    ${OrIf} ${FileExists} "$OldInstallDir\*.dek"
    ${OrIf} ${FileExists} "$OldInstallDir\*.mwDeck"
        !insertmacro CockLog "relocate: copying restored decks $OldInstallDir -> $INSTDIR"
        CreateDirectory "$INSTDIR"
        CopyFiles /SILENT "$OldInstallDir\*.cod" "$INSTDIR\"
        CopyFiles /SILENT "$OldInstallDir\*.dec" "$INSTDIR\"
        CopyFiles /SILENT "$OldInstallDir\*.dek" "$INSTDIR\"
        CopyFiles /SILENT "$OldInstallDir\*.mwDeck" "$INSTDIR\"
        DetailPrint "Copied restored deck files to the new install folder."
    ${EndIf}
${EndIf}
${If} $InstallSubdirsBackupDone == "1"
${AndIf} ${FileExists} "$DataBackupDir\install_subdirs"
    FindFirst $DataBackupFindHandle $DataBackupFindName "$DataBackupDir\install_subdirs\*.*"
    ${If} $DataBackupFindHandle != ""
    relocate_subdirs_loop:
        ${If} $DataBackupFindName != "."
        ${AndIf} $DataBackupFindName != ".."
        ${AndIf} ${FileExists} "$OldInstallDir\$DataBackupFindName\*.*"
            !insertmacro CockLog "relocate: copying restored subdir $OldInstallDir\$DataBackupFindName -> $INSTDIR\$DataBackupFindName"
            CreateDirectory "$INSTDIR\$DataBackupFindName"
            CopyFiles /SILENT "$OldInstallDir\$DataBackupFindName\*.*" "$INSTDIR\$DataBackupFindName\"
            DetailPrint "Copied restored folder $DataBackupFindName to the new install folder."
        ${EndIf}
        ClearErrors
        FindNext $DataBackupFindHandle $DataBackupFindName
        IfErrors 0 relocate_subdirs_loop
        FindClose $DataBackupFindHandle
    ${EndIf}
${EndIf}
done:
FunctionEnd
