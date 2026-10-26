; Running instance handling: make sure no installed application still holds the
; files that are about to be deleted or replaced, closing or force-stopping it
; where the caller allows that, and prompting the user where it does not.
;
; Every routine exists twice, once plain and once with the un prefix. That is
; forced by NSIS: the uninstaller is a separate program compiled into the same
; installer, and it cannot call the installer's functions. Keep the two copies in
; step.
;
; Register contract, shared by the routines here and their un copies:
;   $GuardDir         in:  directory a matched process has to run from
;   $R1               in:  display name for the prompts (e.g. cockatrice.exe)
;   $R2               in:  image name for Get-Process -Name (e.g. cockatrice)
;   $AllowForceClose  in:  1 when the application may be stopped by PID once the
;                          grace period is over, 0 when only WM_CLOSE is allowed
;   $R0               out: IsAppRunning/IsProcessLookupAvailable - 1 yes,
;                          0 no, 2 could not determine
;   $AbortReason      out: set before AbortInstall on every abort path
;   $LauncherExited   out: WaitForLauncherToExit - 1 when the /PID= process was
;                          seen to exit, 0 when no usable /PID= was given
;
; AbortInstall itself stays in the template: its Sentry offer is substituted by
; CPack, which only configures that one file.

; --- Running instance handling ---
; Cockatrice, Oracle and Servatrice must not be running while files are
; replaced or deleted. A still-running process holds locks on its .exe and
; Qt runtime DLLs, so a silent upgrade could otherwise end up with a mix of
; old and new Qt DLLs next to the new executable, failing with
; "The procedure entry point X could not be located in the dynamic link
; library ...Qt6Network.dll" on the next start.
;
; On a silent update (/R /S) matching processes are asked to close gracefully
; (WM_CLOSE) and waited for up to 60 s. Cockatrice is never force-stopped: it
; may be holding an unsaved deck or showing a "really leave this game?"
; confirmation, so if it refuses to close the install is aborted - nothing has
; been replaced yet, so the update is simply deferred. Oracle and Servatrice
; are console applications without a main window, so CloseMainWindow() does
; nothing for them and waiting cannot help; they hold no unsaved user data and
; are force-stopped by PID once the grace period is over. On interactive
; installs and uninstalls the user is prompted to close them instead.
;
; Processes are matched by image name AND by their executable path living
; under the guarded directory ($GuardDir, normally $INSTDIR), so unrelated
; processes that merely share an image name (e.g. the Oracle DB instance
; "oracle.exe") are never touched.

; usage: set $R2 to the base image name (without extension, e.g. "cockatrice",
;        as accepted by Get-Process -Name), call this, result in $R0
;        (1 = running from $GuardDir, 0 = not running, 2 = could not determine)
Function IsAppRunning
    ; Every caller has to set $GuardDir; an empty one would make the path check
    ; below match every process that merely shares the image name, which is the
    ; opposite of what this guard is for.
    ${If} $GuardDir == ""
        StrCpy $R0 2
        Return
    ${EndIf}
    ; Pass the directory to guard to PowerShell via an environment variable so
    ; it never has to be embedded into the -Command string: apostrophes in a
    ; path would terminate the command early, and the path in a -like pattern
    ; would treat [ and ] as wildcard characters.
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    ; Exit the process explicitly so nsExec's pop reflects whether a matching
    ; process was found: powershell.exe exits 0 whether or not the pipeline
    ; produced objects. 0 = not running, 1 = running, anything else = could not
    ; be determined (see IsProcessLookupAvailable).
    ; A process is only a match when its path is readable and lives under
    ; $GuardDir. A process we cannot inspect is not one of ours: the updater is
    ; always started by the application it updates, so it runs in the same user
    ; session, and an interactive install is elevated and can read the paths of
    ; the user's own processes. Assuming a match instead would be a hard stop
    ; rather than a fail-closed guard - an Oracle database instance running as
    ; SYSTEM has a process named "oracle" whose path raises Access denied, and
    ; that would make every silent update abort and every interactive install
    ; loop on the Retry prompt with no way out but Cancel.
    nsExec::ExecToLog 'powershell -NoProfile -Command "if (Get-Process -Name $\'$R2$\' -ErrorAction SilentlyContinue | Where-Object { try { $$p = $$_.Path; if ($$p) { $$p.StartsWith($$env:COCKATRICE_GUARD_DIR, [StringComparison]::OrdinalIgnoreCase) } else { $$false } } catch { $$false } } | Select-Object -First 1) { exit 1 } else { exit 0 }"'
    Pop $R0
    ${If} $R0 == "0"
        StrCpy $R0 0
    ${ElseIf} $R0 == "1"
        StrCpy $R0 1
    ${Else}
        ; powershell.exe could not be run at all (nsExec pushes the string
        ; "error"), or died with an unexpected code. That is "could not
        ; determine", which has to stay distinguishable from "definitely
        ; running": a state that can never clear would block the installer
        ; forever. IsProcessLookupAvailable warns about that case once.
        StrCpy $R0 2
    ${EndIf}
FunctionEnd

; Sets $R0 to 1 if PowerShell can be started at all, 0 if it cannot. The
; running instance check is built on top of it, so a machine where
; powershell.exe is missing, blocked by policy or refused by antivirus is
; reported once instead of being reported as "Cockatrice is running" on every
; single check.
Function IsProcessLookupAvailable
    nsExec::ExecToLog 'powershell -NoProfile -Command "exit 0"'
    Pop $R0
    ${If} $R0 == "0"
        StrCpy $R0 1
    ${Else}
        StrCpy $R0 0
    ${EndIf}
FunctionEnd

Function CloseMatchingApps
    ; gracefully ask every matching instance to close (sends WM_CLOSE)
    ;
    ; The filter here is deliberately the strict one: a process is only asked to
    ; close when its path is readable and lives under $GuardDir. Treating a
    ; process whose path cannot be read (an unrelated oracle.exe / servatrice.exe,
    ; or one owned by another account running as SYSTEM) as a match would make
    ; the installer close a third party application, and it runs elevated, so
    ; CloseMainWindow() would succeed on it.
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    nsExec::ExecToLog 'powershell -NoProfile -Command "Get-Process -Name $\'$R2$\' -ErrorAction SilentlyContinue | Where-Object { try { $$p = $$_.Path; if ($$p) { $$p.StartsWith($$env:COCKATRICE_GUARD_DIR, [StringComparison]::OrdinalIgnoreCase) } else { $$false } } catch { $$false } } | ForEach-Object { $$null = $$_.CloseMainWindow() }"'
    Pop $R3
FunctionEnd

Function ForceCloseApps
    ; stop the matching instances by PID once the grace period is over. Only
    ; used for the console applications ($AllowForceClose), which have no window
    ; to close and hold no unsaved user data. Same strict filter as above.
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    nsExec::ExecToLog 'powershell -NoProfile -Command "Get-Process -Name $\'$R2$\' -ErrorAction SilentlyContinue | Where-Object { try { $$p = $$_.Path; if ($$p) { $$p.StartsWith($$env:COCKATRICE_GUARD_DIR, [StringComparison]::OrdinalIgnoreCase) } else { $$false } } catch { $$false } } | ForEach-Object { Stop-Process -Id $$_.Id -Force -ErrorAction SilentlyContinue }"'
    Pop $R3
FunctionEnd

Function WaitForAppToClose
    ; usage: set $R1 to the display name (e.g. "cockatrice.exe") and $R2 to the
    ; base image name (e.g. "cockatrice")
    Call IsAppRunning
    !insertmacro CockLog "guard $R1: state=$R0 (dir=$GuardDir)"
    ; Only a definite "running" is worth waiting for. "Not running" is done, and
    ; "could not determine" must not keep the installer waiting on an answer it
    ; will never get - but it cannot simply be waved through either, see
    ; HandleUnknownAppState.
    ${If} $R0 = 0
        Return
    ${EndIf}
    ${If} $R0 = 2
        Call HandleUnknownAppState
        Return
    ${EndIf}

    ${If} ${Silent}
        ; ask the application to close gracefully (WM_CLOSE), then wait for it to exit
        DetailPrint "Closing $R1 ..."
        Call CloseMatchingApps
        ; wall-clock deadline: each poll has to spawn a fresh powershell.exe, so
        ; a bare loop counter would not bound the real wait time to 60 s.
        System::Call 'kernel32::GetTickCount()i.s'
        Pop $R8
        ck_wait_loop:
        Sleep 500
        Call IsAppRunning
        ${If} $R0 = 0
            !insertmacro CockLog "guard $R1: closed gracefully"
            DetailPrint "$R1 closed."
            Return
        ${EndIf}
        ${If} $R0 = 2
            Call HandleUnknownAppState
            Return
        ${EndIf}
        System::Call 'kernel32::GetTickCount()i.s'
        Pop $R9
        IntOp $R9 $R9 - $R8
        ${If} $R9 < 60000
            Goto ck_wait_loop
        ${EndIf}
        ; The grace period is over. Cockatrice is never force-stopped: it may be
        ; showing a "really leave this game?" confirmation or holding an unsaved
        ; deck, and nothing has been replaced yet, so aborting just defers the
        ; update and preserves the user's data.
        ${If} $AllowForceClose = 0
            !insertmacro CockLog "guard $R1: still running after 60s, aborting (no force close)"
            DetailPrint "$R1 is still running after 60 s; aborting the update so that no data is lost."
            StrCpy $AbortReason \
                "$R1 did not close within 60 seconds, so the installation was aborted to avoid losing unsaved data.$\r$\n$\r$\nPlease close $R1 and run the installer again."
            Call AbortInstall
        ${EndIf}
        ; Oracle and Servatrice are console applications: CloseMainWindow()
        ; returned false for them and waiting longer cannot help. They hold no
        ; unsaved user data, so stop them by PID.
        !insertmacro CockLog "guard $R1: still running after 60s, force-stopping"
        DetailPrint "Force-stopping $R1 ..."
        Call ForceCloseApps
        Sleep 2000
        Call IsAppRunning
        ${If} $R0 = 0
            !insertmacro CockLog "guard $R1: force-stopped"
            DetailPrint "$R1 closed."
            Return
        ${EndIf}
        ${If} $R0 = 2
            Call HandleUnknownAppState
            Return
        ${EndIf}
        !insertmacro CockLog "guard $R1: could not be stopped, aborting"
        DetailPrint "$R1 is still running after 60 s; aborting the update."
        StrCpy $AbortReason \
            "$R1 could not be stopped, so the installation was aborted.$\r$\n$\r$\nPlease close $R1 and run the installer again."
        Call AbortInstall
    ${Else}
        ck_wait_prompt:
        MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON1 \
            "$R1 is still running.$\r$\n$\r$\nPlease close it, then click Retry.$\r$\nClick Cancel to abort." \
            IDCANCEL ck_abort_install
        Call IsAppRunning
        ${If} $R0 = 0
            Return
        ${EndIf}
        ${If} $R0 = 2
            Call HandleUnknownAppState
            Return
        ${EndIf}
        Goto ck_wait_prompt
        ck_abort_install:
        Abort
    ${EndIf}
FunctionEnd

; A process lookup came back "could not determine": powershell.exe ran but the
; answer was not usable, e.g. the process could not be inspected at all. That
; state is fine to carry on from when the user is at the keyboard - they can
; close the applications by hand and the stale-runtime check still stands
; between a failed guard and a mismatched install.
;
; It is not fine on a silent upgrade. There the whole point of this guard is to
; know that nothing holds locks on the files about to be replaced, and this is
; exactly the state where that is unknown, with no one to click a dialog: the
; old code returned and let the install carry on into the mixed Qt DLL set that
; made the next start fail. Cancel the update instead; the next attempt runs
; with the applications closed and succeeds.
Function HandleUnknownAppState
    ${If} $ReinstallMode = 1
        StrCpy $AbortReason \
            "The installation was cancelled because it could not determine whether Cockatrice is still running.$\r$\n$\r$\nPlease close Cockatrice, Oracle and Servatrice and run the installer again."
        Call AbortInstall
    ${EndIf}
    DetailPrint "Could not determine whether $R1 is running; close Cockatrice, Oracle and Servatrice manually if the installation reports a problem."
    ${IfNot} ${Silent}
        MessageBox MB_OK|MB_ICONEXCLAMATION \
            "This installer could not determine whether $R1 is still running.$\r$\n$\r$\nPlease close Cockatrice, Oracle and Servatrice manually before continuing."
    ${EndIf}
FunctionEnd

Function EnsureAppsNotRunning
    ; guards the install directory
    StrCpy $GuardDir $INSTDIR
    Call GuardAppsNotRunning
FunctionEnd

; Waits for the application that launched this installer to exit, using the /PID=
; it passed. Sets $LauncherExited to 1 when that process is provably gone.
;
; This is the authority for the one question the rest of the installer can only
; approximate: is the process that started us still holding its Qt DLLs mapped?
; A mapped module cannot be deleted, so an answer of "not running" obtained by
; enumerating process names is what allows the update to write a new payload on
; top of files it could not remove. Waiting on a real handle has no such gap,
; needs no powershell.exe, and is unaffected by a process whose path cannot be
; read - the case the name based check has to treat as "not ours" so it never
; closes a third party application.
;
; Sets $LauncherExited to 0 when no usable /PID= was given, which is the older
; client that does not pass one; the caller then has no authority available and
; falls back to the name based guard.
Function WaitForLauncherToExit
    StrCpy $LauncherExited 0
    !insertmacro CockLog "wait-launcher: pid='$WaitPid'"
    ${If} $WaitPid == ""
        !insertmacro CockLog "wait-launcher: no pid given, falling back to name check"
        Return
    ${EndIf}
    ; PROCESS_SYNCHRONIZE is the only access this needs - enough to wait on the
    ; handle, and the least likely to be refused for a reason that has nothing to
    ; do with the upgrade.
    ;
    ; System.dll maps its output registers so that .rN writes $N and .RN writes
    ; $RN. The caller reads $R0/$R1, so the uppercase .RN form is required; the
    ; original .r0/.r1 wrote $0/$1 and left $R1 empty, which made the "did not
    ; exit" test below compare "" against "0" and abort on every /PID= upgrade.
    ClearErrors
    System::Call 'kernel32::OpenProcess(i 0x00100000, i 0, i $WaitPid) p.R0'
    ; OpenProcess returns a null handle for a process that is already gone. The
    ; System plugin does not raise the NSIS error flag from a called function's
    ; return value, so ${Errors} alone never saw that case and the null handle was
    ; passed straight to WaitForSingleObject. Fold a plugin failure into the same
    ; null-handle case instead of reading a handle that was never written.
    ${If} ${Errors}
        StrCpy $R0 0
    ${EndIf}
    ${If} $R0 == 0
    ; The launcher is gone - OpenProcess cannot open a dead process - which is
    ; exactly the state the wait was for.
        !insertmacro CockLog "wait-launcher: already exited"
        DetailPrint "The application that started this installer has already exited."
        StrCpy $LauncherExited 1
        Return
    ${EndIf}
    DetailPrint "Waiting for the application that started this installer to exit ..."
    ; Same 60 s grace period the name based check uses, so a client that is slow
    ; to flush its card database does not get less room than it used to.
    ; WAIT_OBJECT_0 is 0; WAIT_TIMEOUT is 258 and a failed wait reports 0xFFFFFFFF.
    System::Call 'kernel32::WaitForSingleObject(p R0, i 60000) i.R1'
    System::Call 'kernel32::CloseHandle(p R0)'
    ${If} $R1 != 0
        !insertmacro CockLog "wait-launcher: did not exit within 60s (wait=$R1)"
        DetailPrint "The application that started this installer did not exit within 60 s."
        StrCpy $AbortReason \
            "The application that started this update has not exited yet, so the installation was cancelled rather than replace files it is still using.$\r$\n$\r$\nPlease close Cockatrice and run the installer again."
        Call AbortInstall
    ${EndIf}
    !insertmacro CockLog "wait-launcher: launcher exited"
    DetailPrint "The application that started this installer has exited."
    StrCpy $LauncherExited 1
FunctionEnd

Function EnsureOldInstallNotRunning
    ; Guards the directory of a *previous* installation, as recorded in the
    ; registry. Needed by componentsPagePre, which runs the old uninstaller
    ; before the directory page, so $INSTDIR is still the default there and not
    ; the directory the old install lives in. Every uninstaller shipped before
    ; this change had no running instance guard of its own, so the old one would
    ; delete files while an instance of the application is still holding them.
    ReadRegStr $R3 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice" "InstallLocation"
    ${If} $R3 == ""
        ; no usable record of where the old install is; $INSTDIR is the default
        ; install directory at this point, which is where it usually is anyway
        DetailPrint "Could not read the previous installation directory from the registry; checking $INSTDIR instead."
        StrCpy $R3 $INSTDIR
    ${EndIf}
    StrCpy $GuardDir $R3
    Call GuardAppsNotRunning
FunctionEnd

Function GuardAppsNotRunning
    ; usage: set $GuardDir to the directory to guard, then call this
    Call IsProcessLookupAvailable
    !insertmacro CockLog "guard: powershell available=$R0 dir=$GuardDir launcherExited=$LauncherExited"
    ${If} $R0 = 0
        ; Without PowerShell we cannot tell whether a running instance is
        ; holding locks on the files we are about to replace.
        !insertmacro CockLog "guard: powershell unavailable, skipping instance check"
        DetailPrint "Could not start powershell.exe; skipping the running instance check. Close Cockatrice, Oracle and Servatrice manually if the installation reports a problem."
        ; On a silent upgrade that used to be a warning and nothing more, which
        ; left the very process that launched the installer free to go on
        ; replacing files it still had mapped - the one outcome this guard exists
        ; to prevent - and then writing the new payload on top of them. Cancel
        ; instead. An interactive install can still continue: the user is at the
        ; keyboard, and refusing outright would leave them with an installer that
        ; can never complete.
        ;
        ; Unless the launcher has already been waited on directly: WaitForLauncherToExit
        ; needs no powershell.exe, so when it has run, Cockatrice is known to be
        ; gone and the lookup failing to run costs nothing but the Oracle and
        ; Servatrice checks. An older client that passes no /PID= leaves no such
        ; authority, and there the upgrade is cancelled as before.
        ${If} $LauncherExited != 1
        ${AndIf} $ReinstallMode = 1
            StrCpy $AbortReason \
                "The installation was cancelled because it could not check whether Cockatrice is still running, because powershell.exe could not be started.$\r$\n$\r$\nPlease close Cockatrice, Oracle and Servatrice and run the installer again."
            Call AbortInstall
        ${EndIf}
        ; The user has to be told to close the applications by hand, but a /S
        ; install without /R is unattended and would block on this forever.
        ${If} $ReinstallMode = 1
        ${OrIfNot} ${Silent}
            MessageBox MB_OK|MB_ICONEXCLAMATION "This installer could not check whether Cockatrice is still running, because powershell.exe could not be started.$\r$\n$\r$\nPlease close Cockatrice, Oracle and Servatrice manually before continuing."
        ${EndIf}
        Return
    ${EndIf}
    StrCpy $R1 "cockatrice.exe"
    StrCpy $R2 "cockatrice"
    ; a GUI application with unsaved user data: graceful close only
    StrCpy $AllowForceClose 0
    Call WaitForAppToClose
    StrCpy $R1 "oracle.exe"
    StrCpy $R2 "oracle"
    StrCpy $AllowForceClose 1
    Call WaitForAppToClose
    StrCpy $R1 "servatrice.exe"
    StrCpy $R2 "servatrice"
    StrCpy $AllowForceClose 1
    Call WaitForAppToClose
FunctionEnd

; Uninstaller copies of the same routines (the uninstaller gets its own
; function set compiled in, it cannot call the installer functions).
Function un.IsAppRunning
    ; An unset $GuardDir would match every process that merely shares the image
    ; name, see IsAppRunning in the installer.
    ${If} $GuardDir == ""
        StrCpy $R0 2
        Return
    ${EndIf}
    ; Pass the directory to guard to PowerShell via an environment variable so
    ; it never has to be embedded into the -Command string: apostrophes in a
    ; path would terminate the command early, and the path in a -like pattern
    ; would treat [ and ] as wildcard characters.
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    ; Exit the process explicitly so nsExec's pop reflects whether a matching
    ; process was found: powershell.exe exits 0 whether or not the pipeline
    ; produced objects. 0 = not running, 1 = running, anything else = could not
    ; be determined (see un.IsProcessLookupAvailable).
    ; Strict match: a readable path under $GuardDir, same as in the installer.
    nsExec::ExecToLog 'powershell -NoProfile -Command "if (Get-Process -Name $\'$R2$\' -ErrorAction SilentlyContinue | Where-Object { try { $$p = $$_.Path; if ($$p) { $$p.StartsWith($$env:COCKATRICE_GUARD_DIR, [StringComparison]::OrdinalIgnoreCase) } else { $$false } } catch { $$false } } | Select-Object -First 1) { exit 1 } else { exit 0 }"'
    Pop $R0
    ${If} $R0 == "0"
        StrCpy $R0 0
    ${ElseIf} $R0 == "1"
        StrCpy $R0 1
    ${Else}
        ; powershell.exe could not be run at all (nsExec pushes the string
        ; "error"), or died with an unexpected code. That is "could not
        ; determine", which has to stay distinguishable from "definitely
        ; running": a state that can never clear would block the uninstaller
        ; forever. un.IsProcessLookupAvailable warns about that case once.
        StrCpy $R0 2
    ${EndIf}
FunctionEnd

Function un.IsProcessLookupAvailable
    nsExec::ExecToLog 'powershell -NoProfile -Command "exit 0"'
    Pop $R0
    ${If} $R0 == "0"
        StrCpy $R0 1
    ${Else}
        StrCpy $R0 0
    ${EndIf}
FunctionEnd

Function un.CloseMatchingApps
    ; gracefully ask every matching instance to close (sends WM_CLOSE)
    ;
    ; Strict filter, same as in the installer: never close a process whose path
    ; cannot be read, it is not necessarily one of ours.
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    nsExec::ExecToLog 'powershell -NoProfile -Command "Get-Process -Name $\'$R2$\' -ErrorAction SilentlyContinue | Where-Object { try { $$p = $$_.Path; if ($$p) { $$p.StartsWith($$env:COCKATRICE_GUARD_DIR, [StringComparison]::OrdinalIgnoreCase) } else { $$false } } catch { $$false } } | ForEach-Object { $$null = $$_.CloseMainWindow() }"'
    Pop $R3
FunctionEnd

Function un.ForceCloseApps
    ; stop the matching instances by PID once the grace period is over, see
    ; ForceCloseApps in the installer.
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    nsExec::ExecToLog 'powershell -NoProfile -Command "Get-Process -Name $\'$R2$\' -ErrorAction SilentlyContinue | Where-Object { try { $$p = $$_.Path; if ($$p) { $$p.StartsWith($$env:COCKATRICE_GUARD_DIR, [StringComparison]::OrdinalIgnoreCase) } else { $$false } } catch { $$false } } | ForEach-Object { Stop-Process -Id $$_.Id -Force -ErrorAction SilentlyContinue }"'
    Pop $R3
FunctionEnd

Function un.WaitForAppToClose
    Call un.IsAppRunning
    ; Only a definite "running" is worth waiting for, see WaitForAppToClose.
    ${If} $R0 != 1
        Return
    ${EndIf}

    ${If} ${Silent}
        DetailPrint "Closing $R1 ..."
        Call un.CloseMatchingApps
        ; wall-clock deadline: each poll has to spawn a fresh powershell.exe, so
        ; a bare loop counter would not bound the real wait time to 60 s.
        System::Call 'kernel32::GetTickCount()i.s'
        Pop $R8
        un_ck_wait_loop:
        Sleep 500
        Call un.IsAppRunning
        ${If} $R0 = 0
            DetailPrint "$R1 closed."
            Return
        ${EndIf}
        ${If} $R0 != 1
            Return
        ${EndIf}
        System::Call 'kernel32::GetTickCount()i.s'
        Pop $R9
        IntOp $R9 $R9 - $R8
        ${If} $R9 < 60000
            Goto un_ck_wait_loop
        ${EndIf}
        ; The grace period is over. Cockatrice is never force-stopped, see
        ; WaitForAppToClose in the installer. A silent uninstall only reports
        ; through the log and its exit code: it is also run unattended (the /R
        ; updater uses it via QuietUninstallString, package managers use it
        ; directly), and a message box would block there. The installer that ran
        ; us reports the incomplete result to the user itself.
        ${If} $AllowForceClose = 0
            DetailPrint "$R1 is still running after 60 s; aborting so that no data is lost."
            SetErrorLevel 1
            Abort
        ${EndIf}
        DetailPrint "Force-stopping $R1 ..."
        Call un.ForceCloseApps
        Sleep 2000
        Call un.IsAppRunning
        ${If} $R0 = 0
            DetailPrint "$R1 closed."
            Return
        ${EndIf}
        ${If} $R0 != 1
            Return
        ${EndIf}
        DetailPrint "$R1 is still running after 60 s; aborting."
        SetErrorLevel 1
        Abort
    ${Else}
        un_ck_wait_prompt:
        MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON1 \
            "$R1 is still running.$\r$\n$\r$\nPlease close it, then click Retry.$\r$\nClick Cancel to abort." \
            IDCANCEL un_ck_abort_install
        Call un.IsAppRunning
        ${If} $R0 = 0
            Return
        ${EndIf}
        ${If} $R0 != 1
            Return
        ${EndIf}
        Goto un_ck_wait_prompt
        un_ck_abort_install:
        Abort
    ${EndIf}
FunctionEnd

Function un.EnsureAppsNotRunning
    ; guards the install directory; the uninstaller only ever has to check this one
    StrCpy $GuardDir $INSTDIR
    Call un.GuardAppsNotRunning
FunctionEnd

Function un.GuardAppsNotRunning
    Call un.IsProcessLookupAvailable
    ${If} $R0 = 0
        ; Same reasoning as in the installer: without PowerShell we cannot tell
        ; whether a running instance still holds file locks, and refusing to
        ; uninstall would leave the user with an uninstaller that never
        ; completes. Warn once and continue.
        DetailPrint "Could not start powershell.exe; skipping the running instance check. Close Cockatrice, Oracle and Servatrice manually if the uninstall reports a problem."
        MessageBox MB_OK|MB_ICONEXCLAMATION "This uninstaller could not check whether Cockatrice is still running, because powershell.exe could not be started.$\r$\n$\r$\nPlease close Cockatrice, Oracle and Servatrice manually before continuing."
        Return
    ${EndIf}
    StrCpy $R1 "cockatrice.exe"
    StrCpy $R2 "cockatrice"
    ; a GUI application with unsaved user data: graceful close only
    StrCpy $AllowForceClose 0
    Call un.WaitForAppToClose
    StrCpy $R1 "oracle.exe"
    StrCpy $R2 "oracle"
    StrCpy $AllowForceClose 1
    Call un.WaitForAppToClose
    StrCpy $R1 "servatrice.exe"
    StrCpy $R2 "servatrice"
    StrCpy $AllowForceClose 1
    Call un.WaitForAppToClose
FunctionEnd
