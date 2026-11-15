; Instalador de La Pauta Mic Pro (VST3, Windows 64 bits)
; Se compila con Inno Setup: iscc installer\LaPautaMicPro.iss

#define MyAppName "La Pauta Mic Pro"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "La Pauta"
#define VstBundle "La Pauta Mic Pro.vst3"

[Setup]
AppId={{B6E2A3F4-5C1D-4E8A-9A7B-3D2F1C0E9A41}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=Output
OutputBaseFilename=La-Pauta-Mic-Pro-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallDisplayName={#MyAppName} (VST3)

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Files]
; Copia el bundle .vst3 completo a la carpeta estándar de VST3 de Windows
Source: "..\build\LaPautaMicPro_artefacts\Release\VST3\{#VstBundle}\*"; \
    DestDir: "{commoncf64}\VST3\{#VstBundle}"; \
    Flags: ignoreversion recursesubdirs createallsubdirs

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\{#VstBundle}"

[Messages]
spanish.FinishedLabel=La Pauta Mic Pro se instaló en C:\Program Files\Common Files\VST3.%nAbre tu DAW y vuelve a escanear los plugins VST3.
