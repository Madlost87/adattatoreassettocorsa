; Compilare su Windows con Inno Setup 6: ISCC.exe Setup.iss
#define AppName "SETUP GAS"
#define AppVersion "1.0.0"
[Setup]
AppName={#AppName}
AppVersion={#AppVersion}
DefaultDirName={autopf}\Setup Gas
DefaultGroupName=SETUP GAS
OutputDir=..\..\release
OutputBaseFilename=Setup
Compression=lzma2
SolidCompression=yes
[Files]
Source: "..\GasSetup\bin\Release\net8.0-windows\win-x64\publish\GasSetup.exe"; DestDir: "{app}"
[Icons]
Name: "{group}\SETUP GAS"; Filename: "{app}\GasSetup.exe"
[Run]
Filename: "{app}\GasSetup.exe"; Description: "Avvia SETUP GAS"; Flags: nowait postinstall skipifsilent
