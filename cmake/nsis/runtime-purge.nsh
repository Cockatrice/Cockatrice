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

; Removes the Plugins directory and all top-level DLLs from $INSTDIR, then
; retries up to two more times (2 s apart) when files remain locked, so that
; transient holders such as antivirus scanners have a chance to release their
; handles before the installer gives up. Three attempts total.
; Sets $R0 to 0 when the purge is clean, or to 1 when stale DLLs remain
; after all attempts.
Function RetryDllPurge
    StrCpy $R5 0        ; counts retries after the initial attempt
    rdp_loop:
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
; The probe opens each survivor for DELETE access and then closes it again
; without deleting anything. The code is read by the plugin right after that call
; (?e), so a failure reports what Windows refused; clearing LastError first is
; what makes a zero code mean "this open worked", i.e. the purge-time failure was
; transient (an antivirus scan that has finished, an instance still shutting
; down) rather than a standing condition. The plugin parses the call's arguments
; between the clear and the call, so a non-zero code is reported as what Windows
; said rather than as proof of a cause - the wording below never claims one.
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
                ; A valid handle: close it. The open proves the file is not held
                ; open, but not that Delete would succeed: a read-only file opens
                ; for DELETE and still fails DeleteFile until the attribute is
                ; cleared, which the purge does not do. Report that standing
                ; cause instead of calling the failure transient.
                System::Call 'kernel32::CloseHandle(p R1)'
                System::Call 'kernel32::GetFileAttributesW(w "$INSTDIR\$StaleFindName") i.R3'
                ${If} $R3 = -1
                    StrCpy $R0 "a delete-access open now succeeds, but its attributes could not be read"
                ${Else}
                    IntOp $R3 $R3 & 1
                    ${If} $R3 = 1
                        StrCpy $R0 "a delete-access open now succeeds, but it is still read-only, which Delete does not clear"
                    ${Else}
                        StrCpy $R0 "a delete-access open now succeeds"
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

; Names the applications and services that still have the install directory's files
; open or mapped. Writes one line to the log and sets $HolderFound (1 when anything was
; named), so the abort message can repeat that line to the user.
;
; The lookup is a small executable built from tools/installer-holder-probe, extracted to
; $PLUGINSDIR only when the purge has already failed: the successful path pays nothing
; for it, and $PLUGINSDIR takes it away again when the installer exits. It is compiled by
; the same Windows build that compiles this installer, which is what lets it call the
; Restart Manager API directly. The same call made from this script would have to
; marshal RM_PROCESS_INFO through System::Call as a table of hand-computed offsets, and
; that table is precisely what cannot be checked without a Windows toolchain; in
; compiled code the layout belongs to the Windows SDK header.
;
; Restart Manager is what makes the answer complete. Walking processes can only name one
; whose image lives here or that loaded one of these libraries, while Restart Manager
; names a process that holds a plain handle on a file - an antivirus scanner - and names
; services as well as user programs. Its blind spot is a handle taken in kernel mode,
; where there is no process to name; then the codes logged above are the whole story, and
; the probe says so by naming nothing rather than by failing.
;
; $INSTDIR travels through the same COCKATRICE_GUARD_DIR environment variable the
; running-instance guard uses, so a path with spaces or apostrophes never has to be
; embedded in a command line.
Function LogProcessesUsingInstallDir
    StrCpy $HolderExit 0
    StrCpy $HolderOutput ""
    StrCpy $HolderFound 0
    InitPluginsDir
    ; Embedded by the same relative path that resolves NSIS.definitions.nsh above (both
    ; are built into the build root, which is three levels up from project.nsi).
    File "/oname=$PLUGINSDIR\installer-holder-probe.exe" "..\..\..\installer-holder-probe.exe"
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$INSTDIR")'
    ; /TIMEOUT because this runs on the abort path, ahead of the message box: one call
    ; into the Restart Manager cannot take the minute a walk over every process can, but
    ; if that service stalls it must not leave the installer hanging with nothing on
    ; screen. On a timeout nsExec reports it through the exit code like any other failure.
    nsExec::ExecToStack /TIMEOUT=20000 '"$PLUGINSDIR\installer-holder-probe.exe"'
    Pop $HolderExit
    Pop $HolderOutput
    ; The exit code, not the text, decides what happened: it cannot be mangled by what
    ; nsExec leaves on the end of the captured output. 1 = the line below names what is
    ; holding the files, 0 = nothing holds them any more, anything else = the probe could
    ; not answer, and says why in its own output.
    ${If} $HolderExit == "1"
        StrCpy $HolderFound 1
    ${ElseIf} $HolderExit == "0"
        StrCpy $HolderOutput "nothing is holding them any more"
    ${Else}
        StrCpy $HolderOutput "the holder probe could not run: $HolderOutput"
    ${EndIf}
    !insertmacro CockLog "holders: $HolderOutput"
FunctionEnd
