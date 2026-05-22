; Inno Setup Script - TriDJs Stems Suite
; Generate .ico from logo.png if needed, or use the JUCE-generated icon

#define MyAppName "TriDJs Stems Suite"
#define MyAppVersion "1.1.0"
#define MyAppPublisher "TriDJs"
#define MyAppURL "https://www.tridjs.com.br"
#define MyAppExeName "TriDJs Stems.exe"
#define MyAppId "{{B8A3C4D1-9E2F-4A7B-8C6D-5E1F2A3B4C5D}"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=installer
OutputBaseFilename=TriDJs_Stems_Setup_v{#MyAppVersion}
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupIconFile=build\TriDJs_Separador_Stems_artefacts\JuceLibraryCode\icon.ico
LicenseFile=TERMS_OF_USE.md
PrivilegesRequired=admin
DisableProgramGroupPage=yes

[Languages]
Name: "portuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Criar atalho na &Área de Trabalho"; GroupDescription: "Ícones adicionais:"; Flags: checkedonce

[Files]
; Main executable
Source: "build\TriDJs_Separador_Stems_artefacts\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; LibTorch CPU runtime DLLs
Source: "build\TriDJs_Separador_Stems_artefacts\Release\c10.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\torch.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\torch_cpu.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\torch_global_deps.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\fbgemm.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\fbjni.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\pytorch_jni.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\asmjit.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\uv.dll"; DestDir: "{app}"; Flags: ignoreversion

; Intel OpenMP
Source: "build\TriDJs_Separador_Stems_artefacts\Release\libiomp5md.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\libiompstubs5md.dll"; DestDir: "{app}"; Flags: ignoreversion

; Intel MKL (CPU math acceleration)
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_avx.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_avx2.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_avx512.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_core.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_def.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_intel_thread.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_mc.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_mc3.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_msg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_pgi_thread.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_rt.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_sequential.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_tbb_thread.1.dll"; DestDir: "{app}"; Flags: ignoreversion

; Intel MKL VML (vector math dispatchers)
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_vml_avx.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_vml_avx2.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_vml_avx512.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_vml_cmpt.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_vml_def.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_vml_mc.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_vml_mc2.1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\mkl_vml_mc3.1.dll"; DestDir: "{app}"; Flags: ignoreversion

; Assets (images)
Source: "build\TriDJs_Separador_Stems_artefacts\Release\logo.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\banner.jpg"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\banner2.jpg"; DestDir: "{app}"; Flags: ignoreversion
Source: "build\TriDJs_Separador_Stems_artefacts\Release\splash2.png"; DestDir: "{app}"; Flags: ignoreversion

; Resources folder
Source: "build\TriDJs_Separador_Stems_artefacts\Release\resources\*"; DestDir: "{app}\resources"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: ""
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Executar {#MyAppName}"; Flags: nowait postinstall skipifsilent

; Pre-install: detect and uninstall previous version automatically
[Code]
function InitializeSetup: Boolean;
var
  sUninstallPath: string;
  iResult: Integer;
begin
  Result := True;

  if RegQueryStringValue(HKLM, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#MyAppId}_is1', 'UninstallString', sUninstallPath) or
     RegQueryStringValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#MyAppId}_is1', 'UninstallString', sUninstallPath) then
  begin
    if MsgBox('O TriDJs Stems Suite já está instalado.' + #13#10 +
              'A instalação anterior será removida antes de continuar.' + #13#10#13#10 +
              'Deseja continuar?',
              mbConfirmation, MB_YESNO) = IDYES then
    begin
      Exec(RemoveQuotes(sUninstallPath), '/SILENT', '', SW_HIDE, ewWaitUntilTerminated, iResult);
    end
    else
      Result := False;
  end;
end;
