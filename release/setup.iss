#define MyAppName "arxybin."
#define MyAppVersion "1.0.0"
#define MyAppPublisher "nilotoo art."
#define MyAppURL "https://arxybin.com"

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={commonpf}\arxybin
DefaultGroupName=arxybin.
OutputDir=.\release
OutputBaseFilename=arxybin_v1.0.0_Setup
WizardImageFile=.\pictures\install_preview.bmp
WizardSmallImageFile=.\pictures\install_preview_small.bmp
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
DisableWelcomePage=no

[Types]
Name: "full"; Description: "Full installation (VST3 + Standalone)"
Name: "vst3"; Description: "VST3 plugin only"
Name: "standalone"; Description: "Standalone application only"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 Plugin"; Types: full vst3 custom; Flags: checkablealone
Name: "standalone"; Description: "Standalone Application"; Types: full standalone custom; Flags: checkablealone
Name: "presets"; Description: "Factory Presets (Documents\arxybin\Presets)"; Types: full vst3 standalone custom

[Files]
; VST3
Source: ".\release\arxybin_v1.0.0\arxybin..vst3\*"; DestDir: "{code:GetVST3Path}"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: vst3
; Standalone
Source: ".\release\arxybin_v1.0.0\arxybin..exe"; DestDir: "{code:GetStandalonePath}"; Flags: ignoreversion; Components: standalone
; Factory presets -> factory/
Source: ".\release\arxybin_v1.0.0\*.arxybin"; DestDir: "{userdocs}\arxybin\Presets\factory"; Flags: ignoreversion; Components: presets

[Icons]
Name: "{group}\arxybin. (Standalone)"; Filename: "{code:GetStandalonePath}\arxybin..exe"; Components: standalone
Name: "{group}\Uninstall arxybin."; Filename: "{uninstallexe}"

[Code]
var
  VST3PathPage: TInputDirWizardPage;
  StandalonePathPage: TInputDirWizardPage;

function GetVST3Path(Param: String): String;
begin
  Result := VST3PathPage.Values[0];
end;

function GetStandalonePath(Param: String): String;
begin
  Result := StandalonePathPage.Values[0];
end;

procedure InitializeWizard;
begin
  VST3PathPage := CreateInputDirPage(wpSelectComponents,
    'VST3 Plugin Installation Path',
    'Select where to install the VST3 plugin.',
    'The plugin will be placed in a subfolder "arxybin..vst3" under this path.' + #13#10 +
    'Default: C:\Program Files\Common Files\VST3\',
    False, '');
  VST3PathPage.Add('VST3 Path:');
  VST3PathPage.Values[0] := ExpandConstant('{commoncf}\VST3\arxybin..vst3');

  StandalonePathPage := CreateInputDirPage(wpSelectComponents,
    'Standalone Installation Path',
    'Select where to install the Standalone application.',
    'Default: C:\Program Files\arxybin\',
    False, '');
  StandalonePathPage.Add('Standalone Path:');
  StandalonePathPage.Values[0] := ExpandConstant('{commonpf}\arxybin');
end;
