# GFxShaderMaker build script (Windows)
# Usage:
#   .\build.ps1              - build net8.0 (default)
#   .\build.ps1 -Publish     - publish single-file exe for win-x64
#   .\build.ps1 -Framework net35 - build legacy net35 (requires .NET FX 3.5 / MSBuild)

param(
    [switch]$Publish,
    [string]$Framework = "net8.0",
    [string]$Runtime = "win-x64",
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = $PSScriptRoot
$ProjectPath = Join-Path $ProjectRoot "GFxShaderMaker.csproj"

if (-not (Test-Path $ProjectPath)) {
    Write-Error "Project not found: $ProjectPath"
    exit 1
}

if ($Publish) {
    Write-Host "Publishing GFxShaderMaker -f $Framework -r $Runtime -c $Configuration" -ForegroundColor Cyan
    dotnet publish $ProjectPath `
        -p:ReleaseOnly=true `
        -f $Framework `
        -r $Runtime `
        -c $Configuration `
        --self-contained true `
        -p:PublishSingleFile=true `
        -p:IncludeNativeLibrariesForSelfExtract=true
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $outDir = Join-Path $ProjectRoot "bin" $Configuration $Framework $Runtime "publish"
    Write-Host "Output: $outDir" -ForegroundColor Green
} else {
    Write-Host "Building GFxShaderMaker -f $Framework -c $Configuration" -ForegroundColor Cyan
    dotnet build $ProjectPath -f $Framework -c $Configuration
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $outDir = Join-Path $ProjectRoot "bin" $Configuration $Framework
    Write-Host "Output: $outDir" -ForegroundColor Green
}
