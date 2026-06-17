param(
    [Parameter(Mandatory)]
    [string]$BuildOutputDir,

    [Parameter(Mandatory)]
    [string]$Version,

    [string]$OutputDir = "./packaging/output",
    [string]$PackagingDir = "./packaging"
)

$ErrorActionPreference = "Stop"

function Find-MakeAppx {
    $paths = @(
        "${env:ProgramFiles(x86)}\Windows Kits\10\bin\*\x64\MakeAppx.exe",
        "${env:ProgramFiles}\Windows Kits\10\bin\*\x64\MakeAppx.exe",
        "${env:ProgramFiles(x86)}\Microsoft SDKs\Windows\*\bin\*\MakeAppx.exe"
    )
    foreach ($pattern in $paths) {
        $match = Get-ChildItem $pattern -ErrorAction SilentlyContinue | Sort-Object -Property FullName -Descending | Select-Object -First 1
        if ($match) { return $match.FullName }
    }
    $which = Get-Command makeappx.exe -ErrorAction SilentlyContinue
    if ($which) { return $which.Source }
    throw "MakeAppx.exe not found. Install Windows SDK."
}

function New-MsixPackage {
    $makeappx = Find-MakeAppx
    Write-Host "Using MakeAppx: $makeappx"

    $staging = Join-Path $OutputDir "staging"
    if (Test-Path $staging) { Remove-Item -Recurse -Force $staging }
    New-Item -ItemType Directory -Force -Path $staging | Out-Null

    Write-Host "Copying application files..."
    Get-ChildItem $BuildOutputDir -Filter "*.exe" | Copy-Item -Destination $staging
    Get-ChildItem $BuildOutputDir -Filter "*.dll" | Copy-Item -Destination $staging

    Write-Host "Copying resources..."
    $resourcesSrc = Join-Path $BuildOutputDir "resources"
    if (Test-Path $resourcesSrc) {
        Copy-Item -Recurse -Path $resourcesSrc -Destination (Join-Path $staging "resources")
    }

    Write-Host "Copying app assets..."
    Get-ChildItem $BuildOutputDir -Include "*.png", "*.jpg" -Name | ForEach-Object {
        Copy-Item (Join-Path $BuildOutputDir $_) -Destination (Join-Path $staging $_)
    }

    $manifestSrc = Join-Path $PackagingDir "AppxManifest.xml"
    $manifestContent = Get-Content $manifestSrc -Raw
    $manifestContent = $manifestContent -replace '(?<=<Identity[^>]*Version=")[^"]+', "$Version.0"
    $manifestContent | Set-Content (Join-Path $staging "AppxManifest.xml") -NoNewline

    Write-Host "Copying Store assets..."
    $assetsSrc = Join-Path $PackagingDir "Assets"
    Copy-Item -Recurse -Path $assetsSrc -Destination (Join-Path $staging "Assets")

    $msixName = "Exder.TriDJsStems_${Version}_x64.msix"
    $msixPath = Join-Path $OutputDir $msixName

    Write-Host "Creating MSIX package: $msixPath"
    & $makeappx pack /d $staging /p $msixPath /l

    if ($LASTEXITCODE -ne 0) {
        throw "MakeAppx failed with exit code $LASTEXITCODE"
    }

    Remove-Item -Recurse -Force $staging
    Write-Host "MSIX package created: $msixPath"
    return $msixPath
}

try {
    $msix = New-MsixPackage
    Write-Host "SUCCESS: $msix"
} catch {
    Write-Host "ERROR: $_"
    exit 1
}
