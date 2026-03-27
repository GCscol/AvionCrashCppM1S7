$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir

function Invoke-Checked {
    param(
        [string]$Exe,
        [string[]]$Arguments
    )
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed ($LASTEXITCODE): $Exe $($Arguments -join ' ')"
    }
}

$clangUml = 'C:\Program Files\clang-uml\bin\clang-uml.exe'
if (-not (Test-Path $clangUml)) {
    throw "clang-uml executable not found at: $clangUml"
}

$flags = @(
    '--target=i686-w64-windows-gnu',
    '-isystemc:/mingw/lib/gcc/mingw32/6.3.0/include/c++',
    '-isystemc:/mingw/lib/gcc/mingw32/6.3.0/include/c++/mingw32',
    '-isystemc:/mingw/lib/gcc/mingw32/6.3.0/include/c++/backward',
    '-isystemc:/mingw/lib/gcc/mingw32/6.3.0/include',
    '-isystemc:/mingw/include',
    '-isystemc:/mingw/lib/gcc/mingw32/6.3.0/include-fixed'
)

$clangConfig = Join-Path $scriptDir 'clang-uml.yml'
$commonArgs = @('-c', $clangConfig, '-n', 'avion_class_overview')

$plantArgs = $commonArgs + @('-g', 'plantuml')
foreach ($f in $flags) {
    $plantArgs += @('--add-compile-flag', $f)
}
Invoke-Checked -Exe $clangUml -Arguments $plantArgs

$mermaidArgs = $commonArgs + @('-g', 'mermaid')
foreach ($f in $flags) {
    $mermaidArgs += @('--add-compile-flag', $f)
}
Invoke-Checked -Exe $clangUml -Arguments $mermaidArgs

$plantumlJar = Join-Path $scriptDir 'plantuml.jar'
$pumlPath = Join-Path $repoRoot 'uml_diagrams/output/avion_class_overview.puml'
$pumlCleanPath = Join-Path $repoRoot 'uml_diagrams/output/avion_class_overview_clean.puml'
$javaExe = 'java'
$java17 = 'C:\Program Files\Microsoft\jdk-17.0.18.8-hotspot\bin\java.exe'
if (Test-Path $java17) {
    $javaExe = $java17
}

function Write-CleanPuml {
    param(
        [string]$InputPath,
        [string]$OutputPath
    )

    $lines = Get-Content -Path $InputPath
    $stdIds = New-Object System.Collections.Generic.HashSet[string]

    foreach ($line in $lines) {
        if ($line -match '^\s*(class|abstract)\s+"std::[^"]*"\s+as\s+(C_\d+)\s*$') {
            [void]$stdIds.Add($matches[2])
        }
    }

    $filtered = New-Object System.Collections.Generic.List[string]
    $skipAliasBlock = $false
    $skipAliasId = ''

    foreach ($raw in $lines) {
        $line = $raw

        if ($line -match '^\s*(class|abstract)\s+"std::[^"]*"\s+as\s+(C_\d+)\s*$') {
            $skipAliasBlock = $true
            $skipAliasId = $matches[2]
            continue
        }

        if ($skipAliasBlock) {
            if ($line -match "^\s*class\s+$([regex]::Escape($skipAliasId))\s*\{") {
                continue
            }
            if ($line -match '^\s*__\s*$') {
                continue
            }
            if ($line -match '^\s*\}\s*$') {
                $skipAliasBlock = $false
                $skipAliasId = ''
                continue
            }
            continue
        }

        $containsStdId = $false
        foreach ($id in $stdIds) {
            if ($line -match [regex]::Escape($id)) {
                $containsStdId = $true
                break
            }
        }

        if ($containsStdId) {
            continue
        }

        $line = $line -replace 'std::', ''
        [void]$filtered.Add($line)
    }

    Set-Content -Path $OutputPath -Value $filtered -Encoding UTF8
}

Write-CleanPuml -InputPath $pumlPath -OutputPath $pumlCleanPath

if (Test-Path $plantumlJar) {
    Push-Location $repoRoot
    try {
        $plantUmlJvmArgs = @(
            '-Djava.awt.headless=true',
            '-DPLANTUML_LIMIT_SIZE=32768',
            '-jar',
            $plantumlJar,
            '-tpng'
        )

        Invoke-Checked -Exe $javaExe -Arguments ($plantUmlJvmArgs + @($pumlPath))
        Invoke-Checked -Exe $javaExe -Arguments ($plantUmlJvmArgs + @($pumlCleanPath))
    }
    finally {
        Pop-Location
    }
}
else {
    throw "plantuml.jar not found at: $plantumlJar"
}

Write-Host 'UML generation complete.'
Write-Host 'PlantUML: uml_diagrams/output/avion_class_overview.puml'
Write-Host 'PlantUML clean: uml_diagrams/output/avion_class_overview_clean.puml'
Write-Host 'Mermaid: uml_diagrams/output/avion_class_overview.mmd'
Write-Host 'PNG: uml_diagrams/output/avion_class_overview.png'
Write-Host 'PNG clean: uml_diagrams/output/avion_class_overview_clean.png'
