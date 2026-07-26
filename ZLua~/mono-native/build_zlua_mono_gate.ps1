# Build zlua_mono_gate.dll (x64) into Packages/.../Plugins/x64/
$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$outDir = (Resolve-Path (Join-Path $here "..\..\Plugins\x64")).Path
$outDll = Join-Path $outDir "zlua_mono_gate.dll"
$stageDll = Join-Path $here "zlua_mono_gate_build.dll"

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $vswhere)) {
    throw "vswhere.exe not found"
}

$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) {
    throw "MSVC VC tools not found"
}

$aux = Join-Path $vs "VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $aux)) {
    throw "vcvars64.bat not found: $aux"
}

# Stage next to sources first — writing /Fe straight into Packages/Plugins can silently fail to update.
$cmd = @"
call "$aux"
cd /d "$here"
cl /nologo /O2 /LD /MD /Fe"$stageDll" zlua_mono_gate.c
if errorlevel 1 exit /b 1
"@

cmd /c $cmd
if ($LASTEXITCODE -ne 0) {
    throw "cl failed with exit $LASTEXITCODE"
}

if (-not (Test-Path $stageDll)) {
    throw "cl did not produce $stageDll"
}

Copy-Item -Force $stageDll $outDll
Remove-Item -Force $stageDll,
    (Join-Path $here "zlua_mono_gate.obj"),
    (Join-Path $here "zlua_mono_gate_build.exp"),
    (Join-Path $here "zlua_mono_gate_build.lib"),
    (Join-Path $here "zlua_mono_gate.exp"),
    (Join-Path $here "zlua_mono_gate.lib") -ErrorAction SilentlyContinue

$item = Get-Item $outDll
Write-Host "Built and replaced: $outDll ($($item.Length) bytes, $($item.LastWriteTime))"
