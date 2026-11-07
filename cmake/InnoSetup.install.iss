; Sections added to the setup script CPack's Inno Setup generator writes.
;
; Included through CPACK_INNOSETUP_EXTRA_SCRIPTS, which prepends this file to the
; generated script. Repeating a section that the generator also writes is not an error: the
; compiler re-tests every [Header] against the section it is collecting and enumerates the
; lines of each occurrence (FoundSection in TSetupCompiler.EnumIniSection,
; Compiler.SetupCompiler.pas), so the [Files] and [InstallDelete] below merge with the ones
; built from the install() rules. Behaviour belongs in InnoSetup.code.iss.

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

; The installer-log sender behind the failure hook in InnoSetup.code.iss. dontcopy embeds
; the file in the setup image without installing it, and ExtractTemporaryFile pulls it out
; into the temporary directory only when something went wrong - what the NSIS template did
; by writing the same helper into $PLUGINSDIR. The path arrives from CMake as a public ISCC
; macro defined only for a build that carries a Sentry DSN; without one there is nothing to
; send, so nothing is embedded and the hook compiles out.
#ifdef SentryHelperPath
[Files]
Source: "{#SentryHelperPath}"; Flags: dontcopy
#endif
