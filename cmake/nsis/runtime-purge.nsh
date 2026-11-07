; Stale runtime purge and its diagnostics: which leftovers make an install
; unusable, the retry loop that removes them, the read-only clearing that has to
; happen first, and the Restart Manager probe that names whatever survived.
;
; Everything here runs between the running-instance guard and the payload write,
; and none of it aborts on its own: the template decides what to do with the
; verdict.
;
; Register contract:
;   $R0        out: RetryDllPurge/HasStaleRuntimeFiles/IsExistingCockatriceInstall
;                   verdict (1 = leftovers or an existing install, 0 = neither)
;   $R5        internal: RetryDllPurge retry counter
;   $R6-$R8    internal: ClearReadOnlyTree recursion, saved and restored
;   $HolderExit/$HolderOutput/$HolderFound
;              out: LogProcessesUsingInstallDir - exit code, the one-line summary
;                   the dialogs show, and 1 when it named a holder
;   $PLUGINSDIR\installer-holder-probe.exe
;              built by tools/installer-holder-probe and embedded under the same
;              relative path NSIS.definitions.nsh resolves to the build root

; Sets $R0 to 1 if $INSTDIR looks like an existing Cockatrice install. Used to
; Sets $R0 to 1 if $INSTDIR looks like an existing Cockatrice install. Used to
; guard the stale-runtime-DLL purge so a first-time install into a pre-existing
; directory with an unrelated "Plugins" folder is never touched, while an
; install that only ever held e.g. servatrice.exe is still purged.
Function IsExistingCockatriceInstall
    StrCpy $R0 0
    ${If} ${FileExists} "$INSTDIR\cockatrice.exe"
        StrCpy $R0 1
    ${EndIf}
    ${If} ${FileExists} "$INSTDIR\oracle.exe"
        StrCpy $R0 1
    ${EndIf}
    ${If} ${FileExists} "$INSTDIR\servatrice.exe"
        StrCpy $R0 1
    ${EndIf}
    ${If} ${FileExists} "$INSTDIR\uninstall.exe"
        StrCpy $R0 1
    ${EndIf}
    ; A leftover runtime DLL counts as an install as well. A previously failed
    ; update can leave a directory in which the old uninstaller removed the
    ; executables and uninstall.exe but silently skipped the DLLs its running
    ; instance had locked - which is exactly the broken install the purge is
    ; meant to heal, and without these markers none of the files above would be
    ; there to prove that it is one. Every Qt module Cockatrice links is part of
    ; the payload, so match them all: probing only Qt6Core.dll would miss a
    ; directory where that one file happened to be deletable and the others
    ; were not. Qt6*.dll rather than *.dll on purpose: this function decides
    ; whether a directory is ours, and a folder that merely holds some DLLs of
    ; its own must not be purged. Qt6*.dll is enough even when only the protobuf
    ; runtime survived, because the whole Qt set is mapped by the same process
    ; that maps abseil_dll.dll and so is locked or free with it.
    StrCpy $R3 "Qt6*.dll"
    Call CheckStaleRuntimeMask
    ${If} ${FileExists} "$INSTDIR\qt.conf"
        StrCpy $R0 1
    ${EndIf}
FunctionEnd

; Sets $R0 to 1 if a runtime file of a previous version is still present after
; the purge. NSIS silently skips whatever it cannot delete, so one surviving file
; is enough to prove that something still holds a lock on the install directory.
;
; Any DLL at all counts, not a list of expected ones. The purge removes every
; top-level DLL, so after it has run the only DLL that can still be there is one
; it failed to delete, whatever its name. A mask per library family (Qt, then
; OpenSSL, then zlib) is what let abseil_dll.dll through: it is in neither the
; purge nor this check, so it survived a 3.0.2 to 3.1 update untouched, the new
; executables were written next to it, and the client then failed to start with
; "abseil_dll.dll is either not designed to run on Windows or it contains an
; error". Probing *.dll has no list to fall out of date when the next
; transitive dependency appears.
Function HasStaleRuntimeFiles
    StrCpy $R0 0
    StrCpy $R3 "*.dll"
    Call CheckStaleRuntimeMask
    ; IfFileExists takes wildcards, so a Plugins directory the purge could not
    ; remove counts as a leftover too.
    ${If} ${FileExists} "$INSTDIR\Plugins\*.*"
        !insertmacro CockLog "stale-runtime: Plugins directory survived the purge"
        StrCpy $R0 1
    ${EndIf}
FunctionEnd

; usage: set $R3 to a file mask, call this. Sets $R0 to 1 when $INSTDIR holds a
; matching file. Returns early once something was found, so $R0 stays 1.
Function CheckStaleRuntimeMask
    ${If} $R0 = 1
        Return
    ${EndIf}
    FindFirst $StaleFindHandle $StaleFindName "$INSTDIR\$R3"
    ${If} $StaleFindHandle != ""
        !insertmacro CockLog "stale-runtime: '$StaleFindName' matched $R3 in $INSTDIR"
        StrCpy $R0 1
        FindClose $StaleFindHandle
    ${EndIf}
FunctionEnd

; Clears the read-only attribute of everything the purge is about to delete.
;
; Delete and RMDir only set the error flag: a read-only file is left on disk
; untouched, and the retry loop then cannot make progress however long it waits,
; because the file is not held - it is protected. The abort diagnostics have a
; branch for exactly that ("read-only, which stops Delete"), so a survivor that
; is both free and read-only was a dead end until now. This runs on every
; attempt, so an attribute set again between attempts is cleared as well.
; SetFileAttributes has no wildcard support, hence the walks.
Function ClearReadOnlyPurgeTargets
    FindFirst $StaleFindHandle $StaleFindName "$INSTDIR\*.dll"
    ${If} $StaleFindHandle != ""
    crp_dll_loop:
        SetFileAttributes "$INSTDIR\$StaleFindName" NORMAL
        ClearErrors
        FindNext $StaleFindHandle $StaleFindName
        IfErrors 0 crp_dll_loop
        FindClose $StaleFindHandle
    ${EndIf}
    StrCpy $R6 "$INSTDIR\Plugins"
    Call ClearReadOnlyTree
FunctionEnd

; usage: set $R6 to a directory. Clears the read-only attribute of every entry
; below it, so RMDir /r is not stopped by a single file. Saves and restores the
; registers it works in, because it recurses into itself.
Function ClearReadOnlyTree
    Push $R6
    Push $R7
    Push $R8
    FindFirst $R7 $R8 "$R6\*.*"
    ${If} $R7 != ""
    crt_loop:
        ${If} $R8 != "."
        ${AndIf} $R8 != ".."
            SetFileAttributes "$R6\$R8" NORMAL
            ; Only an entry with contents has anything below it that the purge
            ; would fail on; an empty directory's own attribute is already
            ; cleared above and there is nothing left to descend into.
            ${If} ${FileExists} "$R6\$R8\*.*"
                Push $R6
                StrCpy $R6 "$R6\$R8"
                Call ClearReadOnlyTree
                Pop $R6
            ${EndIf}
        ${EndIf}
        ClearErrors
        FindNext $R7 $R8
        IfErrors 0 crt_loop
        FindClose $R7
    ${EndIf}
    Pop $R8
    Pop $R7
    Pop $R6
FunctionEnd

; Removes the Plugins directory and all top-level DLLs from $INSTDIR, then
; retries up to two more times (2 s apart) when files remain locked, so that
; transient holders such as antivirus scanners have a chance to release their
; handles before the installer gives up. Three attempts total. The read-only
; attribute is cleared before each attempt: a protected file is not a held one
; and would never clear on its own.
; Sets $R0 to 0 when the purge is clean, or to 1 when stale DLLs remain
; after all attempts.
Function RetryDllPurge
    StrCpy $R5 0        ; counts retries after the initial attempt
    rdp_loop:
    Call ClearReadOnlyPurgeTargets
    RMDir /r "$INSTDIR\Plugins"
    Delete "$INSTDIR\*.dll"
    Call HasStaleRuntimeFiles
    ; $R0 = 0: all clear, nothing left to do.
    ${If} $R0 = 0
        Return
    ${EndIf}
    IntOp $R5 $R5 + 1
    ; Allow two retries after the initial attempt ($R5 is the retry counter,
    ; so the loop runs at most 3 times: attempt + retry 1 + retry 2).
    ${If} $R5 > 2
        !insertmacro CockLog "RetryDllPurge: stale files remain after 3 attempts, giving up"
        Return          ; $R0 is already 1
    ${EndIf}
    !insertmacro CockLog "RetryDllPurge: retry $R5 of 2 after stale purge, sleeping 2 s"
    Sleep 2000
    Goto rdp_loop
FunctionEnd  ; RetryDllPurge
; Turns a Win32 error code into the wording that goes into the log.
; usage: set $0 to the code, result in $R0.
;
; These are Windows' own names for the codes and deliberately nothing more. A
; code on its own cannot say whether an upgrade was blocked by a running instance
; or by a permission: deleting a file whose image is mapped by another process
; surfaces as ACCESS_DENIED, exactly like a file whose ACL denies delete does.
; LogProcessesUsingInstallDir is what separates the two, which is why it runs
; right after this and why the two are read together.
Function GetWin32ErrorText
    ${If} $0 = 2
        StrCpy $R0 "already gone"
    ${ElseIf} $0 = 3
        StrCpy $R0 "path not found"
    ${ElseIf} $0 = 5
        StrCpy $R0 "access denied"
    ${ElseIf} $0 = 19
        StrCpy $R0 "write-protected media"
    ${ElseIf} $0 = 32
        StrCpy $R0 "sharing violation"
    ${ElseIf} $0 = 33
        StrCpy $R0 "lock violation"
    ${ElseIf} $0 = 1224
        StrCpy $R0 "file is mapped in memory"
    ${Else}
        StrCpy $R0 "windows error"
    ${EndIf}
FunctionEnd

; Records which runtime files the purge left behind, what Windows said about each
; one, and which processes are using the install directory.
;
; Diagnostic only, and only on the abort path: it deletes nothing, so it can never
; turn a failed upgrade into a half-removed old install, and it decides nothing -
; the abort that called it is already decided. It exists because NSIS Delete/RMDir
; only set the error flag: without this, a report of "files of a previous version
; survived the purge" said which files were present but never which one was held,
; and never whether the cause was a lock or a permission.
;
; Each survivor is opened for DELETE access and then asked the question DeleteFile
; asks: SetFileInformationByHandle(FileDispositionInfo) marks it for deletion on
; close, and the mark is cleared again right away, so nothing is deleted (and if
; that cancel ever failed, the file would go when the handle closes - a file the
; purge already tried to delete, and the line says so). That separates the three
; causes a failed Delete cannot tell apart: another handle that does not share
; delete refuses the mark, a read-only attribute refuses it, and a file nobody
; holds accepts it and would have been deleted. The mark is read through ?e, so a
; refusal reports Windows' own code; clearing LastError first keeps a zero code
; meaning "accepted". The wording below reports what Windows said and never claims
; a cause the code cannot prove. Which process is holding a survivor, if any, is
; what the holders line after the loop answers.
Function LogStaleRuntimeDiagnostics
    FindFirst $StaleFindHandle $StaleFindName "$INSTDIR\*.dll"
    ${If} $StaleFindHandle != ""
    stale_diag_loop:
        ; ClearErrors tells a plugin failure apart from an API failure: the System
        ; plugin sets the error flag only when it could not make the call at all,
        ; in which case it pushes its own message instead of the call's value and
        ; the pop below must not run.
        ClearErrors
        System::Call 'kernel32::SetLastError(i 0)'
        System::Call 'kernel32::CreateFileW(w "$INSTDIR\$StaleFindName", i 0x00010000, i 0, p 0, i 3, i 0, p 0) p.R1 ?e'
        ${If} ${Errors}
            StrCpy $R0 "the delete probe could not be run"
        ${Else}
            Pop $R2
            ${If} $R2 = 0
                ; The open worked, which says nothing about Delete: a DLL a running
                ; process has mapped opens for DELETE and still refuses DeleteFile, and
                ; a read-only file does the same until its attribute is cleared. Ask
                ; Windows the question the purge actually asks - mark the file for
                ; deletion on close, then cancel that again. This is the same request
                ; DeleteFile makes on a temporary handle, so its answer says whether a
                ; delete would succeed now.
                ClearErrors
                System::Call '*(&i1 1) p.R4'
                System::Call 'kernel32::SetFileInformationByHandle(p R1, i 4, p R4, i 1) i.R5 ?e'
                ${If} ${Errors}
                    StrCpy $R0 "a delete-access open now succeeds, but Windows could not be asked whether it would delete it"
                ${Else}
                    Pop $R6
                    ${If} $R5 != 0
                        ; Accepted: cancel the mark again, so this stays a question and
                        ; does not turn into the deletion it is asking about. A cancel
                        ; that fails leaves the file to go when the handle closes, which
                        ; is what the purge wanted anyway; the line says so.
                        ClearErrors
                        System::Call '*(&i1 0) p.R4'
                        System::Call 'kernel32::SetFileInformationByHandle(p R1, i 4, p R4, i 1) i.R5 ?e'
                        ${If} ${Errors}
                            StrCpy $R0 "a delete would succeed now, but the check could not undo the mark it set"
                        ${Else}
                            Pop $R6
                            ${If} $R5 != 0
                                StrCpy $R0 "a delete would succeed now - the earlier failure was transient"
                            ${Else}
                                StrCpy $R0 "a delete would succeed now, and it is marked for deletion from here on"
                            ${EndIf}
                        ${EndIf}
                    ${Else}
                        StrCpy $0 $R6
                        Call GetWin32ErrorText
                        StrCpy $R0 "a delete-access open now succeeds, but Windows refused to mark it for deletion: win32 error $R6: $R0"
                    ${EndIf}
                ${EndIf}
                System::Call 'kernel32::CloseHandle(p R1)'
                ; The read-only attribute is a cause on its own, and the purge clears it
                ; before every attempt, so a survivor that is read-only had it set again.
                System::Call 'kernel32::GetFileAttributesW(w "$INSTDIR\$StaleFindName") i.R3'
                ${If} $R3 != -1
                    IntOp $R3 $R3 & 1
                    ${If} $R3 = 1
                        StrCpy $R0 "$R0; it is read-only again"
                    ${EndIf}
                ${EndIf}
            ${Else}
                StrCpy $0 $R2
                Call GetWin32ErrorText
                StrCpy $R0 "win32 error $R2: $R0"
            ${EndIf}
        ${EndIf}
        !insertmacro CockLog "stale-runtime: '$StaleFindName' is still present: $R0"
        ClearErrors
        FindNext $StaleFindHandle $StaleFindName
        IfErrors 0 stale_diag_loop
        FindClose $StaleFindHandle
    ${EndIf}
    Call LogProcessesUsingInstallDir
FunctionEnd

; Names the applications and services that still have the payload open or mapped.
; Writes one line to the log and sets $HolderFound (1 when anything was named), so the
; abort message can repeat that line to the user.
;
; The lookup is tools/installer-holder-probe, the same executable the guard runs before
; anything is replaced; here it answers the other question, about files a purge could not
; remove. It asks the Restart Manager (RmRegisterResources/RmGetList), which names a
; process holding a plain handle on a file - an antivirus scanner - and names services as
; well as user programs, and it adds every payload program running from the directory
; itself - not one in a folder below it, which is another application the user installed
; there - because a running instance holds its payload as mapped images rather than as open
; handles and the Restart Manager does not reliably report those. Its remaining blind spot
; is a handle taken in kernel mode, where there is no process to name; a payload executable
; among the files then still refuses to be replaced, and the probe answers with that file
; instead of naming nothing.
;
; $INSTDIR travels through the same COCKATRICE_GUARD_DIR environment variable the guard
; uses, so a path with spaces or apostrophes never has to be embedded in a command line.
Function LogProcessesUsingInstallDir
    StrCpy $HolderExit 0
    StrCpy $HolderOutput ""
    StrCpy $HolderFound 0
    ; The directory that just failed the purge, not whatever a check before the directory
    ; page looked at: a portable install into a custom folder reaches here with the two
    ; being different.
    StrCpy $GuardDir $INSTDIR
    Call RunHolderProbe
    StrCpy $HolderExit $GuardExit
    StrCpy $HolderOutput $GuardOutput
    ; The exit code, not the text, decides what happened: it cannot be mangled by what
    ; nsExec leaves on the end of the captured output. 1 = the line below names what is
    ; holding the files, 0 = nothing holds them any more, anything else = the probe could
    ; not answer, and says why in its own output.
    ${If} $HolderExit == "1"
        StrCpy $HolderFound 1
    ${ElseIf} $HolderExit == "0"
        StrCpy $HolderOutput "nothing is holding them and no payload program is running from the directory"
    ${Else}
        StrCpy $HolderOutput "the holder probe could not name what holds them: $HolderOutput"
    ${EndIf}
    !insertmacro CockLog "holders: $HolderOutput"
FunctionEnd

; Called after a purge left files behind and LogStaleRuntimeDiagnostics named who is
; holding them. Instead of aborting straight away - which on /R strands the user, since
; the old version is already uninstalled - it closes once, purges again, and only then
; asks: end what is still holding the files, or try the removal again by hand. Sets
; $StaleRuntimeAfterPurge to the last purge's verdict, so the caller still aborts when the
; files survive.
;
; The shape is the guard's, deliberately: one automatic attempt at a graceful close, then a
; prompt whose Yes ends the payload programs and whose No retries. A client that was still
; shutting down releases the files in the automatic round and never sees a question, and an
; Oracle or Servatrice - which a close cannot reach - is what the prompt exists for, so the
; user is never left with a dialog whose answers cannot free anything.
;
; The automatic close is a request (WM_CLOSE) and the client keeps the last word: it may be
; waiting for an answer to its own "really quit?" confirmation (a running card database
; update, an open game, an unsaved deck), and that stays its decision. The instance is
; brought forward first, so that confirmation is on screen rather than behind this
; installer's prompt.
;
; Ending is the same last resort the guard offers, with the same cost spelled out, and only
; ever the programs the payload installs - see EndHolders.
;
; Restart Manager's RmShutdown was the alternative and was rejected: it shuts down
; services as well as applications, and the only way to bring them back is RmRestart,
; which would restart this very application into files that are about to be replaced.
;
; Always offered, in every mode: a /S install without /R is the only case without someone
; at the keyboard, and this installer does not promise unattended runs - if an error needs
; a decision, it asks for one. No offer when the probe named nobody: there is nothing to
; end, and the abort's per-file diagnostics already record what Windows said.
;
; One question after the automatic close, and never a loop: Yes ends the programs the
; payload installs - and says so, because anything else the probe named is left to the user -
; and purges again; the other answer gives up, and the caller's abort names what survived.
; The listing is re-read before the question, so it says what is left now rather than
; repeating the first one.
Function RecoverFromHolders
    ${If} $HolderFound != 1
        !insertmacro CockLog "recover-holders: no holder was named, nothing to offer"
        Return
    ${EndIf}
    ; The automatic attempt, the same one the guard makes before it asks anything.
    !insertmacro CockLog "recover-holders: closing cockatrice, then trying the removal again"
    StrCpy $GuardDir $INSTDIR
    Call CloseHolders
    Call RetryDllPurge
    StrCpy $StaleRuntimeAfterPurge $R0
    !insertmacro CockLogListing "after-retry-purge" "*.dll"
    !insertmacro CockLogListing "after-retry-plugins" "Plugins\*.*"
    !insertmacro CockLog "recover-holders: stale-runtime after retry=$StaleRuntimeAfterPurge"
    ${If} $StaleRuntimeAfterPurge = 0
        DetailPrint "Files of the previous version were removed; continuing the installation."
        Return
    ${EndIf}
    ; Still stuck: refresh the names, so the prompt and the log describe what survived the
    ; last attempt and not what was there before it.
    Call LogStaleRuntimeDiagnostics
    ${If} $HolderFound != 1
        Return
    ${EndIf}
    ; One question, the same one the guard asks and with the same two answers. No retry loop:
    ; the purge has already been tried after the close, so the only thing that can still free
    ; the files is ending them, and the caller's abort names whatever survives that.
    !insertmacro CockEndPrompt "Files of the previous version of Cockatrice are still in use by:" $HolderOutput "installation"
    MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2 "$GuardPrompt" IDYES rfh_end
    !insertmacro CockLog "recover-holders: cancelled by user"
    Return
    ; Only reached through Yes: Return does not come back here.
    rfh_end:
    !insertmacro CockLog "recover-holders: ending the payload programs on request, then trying the removal again"
    Call EndHolders
    Call RetryDllPurge
    StrCpy $StaleRuntimeAfterPurge $R0
    !insertmacro CockLog "recover-holders: stale-runtime after end=$StaleRuntimeAfterPurge"
    ${If} $StaleRuntimeAfterPurge = 0
        DetailPrint "Files of the previous version were removed; continuing the installation."
        Return
    ${EndIf}
    ; Refresh the names one last time so the abort that follows describes what survived the
    ; end, and then let it stop the run: the same dialog again would only repeat itself.
    Call LogStaleRuntimeDiagnostics
FunctionEnd
