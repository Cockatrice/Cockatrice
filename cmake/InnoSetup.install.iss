; Sections added to the setup script CPack's Inno Setup generator writes.
;
; Included through CPACK_INNOSETUP_EXTRA_SCRIPTS, which prepends this file to the
; generated script. Only sections the generator never emits may appear here: it writes
; [Files], [Dirs], [Icons], [Run] and [Registry] from the install() rules, and a second
; section of one of those is a compile error. Behaviour belongs in InnoSetup.code.iss.

[InstallDelete]
; The previous version's runtime, whatever it was called. Replacing a DLL in place is not
; enough: a module the payload no longer carries (Qt6Multimedia.dll while it was still
; shared, Qt6QuickWidgets.dll) would survive the install and then be loaded next to the new
; executable, which is the "procedure entry point ... could not be located" failure after
; an update. Removing every DLL and letting the payload put its own back is what the NSIS
; RetryDllPurge did with 250 lines of retry and diagnostics; with CloseApplications on,
; Restart Manager has already released the handles, so one pass is enough.
Type: files; Name: "{app}\*.dll"
; Plugins are loaded by name from this directory, so a stale one is just as fatal and is
; not covered by the payload's own file list.
Type: filesandordirs; Name: "{app}\Plugins"
; The NSIS uninstaller. Its Add/Remove Programs registration is deleted after a successful
; install (RemoveLegacyNSISRegistration in InnoSetup.code.iss); the binary itself has no
; business staying next to the new one.
Type: files; Name: "{app}\Uninstall.exe"
