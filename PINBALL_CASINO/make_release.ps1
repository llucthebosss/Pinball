# Builds the game in Release and creates the delivery zip in .\Entrega
# Usage (from PowerShell):  powershell -ExecutionPolicy Bypass -File .\make_release.ps1
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem

$root    = $PSScriptRoot
$proj    = Join-Path $root 'PINBALL\PINBALL'
# Name required by the submission rules: Lastname1Name1_Lastname2Name2_etc
$name    = 'LlucTornerCalvo_AndreaVascoPeir' + [char]0xF3 + '_AlanDelTio'
$outDir  = Join-Path $root 'Entrega'
$pkgDir  = Join-Path $outDir $name
$zipPath = Join-Path $outDir "$name.zip"

# 1) Release build
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
& $msbuild (Join-Path $proj 'Codi definitiu.sln') /p:Configuration=Release /p:Platform=Win32 /m /v:minimal /nologo
if ($LASTEXITCODE -ne 0) { throw 'Release build failed' }

# 2) Package folder
if (Test-Path $pkgDir) { Remove-Item $pkgDir -Recurse -Force }
New-Item -ItemType Directory -Force (Join-Path $pkgDir 'Assets') | Out-Null

Copy-Item (Join-Path $proj 'Build\PhysicsGame\Release\Win32\PhysicsGame.exe') (Join-Path $pkgDir 'PinballCasino.exe')

$vsPath = & $vswhere -latest -property installationPath
$redist = Get-ChildItem (Join-Path $vsPath 'VC\Redist\MSVC') -Directory |
          Where-Object Name -match '^\d' | Sort-Object Name | Select-Object -Last 1
$crt = Join-Path $redist.FullName 'x86\Microsoft.VC143.CRT'
if (-not (Test-Path $crt)) { $crt = Join-Path $proj 'Game' }
Copy-Item (Join-Path $crt 'msvcp140.dll'), (Join-Path $crt 'vcruntime140.dll') $pkgDir

# Only the assets the game actually loads
Copy-Item (Join-Path $proj 'Assets\Background.png'), (Join-Path $proj 'Assets\bonus.wav') (Join-Path $pkgDir 'Assets')
Copy-Item (Join-Path $proj 'README.md'), (Join-Path $proj 'LICENSE') $pkgDir

# 3) Zip (forward slashes so it opens fine everywhere)
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
$zip = [System.IO.Compression.ZipFile]::Open($zipPath, 'Create')
try {
    Get-ChildItem $pkgDir -Recurse -File | ForEach-Object {
        $rel = $name + '/' + $_.FullName.Substring($pkgDir.Length + 1).Replace('\', '/')
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $_.FullName, $rel, 'Optimal') | Out-Null
    }
} finally { $zip.Dispose() }

Write-Host "Done: $zipPath"
