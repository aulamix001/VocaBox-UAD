$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$build = Join-Path $repo 'build-msvc'
# Run from Developer PowerShell for VS 2022 with the C++ desktop workload installed.
cmake -S $repo -B $build -G 'Visual Studio 17 2022' -A x64
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }
cmake --build $build --config Release --parallel 2
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
ctest --test-dir $build -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Tests failed' }
Write-Host "VST3: $build\VocaBox_artefacts\Release\VST3\VocaBox.vst3"
