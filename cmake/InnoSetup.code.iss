// Behaviour for the Windows installer written by CPack's Inno Setup generator.
//
// Included through CPACK_INNOSETUP_CODE_FILES, which places this file inside the generated
// script's code section. Everything here is something the generator cannot express as
// data: the migration off the NSIS installation, the portable marker, and the C++ runtime
// bootstrap. Comments use Pascal braces, so the install-directory constant is spelled out
// as ExpandConstant rather than written inside prose.

const
  // Written by every NSIS build up to and including 3.1.x: HKLM in the 64-bit view for a
  // 64-bit installer, and in the 32-bit view by the older 32-bit ones.
  LegacyUninstallKey = 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Cockatrice';

var
  PortablePage: TCustomPage;
  PortableCheck: TNewCheckBox;

// True when the given switch appears on the setup command line. Used for the legacy
// portable switch, which the in-app updater and the older installers pass.
function HasSwitch(const Name: String): Boolean;
var
  I: Integer;
begin
  Result := False;
  for I := 1 to ParamCount do
    if CompareText(ParamStr(I), Name) = 0 then
    begin
      Result := True;
      Exit;
    end;
end;

// The install directory recorded by the NSIS installer, or an empty string when this
// machine has no such installation. Read-only access to HKLM, so it works unelevated.
function ReadLegacyInstallDir(): String;
var
  Value: String;
begin
  Result := '';
  if RegQueryStringValue(HKLM64, LegacyUninstallKey, 'InstallLocation', Value) then
    Result := Value
  else if RegQueryStringValue(HKLM32, LegacyUninstallKey, 'InstallLocation', Value) then
    Result := Value;
end;

// Scripted constant behind the DefaultDirName directive. An NSIS installation keeps no
// Inno-style "previous app path", so without this an upgrade of a user who picked a custom
// folder in 3.1.x would land in the default folder and leave the old copy behind.
function GetDefaultInstallDir(Param: String): String;
var
  Legacy: String;
begin
  Legacy := ReadLegacyInstallDir();
  if (Legacy <> '') and DirExists(Legacy) then
  begin
    Result := Legacy;
    Log('Default install directory follows the NSIS installation: ' + Result);
  end
  else
    Result := ExpandConstant('{autopf}\Cockatrice');
end;

procedure InitializeWizard();
begin
  PortablePage := CreateCustomPage(wpSelectDir, 'Installation mode',
    'Cockatrice keeps settings, decks and downloaded pictures in your user profile by default. '
    + 'Portable mode keeps them in the install folder instead.');
  PortableCheck := TNewCheckBox.Create(PortablePage);
  PortableCheck.Parent := PortablePage.Surface;
  PortableCheck.Left := 0;
  PortableCheck.Top := 0;
  PortableCheck.Width := PortablePage.SurfaceWidth;
  PortableCheck.Height := ScaleY(32);
  PortableCheck.WordWrap := True;
  PortableCheck.Caption := 'Portable: keep settings and decks in the install folder';
  PortableCheck.Checked := HasSwitch('/PORTABLE');
end;

function PortableRequested(): Boolean;
begin
  Result := (Assigned(PortableCheck) and PortableCheck.Checked) or HasSwitch('/PORTABLE');
end;

// The client decides portable mode by the presence of this file next to the executable
// (cache_settings.cpp and logger.cpp), not by its contents.
procedure WritePortableMarker();
var
  Lines: TArrayOfString;
begin
  if not PortableRequested() then
    Exit;
  SetArrayLength(Lines, 1);
  Lines[0] := 'Portable install: settings, decks and pictures live beside the executable.';
  if not SaveStringsToFile(ExpandConstant('{app}\portable.dat'), Lines, False) then
    MsgBox('Could not write portable.dat into the install folder. Portable mode will not be'
      + ' active until it exists.', mbError, MB_OK);
end;

// Runs the Visual C++ redistributable the payload carries, when it carries one. The file
// name differs between the VS 2017 and VS 2022 layouts, so it is matched rather than named.
// A failure is logged and ignored: the redistributable is usually already present, and the
// application is what reports a real problem.
procedure InstallVCRedist();
var
  FindRec: TFindRec;
  ResultCode: Integer;
  Redist: String;
begin
  if not FindFirst(ExpandConstant('{app}\vc*redist*.exe'), FindRec) then
    Exit;
  try
    repeat
      if (FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY) = 0 then
      begin
        Redist := ExpandConstant('{app}\') + FindRec.Name;
        Log('Installing the C++ runtime: ' + Redist);
        if Exec(Redist, '/install /quiet /norestart', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
        begin
          Log('C++ runtime installer exited with ' + IntToStr(ResultCode));
          DeleteFile(Redist);
        end
        else
          Log('Could not start the C++ runtime installer: ' + Redist);
      end;
    until not FindNext(FindRec);
  finally
    FindClose(FindRec);
  end;
end;

// Removes the Add/Remove Programs entry the NSIS installer left behind. Without this the
// user sees two entries for Cockatrice and can still run the old uninstaller, whose
// blanket removal of the install directory is the data loss the 3.1.x backup pair was
// written to survive. Deleting the registration is what makes that uninstaller
// unreachable; the binary next to it is removed earlier by the install-delete section.
procedure RemoveLegacyNSISRegistration();
var
  Uninstaller: String;
begin
  if not (RegQueryStringValue(HKLM64, LegacyUninstallKey, 'UninstallString', Uninstaller)
    or RegQueryStringValue(HKLM32, LegacyUninstallKey, 'UninstallString', Uninstaller)) then
    Exit;
  RegDeleteKeyIncludingSubkeys(HKLM64, LegacyUninstallKey);
  RegDeleteKeyIncludingSubkeys(HKLM32, LegacyUninstallKey);
  Log('Removed the legacy NSIS uninstall registration: ' + Uninstaller);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    InstallVCRedist();
    WritePortableMarker();
    RemoveLegacyNSISRegistration();
  end;
end;

// The 3.1.x client launches the installer with two switches that belong to the NSIS build:
// /R (silent reinstall) and /PID= (the process to wait for). Recording them from the log is
// how the first test of this installer answers the one open question about the migration:
// whether Inno Setup reaches this point at all with them on the command line. If it does,
// the updater keeps working across the switch; if it does not, Setup rejects them before
// running any script and the updater needs a version gate instead.
function InitializeSetup(): Boolean;
var
  I: Integer;
  Param: String;
begin
  Log('Setup command line: ' + GetCmdTail());
  for I := 1 to ParamCount do
  begin
    Param := ParamStr(I);
    if (CompareText(Param, '/R') = 0) or (CompareText(Copy(Param, 1, 5), '/PID=') = 0) then
      Log('Legacy in-app updater switch: ' + Param);
  end;
  Result := True;
end;
