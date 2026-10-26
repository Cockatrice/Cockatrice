; User data backup and restore around the previous version's uninstaller.
;
; Kept apart from the upgrade orchestration in the template because nothing here
; deletes payload files: it copies the user's data out of $OldInstallDir before
; the old uninstaller runs and puts it back afterwards. The reasoning for the
; shared %TEMP% backup directory and for the copy-not-move relocation is in the
; comment above each function.
;
; Registers: $DataBackupDir, $DataBackupDone, $DataBackupForDir and the
; $OldInstallDir/$INSTDIR pair; nothing here fails the install.

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
