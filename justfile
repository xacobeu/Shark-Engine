set windows-shell := ["powershell", "-NoProfile", "-Command"]
set shell := ["sh", "-c"]

default_target := "desktop"

# Build target: `just build` or `just build web`
build target=default_target: (update-clangd target)
    cmake --workflow --preset {{target}}

# Configure clangd compilation database pointer
update-clangd target=default_target:
    @powershell -NoProfile -Command "Set-Content -Path .clangd -Value 'CompilationDatabase: build/{{ if target == 'web' { 'web-release' } else { 'desktop-dev' } }}' -Encoding utf8"
    -cmake -E copy_if_different build/{{ if target == 'web' { 'web-release' } else { 'desktop-dev' } }}/compile_commands.json compile_commands.json

# Run target: `just run` or `just run web`
run target=default_target: (build target)
    @just _run-{{target}}

[private]
[windows]
_run-desktop:
    cd build/desktop-dev/demo; .\SharkEngineDemo.exe

[private]
[unix]
_run-desktop:
    cd build/desktop-dev/demo && ./SharkEngineDemo

[private]
[windows]
_run-web:
    -try { emrun --no-browser build/web-release/demo/SharkEngineDemo.html } finally { Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*emrun*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue } }

[private]
[unix]
_run-web:
    -trap 'pkill -f emrun' INT TERM EXIT; emrun --no-browser build/web-release/demo/SharkEngineDemo.html

# Clean build artifacts
[windows]
clean:
    -Stop-Process -Name SharkEngineDemo -Force -ErrorAction SilentlyContinue
    -Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*emrun*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
    -Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -like "*http.server*" -and $_.Name -like "*python*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
    cmake -E rm -rf build compile_commands.json .clangd

[unix]
clean:
    -pkill -f SharkEngineDemo
    -pkill -f emrun
    -pkill -f "python.*http.server"
    cmake -E rm -rf build compile_commands.json .clangd