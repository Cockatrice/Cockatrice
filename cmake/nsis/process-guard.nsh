; Running instance handling: make sure no installed application still holds the
; files that are about to be deleted or replaced, closing or force-stopping it
; where the caller allows that, and prompting the user where it does not.
;
; The probe routines exist twice, once plain and once with the un prefix. That is
; forced by NSIS: the uninstaller is a separate program compiled into the same
; installer, and it cannot call the installer's functions. Keep the two copies in
; step. The launcher wait is installer-only: the uninstaller is never given a
; /PID= to wait on.
;
; Register contract:
;   $GuardDir         in:  directory a matched process has to run from
;   $GuardExit        out: RunHolderProbe - 0 when nothing holds the payload, 1
;                          when its line names what does, anything else = no answer
;   $GuardOutput      out: RunHolderProbe - its one-line answer
;   $GuardCloseRounds/$GuardLastHolders/$GuardRecheckCount
;                     internal: the guard's automatic close and its rechecks
;   $GuardPrompt      internal: the question CockEndPrompt/CockEndRecheckPrompt build
;   $WaitPid          in:  WaitForLauncherToExit - the launcher's /PID=
;   $WaitSlice/$WaitCanEnd/$WaitWindowless
;                     internal: the launcher wait's slice and what it may do
;   $LauncherExited   out: WaitForLauncherToExit - 1 when the /PID= process was
;                          seen to exit, 0 when no usable /PID= was given
;   $AbortReason      out: set before AbortInstall on every abort path
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
; Both questions this installer has about running processes - "may I replace
; the payload?" and "what is holding this file I could not delete?" - are
; answered by tools/installer-holder-probe, a small executable built from this
; repository and embedded in the installer and the uninstaller alike. It names
; every process and service that has one of the payload files open or mapped
; (the Restart Manager, RmRegisterResources/RmGetList) and, separately, every
; payload program whose image lies in the directory being replaced rather than
; in a folder below it.
;
; The second lookup is not redundant. Restart Manager answers "who has this
; file open", and a running instance does not have to have one open: its own
; image and the modules it loaded are mapped into it rather than opened by it,
; and a mapped image is not reliably reported. That is the state the failed
; updates this probe exists for were in - every DLL still on disk, an instance
; running from the directory, and "nothing is holding them any more" as the
; answer. A guard built on file handles alone therefore misses exactly the
; instance it exists to find; it would pass, the purge would fail, and the
; install would end in a warning with no offer to close and try again.
;
; Why not enumerate processes from this script: the NSIS stub is a 32-bit
; process, so a bare "powershell" started from it is the 32-bit Windows
; PowerShell, whose Get-Process cannot read the path of a 64-bit process. A
; guard built on that misses exactly the running instance it exists to find.
; The probe opens each process with PROCESS_QUERY_LIMITED_INFORMATION and calls
; QueryFullProcessImageNameW, which has no bitness mismatch, and Restart
; Manager is a system API rather than a view of this process's architecture.
;
; The files that count are the payload the purge removes and the installer
; replaces: the three executables, the top-level DLLs, qt.conf and everything
; below Plugins. A document, a log or a screenshot the user happens to have
; open in the install directory is not part of it and cannot block an upgrade;
; the image lookup matches on the directory a process runs from, so it does not
; widen that set.
;
; Nothing is force-stopped without the user asking for it: Cockatrice may be holding
; an unsaved deck or showing a "really leave this game?" confirmation, Oracle writes
; the card database in place and can be killed mid-write, and Servatrice may be serving
; a game. A close is a request, made automatically because that is what the user started
; the installer to do: the probe first asks every instance over its local socket, which
; reaches a hidden window and an instance sitting on its own confirmation, and falls back
; to WM_CLOSE for an older client. The automatic close is retried while the set of holders
; changes, so one instance exiting does not leave the next one unasked. When a close is
; still not answered the user is told which program is in the way and is offered to force
; close all three, check again after closing them by hand, or abort - the last resort, with
; what it costs spelled out in the prompt.

; One sentence for every prompt whose Yes ends the whole set, so that answer reads the same
; wherever the user meets it: this guard before anything is replaced, the uninstaller's, and
; the recovery prompt after a failed purge.
!define END_COST "An unsaved deck, a running game or an unwritten setting is lost, Servatrice drops its players, and an Oracle that is mid-write can damage the card database."

; What the user does instead when they say no: the prompt's other answer stops the run, so they
; are told how to get past the same wall next time. Also used by the abort dialogs, which is
; where the answer to "why did it stop" ends up.
!define AFTER_CLOSING_GUIDANCE "Answer the confirmation Cockatrice may be showing - open it from the notification area if it is hidden there - and use Task Manager for a program that would not end."

; The escape hatch for a user who would rather not force-close anything: the same release,
; downloaded and run by hand once Cockatrice is closed. Every abort dialog shows this and
; offers to open the page, so an update that cannot free the files still leaves the user a
; way forward that does not end in Task Manager.
!define LATEST_RELEASE_URL "https://github.com/Cockatrice/Cockatrice/releases/latest"
!define MANUAL_UPDATE_GUIDANCE "To update safely instead, close Cockatrice and download the latest installer from:$\r$\n${LATEST_RELEASE_URL}$\r$\nThen run it manually."

; usage: set the variable named by the second argument to the probe's listing, then call
; this. Sets $GuardPrompt to the question the guard and the recovery path ask.
;
; One macro rather than three copies, because the wording is a promise: Yes ends the
; programs the payload installs - which is exactly the set EndHolders acts on - and anything
; else the probe named has to be closed by hand, so a listing that includes a scanner or a
; service hosted elsewhere does not come with an answer that cannot do what it says.
;
; Two answers, one of each: the automatic close already ran when this is asked, so Yes is the
; only thing left that can free the files and No stops the run. The guard itself uses
; CockEndRecheckPrompt instead, whose No checks again; this one stays for the recovery path,
; where a retry cannot change anything because the purge was already retried.
;
; The assembled string has to fit the characters one NSIS string holds, which is what
; bounds HOLDER_LINE_CHAR_MAX in tools\installer-holder-probe\holder_line.h: with the
; longest listing that can arrive it stays inside that budget instead of being cut off at
; the tail, where the cancel line is.
;
; @param header the sentence that introduces the listing, with its trailing colon
; @param listing the variable holding the probe's line, $GuardOutput or $HolderOutput
; @param cancelWord what No gives up, "installation" or "uninstallation"
!macro CockEndPrompt header listing cancelWord
    StrCpy $GuardPrompt "${header}$\r$\n$\r$\n${listing}"
    StrCpy $GuardPrompt "$GuardPrompt$\r$\n$\r$\nChoose Yes to force close the Cockatrice, Oracle and Servatrice programs listed above. ${END_COST}$\r$\n$\r$\nChoose No to cancel the ${cancelWord}. ${AFTER_CLOSING_GUIDANCE}"
!macroend

; usage: like CockEndPrompt, for a MB_YESNOCANCEL question whose No checks again.
;
; The automatic close has already run, so the three answers are what is left: force close
; the programs the payload installs (Yes), close them by hand and check again (No), or stop
; the run (Cancel). No used to be the only alternative to ending them, which meant a user who
; would rather close the instance themselves had to abandon the update and start the
; installer again. The listing the question is asked with is refreshed on every pass, so the
; question changes as programs go away instead of repeating itself word for word.
;
; The manual-update guidance lives in the abort dialog, which is where a Cancel ends up, not
; in this prompt. With the longest listing HOLDER_LINE_CHAR_MAX allows, the assembled string
; stays under 900 characters, inside the 1024 one NSIS string holds, so the Cancel line is
; never the part that gets cut off.
;
; @param header the sentence that introduces the listing, with its trailing colon
; @param listing the variable holding the probe's line, $GuardOutput
; @param cancelWord what Cancel gives up, "installation" or "uninstallation"
!macro CockEndRecheckPrompt header listing cancelWord
    StrCpy $GuardPrompt "${header}$\r$\n$\r$\n${listing}"
    StrCpy $GuardPrompt "$GuardPrompt$\r$\n$\r$\nChoose Yes to force close the Cockatrice, Oracle and Servatrice programs listed above. ${END_COST}$\r$\n$\r$\nChoose No to close them yourself, then check again - answer the confirmation Cockatrice may be showing, and open it from the notification area if it is hidden there.$\r$\n$\r$\nChoose Cancel to abort the ${cancelWord}."
!macroend

; Emitted once for the installer and once for the uninstaller: the uninstaller
; cannot call installer functions, and its own copy of the probe keeps a running
; 64-bit instance from being missed there too.
!macro CockHolderProbeFunctions prefix tag
Function ${prefix}ExtractHolderProbe
    InitPluginsDir
    IfFileExists "$PLUGINSDIR\installer-holder-probe.exe" ck_probe_ready_${tag}
        File "/oname=$PLUGINSDIR\installer-holder-probe.exe" "..\..\..\installer-holder-probe.exe"
    ck_probe_ready_${tag}:
FunctionEnd

; usage: set $GuardDir to the directory to check, then call this. Sets
; $GuardExit to the probe's exit code and $GuardOutput to its one-line answer.
; 0 = nothing holds the payload and no payload program runs from the directory, 1 = the
; line names what does, anything else = the probe could not answer (2 = no
; directory was given, 3 = no payload program was named but one refuses to be replaced, or
; the Restart Manager call failed and no payload program runs from the directory either,
; "timeout" = it did not return in time).
Function ${prefix}RunHolderProbe
    Call ${prefix}ExtractHolderProbe
    ; The directory travels in an environment variable, never on the command
    ; line: a path with spaces, quotes or a trailing backslash then does not
    ; have to survive NSIS quoting.
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    ; /TIMEOUT because this can run before the first page: a Restart Manager
    ; that does not answer must not leave the installer with no window and no
    ; way out.
    nsExec::ExecToStack /TIMEOUT=20000 '"$PLUGINSDIR\installer-holder-probe.exe"'
    Pop $GuardExit
    Pop $GuardOutput
FunctionEnd

; usage: set $GuardDir to the directory being replaced, then call this. Asks the running
; Cockatrice to close, and waits for it to exit.
;
; The set is what the probe names: the processes holding a file of the version being
; replaced, plus the payload programs running from the directory itself. The probe
; closes that set by pid, which is why nothing here has to match an executable path. Only
; cockatrice.exe is asked: Oracle writes the card database in place and Servatrice has no
; window to ask, so both are left alone here and only ended by the explicit answer.
;
; Each instance is asked to close over the local socket it listens on, which is the same
; request as clicking its close button and reaches one whose window is hidden in the
; notification area or is sitting on its own confirmation. A client from before that socket
; is asked with WM_CLOSE on its own window instead, and a visible window is brought forward
; first. Either way the close is answered, not obeyed - Cockatrice asks before it quits while
; a card database update is running, a game is open or a deck is unsaved - and that question
; is only useful if it is in front of the installer instead of behind it.
;
; The probe waits up to 30 s for the processes it asked to exit, so the check that follows
; measures a settled directory rather than sleeping for a fixed time. Its line goes into the
; log as it stands: it names every holder, including the ones left to the user, so a close
; that did nothing can be told apart from one that was never sent.
;
; Never a force-close here: the application decides whether to quit. Ending is a separate
; answer the guard prompt offers when a close did not free the files, spelled out with what
; it costs - see EndHolders - and it ends every program the payload installs.
Function ${prefix}CloseHolders
    Call ${prefix}ExtractHolderProbe
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    ; The probe waits 30 s itself. The timeout here only bounds a probe that never returns;
    ; the Restart Manager call inside it is the same one the holder listing makes.
    nsExec::ExecToStack /TIMEOUT=40000 '"$PLUGINSDIR\installer-holder-probe.exe" --close-holders'
    Pop $R3
    Pop $R4
    ; Not the CockLog macro: this function exists twice and the uninstaller copy may only
    ; Call un. functions, so the macro's two lines are spelled with the prefix here.
    Push "close-holders: probe exit=$R3 [$R4]"
    Call ${prefix}CockLogWrite
    DetailPrint "Closing Cockatrice: $R4"
FunctionEnd

; usage: set $GuardDir to the directory being replaced, then call this. Ends the running
; Cockatrice, Oracle and Servatrice instances the probe names.
;
; This is the only place anything is stopped without being asked to stop, so it is only
; reached after the user picked it from a prompt that says what it costs: a client that is
; still running may have an unsaved deck, a game or a settings write in flight, Oracle may
; be part-way through writing the card database, and Servatrice may be serving a game. The
; probe ends the three programs the payload installs, services included, and nothing else:
; a scanner or a program the user keeps in the directory is named and left to the user.
Function ${prefix}EndHolders
    Call ${prefix}ExtractHolderProbe
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    ; The probe waits up to 5 s for what it ended to be gone, so the check that follows
    ; measures a settled directory instead of racing the processes it just terminated.
    nsExec::ExecToStack /TIMEOUT=20000 '"$PLUGINSDIR\installer-holder-probe.exe" --end-holders'
    Pop $R3
    Pop $R4
    Push "end-holders: probe exit=$R3 [$R4]"
    Call ${prefix}CockLogWrite
    DetailPrint "Ending Cockatrice, Oracle and Servatrice: $R4"
FunctionEnd
!macroend

!insertmacro CockHolderProbeFunctions "" install
!insertmacro CockHolderProbeFunctions "un." uninstall

; usage: set $GuardDir to the directory to guard, then call this.
;
; Asks Cockatrice to close once, then asks one question with one answer that can free the
; payload. The automatic close is the same request clicking the window's close button makes,
; so the common case - the client was still shutting down - never turns into a question. The
; prompt is what remains when the close cannot reach everything: Oracle writes the card
; database in place, Servatrice has no window, and a client that is sitting on its own
; confirmation needs an answer nobody can give it from here. It offers to end the programs
; the payload installs - and only those, which the text says - with what that costs (Yes),
; and its other answer stops the install (No) instead of repeating the same dialog. What
; survives an end is reported in the abort, never asked about again - ending is the last
; resort, see EndHolders.
;
; An error is always shown, in every mode. A /S install without /R has nobody at
; the keyboard, but this installer does not promise unattended runs: when an
; error needs a decision, asking for one is better than silently leaving a
; half-updated install behind. The happy path has no dialog, so a /S reinstall
; that has nothing to report stays silent.
Function GuardAppsNotRunning
    !insertmacro CockLog "guard: checking $GuardDir (launcherExited=$LauncherExited)"
    StrCpy $GuardCloseRounds 0
    StrCpy $GuardRecheckCount 0
    StrCpy $GuardLastHolders ""
    guard_check:
    Call RunHolderProbe
    ${If} $GuardExit == "0"
        !insertmacro CockLog "guard: nothing is holding the payload"
        DetailPrint "No running Cockatrice, Oracle or Servatrice instance was found."
        Return
    ${EndIf}
    ${If} $GuardExit == "1"
        !insertmacro CockLog "guard: still in use by $GuardOutput"
        ; Close it for the user first. The probe asks every instance over its local socket
        ; and falls back to WM_CLOSE for an older client, so this is the same request as
        ; clicking the window's close button: the application still answers its own "really
        ; quit?" prompt, so nothing is lost. A second round is spent only when the listing
        ; changed, which means something exited and the rest is worth asking again; repeating
        ; the same request against the same processes cannot change anything.
        ${If} $GuardCloseRounds < 2
        ${AndIf} $GuardOutput != $GuardLastHolders
            StrCpy $GuardLastHolders $GuardOutput
            IntOp $GuardCloseRounds $GuardCloseRounds + 1
            !insertmacro CockLog "guard: asking the payload programs to close (round $GuardCloseRounds)"
            Call CloseHolders
            Goto guard_check
        ${EndIf}
        DetailPrint "The install directory is in use by: $GuardOutput"
        ; The close ran and did not free the files. Force-closing them is what the installer
        ; can do (Yes); closing them by hand is what the user can do (No checks again, with
        ; the fresh listing); Cancel stops the run and the abort dialog offers the same
        ; update from the official download page. Without the middle answer a user who did
        ; not want anything killed had no way forward but to abandon the update.
        guard_ask:
        !insertmacro CockEndRecheckPrompt "Cockatrice, Oracle or Servatrice is still using files of the installation:" $GuardOutput "installation"
        MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON2 "$GuardPrompt" IDYES guard_end IDNO guard_recheck
        Goto guard_cancel
        guard_recheck:
        IntOp $GuardRecheckCount $GuardRecheckCount + 1
        !insertmacro CockLog "guard: user closed programs by hand, checking again (recheck $GuardRecheckCount)"
        ; A fresh set gets a fresh automatic close, exactly as if the guard had just started:
        ; what the user closed by hand may have been one instance of several.
        StrCpy $GuardCloseRounds 0
        StrCpy $GuardLastHolders ""
        Goto guard_check
        guard_cancel:
        !insertmacro CockLog "guard: cancelled by user after $GuardRecheckCount recheck(s)"
        StrCpy $AbortReason \
            "The installation was cancelled because Cockatrice, Oracle or Servatrice is still running and holding files of the previous version.$\r$\n$\r$\n${AFTER_CLOSING_GUIDANCE}$\r$\n$\r$\nClose them and run the installer again."
        ; RemoveOldPayloadLeftovers runs with the pre-uninstall backup already taken;
        ; restoring here keeps this abort from stranding the only copy in %TEMP%.
        Call RestoreUserDataAfterUninstall
        Call AbortInstall
        ; Only reached through Yes: AbortInstall does not return.
        guard_end:
        !insertmacro CockLog "guard: ending the payload programs on request, then checking again"
        Call EndHolders
        ; Measure once and report, rather than asking the same question again. Everything the
        ; payload installs was ended, so what survived is a program this installer may not
        ; open or does not own - another dialog offering to end it again would only repeat
        ; itself.
        Call RunHolderProbe
        ${If} $GuardExit == "0"
            !insertmacro CockLog "guard: nothing is holding the payload"
            Return
        ${EndIf}
        !insertmacro CockLog "guard: still in use after ending them ($GuardExit): $GuardOutput"
        ${If} $GuardExit == "1"
            StrCpy $AbortReason \
                "The installation was cancelled because these programs still hold files of the previous version and could not be ended:$\r$\n$\r$\n$GuardOutput$\r$\n$\r$\n${AFTER_CLOSING_GUIDANCE}$\r$\n$\r$\nClose them and run the installer again."
        ${Else}
            StrCpy $AbortReason \
                "The installation was cancelled because, after ending Cockatrice, Oracle and Servatrice, the installer could not check whether they still hold files of the previous version.$\r$\n$\r$\nThe check reported: $GuardOutput$\r$\n$\r$\nClose them and run the installer again."
        ${EndIf}
        Call RestoreUserDataAfterUninstall
        Call AbortInstall
    ${EndIf}
    ; Not an answer at all: the probe was not configured, the Restart Manager
    ; call failed and no payload program runs from the directory, a payload
    ; program refuses to be replaced but no lookup could name it (it may run
    ; under another account), or the probe timed out. Carrying on from that is a
    ; decision, so it is asked for rather than assumed - the stale-runtime check
    ; after the purge still stands between a failed guard and a mismatched
    ; install, but it only fires after files have been replaced, and on an
    ; upgrade the launcher asked for nobody is watching the installer's exit
    ; code. Retry asks the probe again (a Restart Manager that was busy answers
    ; the second time, and a program the user has closed frees its file), Ignore
    ; installs anyway, Abort cancels instead of writing a payload next to files
    ; the guard could not rule out.
    !insertmacro CockLog "guard: the holder probe could not answer ($GuardExit): $GuardOutput"
    DetailPrint "Could not confirm whether Cockatrice is still running: $GuardOutput"
    MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION|MB_DEFBUTTON1 \
        "This installer could not confirm whether Cockatrice, Oracle or Servatrice is still running.$\r$\n$\r$\nThe check reported: $GuardOutput$\r$\n$\r$\nClose them, then choose Retry to check again. Choose Ignore to install anyway, or Abort to cancel the installation." \
        IDRETRY guard_check IDABORT guard_unknown_abort
    !insertmacro CockLog "guard: continuing without an answer from the probe"
    Return
    guard_unknown_abort:
    StrCpy $AbortReason \
        "The installation was cancelled because it could not confirm whether Cockatrice, Oracle or Servatrice is still running.$\r$\n$\r$\nThe check reported: $GuardOutput$\r$\n$\r$\nPlease close them and run the installer again."
    ; Same reason as the cancel path above: this abort can be reached from the
    ; leftover-removal guard, after the backup was taken.
    Call RestoreUserDataAfterUninstall
    Call AbortInstall
FunctionEnd

Function EnsureAppsNotRunning
    ; guards the install directory
    StrCpy $GuardDir $INSTDIR
    Call GuardAppsNotRunning
FunctionEnd

; usage: set $GuardDir and $WaitPid, then call this. Asks the one process the installer
; waits on to close and sets $R3 to what it could do: 1 = a window exists and was asked to
; close (or is waiting on a question of its own), 4 = it has no window a user could answer,
; anything else = the probe could not answer. Only 4 lets the wait act on its own; every
; other code keeps the user in the decision, because the installer holds a live handle and
; "the probe did not find it" is not the same answer as "it is gone".
;
; The guard's CloseHolders answers the same question for every holder and leaves a
; windowless instance to the user, which is right there: the prompt names it. The launcher
; is the one process nobody can be asked about - the in-app updater runs the installer
; silent and detached, and a client that has already hidden its window has answered its
; own prompts - so this answer has to say whether there was a window to ask at all.
Function CloseLauncher
    Call ExtractHolderProbe
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_GUARD_DIR", "$GuardDir")'
    System::Call 'kernel32::SetEnvironmentVariable(t, t) i ("COCKATRICE_LAUNCHER_PID", "$WaitPid")'
    ; The probe waits 30 s itself when it posts a close. The timeout here only bounds a
    ; probe that never returns; the Restart Manager call the other modes make is not
    ; reached in this one.
    nsExec::ExecToStack /TIMEOUT=40000 '"$PLUGINSDIR\installer-holder-probe.exe" --close-launcher'
    Pop $R3
    Pop $R4
    Push "close-launcher: probe exit=$R3 [$R4]"
    Call CockLogWrite
    DetailPrint "Closing Cockatrice: $R4"
FunctionEnd

; Waits for the application that launched this installer to exit, using the /PID=
; it passed. Sets $LauncherExited to 1 when that process is provably gone.
;
; This is the authority for the one question the rest of the installer can only
; approximate: is the process that started us still holding its Qt DLLs mapped?
; A mapped module cannot be deleted, so an answer of "nothing is holding them"
; obtained from file handles is what allows the update to write a new payload on
; top of files it could not remove. Waiting on a real handle has no such gap,
; needs no process enumeration at all, and is unaffected by a process the holder
; probe cannot inspect.
;
; Sets $LauncherExited to 0 when no usable /PID= was given, which is the older
; client that does not pass one; the caller then has no authority available and
; falls back to the holder probe.
;
; A launcher that survives the first grace is asked to close by the probe, and the
; probe also answers whether there was a window a close could reach. A client hides
; its window the moment its own close is accepted, so a launcher without one has
; already answered its own prompts and is stuck in shutdown: no dialog can reach it
; there, and the wait ends it after one more grace instead of asking the user a
; question whose "keep waiting" answer can never change anything.
Function WaitForLauncherToExit
    StrCpy $LauncherExited 0
    StrCpy $WaitWindowless 0
    !insertmacro CockLog "wait-launcher: pid='$WaitPid'"
    ${If} $WaitPid == ""
        !insertmacro CockLog "wait-launcher: no pid given, falling back to name check"
        Return
    ${EndIf}
    ; System.dll maps its output registers so that .rN writes $N and .RN writes
    ; $RN. The caller reads $R0/$R1, so the uppercase .RN form is required; the
    ; original .r0/.r1 wrote $0/$1 and left $R1 empty, which made the "did not
    ; exit" test below compare "" against "0" and abort on every /PID= upgrade.
    ; Reopened on every round below, so a launcher that exited in the meantime answers
    ; "already exited" instead of being waited on again.
    ;
    ; PROCESS_TERMINATE rides along with PROCESS_SYNCHRONIZE so that the "end it"
    ; answer below acts on this very handle: the pid looked up again after a dialog
    ; that can stay open as long as the user takes would name whatever process holds
    ; that pid by then, and a handle cannot be handed to another process.
    ;
    ; When those rights are refused - a launcher started by another account, a
    ; protected process - the wait still has to happen, so the ask is repeated with
    ; synchronize access alone and $WaitCanEnd records that ending is not on offer.
    ; Reporting "already exited" instead would be the worse answer: the payload would
    ; be written over a live process, which is what this wait exists to prevent.
    ;
    ; Ten seconds, not the full grace: a client that started an update exits in well
    ; under a second, so at ten seconds it is either flushing a card database or - far
    ; more often - sitting on its own "quit anyway?" confirmation with nobody looking at
    ; it. Silence for a minute is what makes that read as a hung installer. The full
    ; grace is spent after the first question, so a user who answers Cockatrice's
    ; confirmation is not asked again while they do it.
    StrCpy $WaitSlice 10000
    wait_launcher_open:
    ClearErrors
    StrCpy $WaitCanEnd 1
    System::Call 'kernel32::OpenProcess(i 0x00100001, i 0, i $WaitPid) p.R0'
    ; OpenProcess returns a null handle for a process that is already gone. The
    ; System plugin does not raise the NSIS error flag from a called function's
    ; return value, so ${Errors} alone never saw that case and the null handle was
    ; passed straight to WaitForSingleObject. Fold a plugin failure into the same
    ; null-handle case instead of reading a handle that was never written.
    ${If} ${Errors}
        StrCpy $R0 0
    ${EndIf}
    ${If} $R0 == 0
        StrCpy $WaitCanEnd 0
        ClearErrors
        System::Call 'kernel32::OpenProcess(i 0x00100000, i 0, i $WaitPid) p.R0'
        ${If} ${Errors}
            StrCpy $R0 0
        ${EndIf}
    ${EndIf}
    ${If} $R0 == 0
    ; The launcher is gone - OpenProcess cannot open a dead process - which is
    ; exactly the state the wait was for. A handle refused for want of the terminate
    ; right is not this case; the synchronize-only ask above covers it.
        !insertmacro CockLog "wait-launcher: already exited"
        DetailPrint "The application that started this installer has already exited."
        StrCpy $LauncherExited 1
        Return
    ${EndIf}
    DetailPrint "Waiting for the application that started this installer to exit ..."
    ; WAIT_OBJECT_0 is 0; WAIT_TIMEOUT is 258 and a failed wait reports 0xFFFFFFFF.
    System::Call 'kernel32::WaitForSingleObject(p R0, i $WaitSlice) i.R1'
    ${If} $R1 != 0
    ; $R0 is deliberately still open: it is the process that timed out, and the answer
    ; below has to act on that process and not on whatever $WaitPid names by then.
        !insertmacro CockLog "wait-launcher: did not exit within $WaitSlice ms (wait=$R1, canEnd=$WaitCanEnd)"
        DetailPrint "The application that started this installer did not exit in time."
        ; Ask the launcher itself before asking the user. A client that has already hidden
        ; its window has answered its own prompts and is stuck in shutdown: there is nothing
        ; left a dialog could reach, and the "ask it to close again" answer the dialog used
        ; to offer could only repeat itself until the user cancelled.
        StrCpy $GuardDir $INSTDIR
        Call CloseLauncher
        ; $R3 = what the probe could do (see CloseLauncher), $R4 = its line, already logged.
        ; A process that answered the close during the probe's own wait is gone by now.
        System::Call 'kernel32::WaitForSingleObject(p R0, i 0) i.R1'
        ${If} $R1 == 0
            System::Call 'kernel32::CloseHandle(p R0)'
            StrCpy $R0 0
            Goto wait_launcher_open
        ${EndIf}
        ${If} $R3 == "4"
            ; Nothing a user could answer: the window is gone, so the launcher is past its
            ; own prompts and stuck in shutdown. It cannot be asked, and it is not going to
            ; ask the user anything either, so the dialog below would be unanswerable.
            ${If} $WaitCanEnd == 1
                ${If} $WaitWindowless == 0
                    ; Hiding the window is what the client does when its close is accepted,
                    ; so what remains is the shutdown it already started: one full grace for
                    ; that, without a question nobody could act on.
                    StrCpy $WaitWindowless 1
                    StrCpy $WaitSlice 60000
                    !insertmacro CockLog "wait-launcher: no window to ask, giving it one more minute"
                    System::Call 'kernel32::CloseHandle(p R0)'
                    StrCpy $R0 0
                    Goto wait_launcher_open
                ${EndIf}
                ${If} $WaitWindowless == 1
                    ; It did not use that minute either. Ending it is what the dialog's Yes
                    ; answer would have done, and there is no question left to put to the
                    ; user: the instance lost its window before this round started.
                    StrCpy $WaitWindowless 2
                    !insertmacro CockLog "wait-launcher: still no window to ask; ending the launcher"
                    DetailPrint "The application that started this installer could not be asked to close; ending it."
                    System::Call 'kernel32::TerminateProcess(p R0, i 1) i.R2'
                    !insertmacro CockLog "wait-launcher: ending the launcher returned $R2"
                    System::Call 'kernel32::CloseHandle(p R0)'
                    StrCpy $R0 0
                    Goto wait_launcher_open
                ${EndIf}
            ${EndIf}
        ${EndIf}
        ; A window exists, or the launcher cannot be ended from here: the three answers
        ; below. "Carry on anyway" is not one of them - a live process has its image and its
        ; Qt DLLs mapped, so writing the payload over them is the broken install this wait
        ; exists to prevent. Yes ends the launcher, Cancel cancels, and No keeps waiting:
        ; the close and the bring-forward just happened above.
        MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON2 \
            "Cockatrice has not finished closing, so the installation is paused rather than replace files it is still using.$\r$\n$\r$\nIt may be waiting for you to answer its own question about an unsaved deck, an open game or a card database update.$\r$\n$\r$\nChoose Yes to end it now - an unsaved deck, a running game or an unwritten setting is lost.$\r$\n$\r$\nChoose No to keep waiting, or Cancel to cancel the installation." \
            IDYES wait_launcher_end IDNO wait_launcher_retry
        !insertmacro CockLog "wait-launcher: cancelled by user"
        StrCpy $AbortReason \
            "The application that started this update has not exited yet, so the installation was cancelled rather than replace files it is still using.$\r$\n$\r$\nPlease close Cockatrice and run the installer again."
        Call AbortInstall
        ; Only reached through Yes or No: AbortInstall does not return.
        wait_launcher_end:
        ; End it through the handle waited on above, and through nothing else: no pid is
        ; looked up again, so a process that inherited the number in the meantime - or a
        ; /PID= that was not the launcher to begin with - cannot be hit by this.
        !insertmacro CockLog "wait-launcher: ending the launcher on request"
        ${If} $WaitCanEnd == 0
            !insertmacro CockLog "wait-launcher: the launcher could not be opened to end it, asking it to close instead"
            Goto wait_launcher_retry
        ${EndIf}
        System::Call 'kernel32::TerminateProcess(p R0, i 1) i.R3'
        System::Call 'kernel32::CloseHandle(p R0)'
        StrCpy $R0 0
        !insertmacro CockLog "wait-launcher: ending the launcher returned $R3"
        Goto wait_launcher_open
        wait_launcher_retry:
        System::Call 'kernel32::CloseHandle(p R0)'
        StrCpy $R0 0
        ; The question has been asked and answered once; from here the wait is the full
        ; grace again, so a slow flush or a confirmation being answered does not turn
        ; into a stream of prompts. The close itself and the bring-forward already
        ; happened just before the dialog, so this only waits. The windowless grace starts
        ; over as well: "keep waiting" must not be answered by ending the process a minute
        ; later without asking again.
        StrCpy $WaitWindowless 0
        StrCpy $WaitSlice 60000
        Goto wait_launcher_open
    ${EndIf}
    System::Call 'kernel32::CloseHandle(p R0)'
    StrCpy $R0 0
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

; Uninstaller copy of the guard (the uninstaller gets its own function set compiled in, it
; cannot call the installer functions). ExtractHolderProbe and RunHolderProbe come from
; CockHolderProbeFunctions above; only the policy differs.
Function un.EnsureAppsNotRunning
    ; guards the install directory; the uninstaller only ever has to check this one
    StrCpy $GuardDir $INSTDIR
    Call un.GuardAppsNotRunning
FunctionEnd

Function un.GuardAppsNotRunning
    StrCpy $GuardCloseRounds 0
    StrCpy $GuardRecheckCount 0
    StrCpy $GuardLastHolders ""
    un_guard_check:
    !insertmacro CockLogUn "uninst-guard: checking $GuardDir"
    Call un.RunHolderProbe
    ${If} $GuardExit == "0"
        !insertmacro CockLogUn "uninst-guard: nothing is holding the payload"
        Return
    ${EndIf}
    ${If} $GuardExit == "1"
        !insertmacro CockLogUn "uninst-guard: still in use by $GuardOutput"
        ; Same convenience as in the installer: ask every instance to close, repeat only
        ; while the set changed, then report what the close could not reach.
        ${If} $GuardCloseRounds < 2
        ${AndIf} $GuardOutput != $GuardLastHolders
            StrCpy $GuardLastHolders $GuardOutput
            IntOp $GuardCloseRounds $GuardCloseRounds + 1
            !insertmacro CockLogUn "uninst-guard: asking the payload programs to close (round $GuardCloseRounds)"
            Call un.CloseHolders
            Goto un_guard_check
        ${EndIf}
        DetailPrint "The install directory is in use by: $GuardOutput"
        ; Same question and the same three answers as the installer's guard: force close,
        ; close by hand and check again, or abort. CockEndRecheckPrompt words it, so the two
        ; copies promise the same thing.
        !insertmacro CockEndRecheckPrompt "Cockatrice, Oracle or Servatrice is still using files of the installation:" $GuardOutput "uninstallation"
        MessageBox MB_YESNOCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON2 "$GuardPrompt" IDYES un_guard_end IDNO un_guard_recheck
        !insertmacro CockLogUn "uninst-guard: cancelled by user after $GuardRecheckCount recheck(s)"
        ; The installer's Cancel lands in AbortInstall, which explains what to do next. The
        ; uninstaller has no such dialog, so say the same thing here: a bare Abort would
        ; close the window and leave the user with no reason and no next step.
        MessageBox MB_OK|MB_ICONEXCLAMATION \
            "The uninstallation was cancelled because Cockatrice, Oracle or Servatrice is still running and holding files of the installation.$\r$\n$\r$\n${AFTER_CLOSING_GUIDANCE}$\r$\n$\r$\nClose them and run the uninstaller again."
        SetErrorLevel 1
        Abort
        un_guard_recheck:
        IntOp $GuardRecheckCount $GuardRecheckCount + 1
        !insertmacro CockLogUn "uninst-guard: user closed programs by hand, checking again (recheck $GuardRecheckCount)"
        StrCpy $GuardCloseRounds 0
        StrCpy $GuardLastHolders ""
        Goto un_guard_check
        ; Only reached through Yes: Abort does not return.
        un_guard_end:
        !insertmacro CockLogUn "uninst-guard: ending the payload programs on request, then checking again"
        Call un.EndHolders
        ; One measurement and a stop, not another question: what survived a terminate is a
        ; program this uninstaller may not open or does not own, and ending it again cannot
        ; change that.
        Call un.RunHolderProbe
        ${If} $GuardExit == "0"
            !insertmacro CockLogUn "uninst-guard: nothing is holding the payload"
            Return
        ${EndIf}
        !insertmacro CockLogUn "uninst-guard: still in use after ending them ($GuardExit): $GuardOutput"
        SetErrorLevel 1
        Abort
    ${EndIf}
    ; Same reasoning as in the installer: without an answer from the probe we cannot tell
    ; whether a running instance still holds file locks, and refusing outright would leave
    ; the user with an uninstaller that never completes. Same three answers too, so an
    ; uninstall that hits a Restart Manager that was busy, or a payload program the user has
    ; not closed yet, can simply be retried.
    !insertmacro CockLogUn "uninst-guard: the holder probe could not answer ($GuardExit): $GuardOutput"
    DetailPrint "Could not confirm whether Cockatrice is still running: $GuardOutput"
    MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION|MB_DEFBUTTON1 \
        "This uninstaller could not confirm whether Cockatrice, Oracle or Servatrice is still running.$\r$\n$\r$\nThe check reported: $GuardOutput$\r$\n$\r$\nClose them, then choose Retry to check again. Choose Ignore to uninstall anyway, or Abort to cancel the uninstallation." \
        IDRETRY un_guard_check IDABORT un_guard_unknown_abort
    !insertmacro CockLogUn "uninst-guard: continuing without an answer from the probe"
    Return
    un_guard_unknown_abort:
    !insertmacro CockLogUn "uninst-guard: cancelled after the probe could not answer"
    SetErrorLevel 1
    Abort
FunctionEnd
