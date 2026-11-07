$ErrorActionPreference='Stop'
dotnet publish "$PSScriptRoot\GasSetup\GasSetup.csproj" -c Release
$iscc=(Get-Command ISCC.exe -ErrorAction SilentlyContinue).Source
if (!$iscc) { throw 'Installa Inno Setup 6, poi riesegui questo script.' }
& $iscc "$PSScriptRoot\installer\Setup.iss"
