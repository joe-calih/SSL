#ifndef AppVersion
  #define AppVersion "2.0.1"
#endif

#define AppName "Loops Channel Strip"
#define AppPublisher "Loops"
#define AppURL "https://loops.co.ke/"
#define PluginBundleName "Channel Strip.vst3"

[Setup]
AppId={{1F7D61A3-F6C1-48C7-AC02-417778C57E81}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\Loops\Channel Strip
DefaultGroupName=Loops
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
OutputDir=..\artifacts
OutputBaseFilename=ChannelStrip-v{#AppVersion}-Windows-Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
LicenseFile=..\LICENSE
UninstallDisplayName={#AppName} {#AppVersion}
UninstallDisplayIcon={commoncf64}\VST3\{#PluginBundleName}\Contents\x86_64-win\Channel Strip.vst3

[Tasks]
Name: "visitwebsite"; Description: "Open Loops.co.ke when setup finishes"; GroupDescription: "After installation:"; Flags: checkedonce

[Files]
Source: "..\build\ChannelStrip_artefacts\Release\VST3\{#PluginBundleName}\*"; DestDir: "{commoncf64}\VST3\{#PluginBundleName}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "Licenses.txt"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Loops Channel Strip website"; Filename: "{#AppURL}"

[Run]
Filename: "{#AppURL}"; Description: "Visit Loops.co.ke"; Flags: postinstall shellexec skipifsilent; Tasks: visitwebsite

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#PluginBundleName}"
