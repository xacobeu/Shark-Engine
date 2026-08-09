set windows-shell := ["powershell", "-NoProfile", "-Command"]
set shell := ["sh", "-c"]

default_target := "desktop"

# Build target: `just build` or `just build web`
build target=default_target: (update-clangd target)
    cmake --workflow --preset {{target}}

# Configure clangd compilation database pointer
update-clangd target=default_target:
    @just _clangd-{{target}}

[private]
_clangd-desktop:
    @powershell -NoProfile -Command "Set-Content -Path .clangd -Value 'CompilationDatabase: build/desktop-dev`n`nCompileFlags:`n  Add:`n    - -Wall`n    - -Wextra`n    - -Wpedantic`n    - -Wshadow`n    - -Wconversion`n    - -Wsign-conversion`n    - -Wold-style-cast`n    - -Wnon-virtual-dtor`n    - -Wunused`n    - -Wnull-dereference`n    - -Wdouble-promotion`n    - -Wextra-semi`n  Remove:`n    - /W*`n    - /w*`n    - /external:*' -Encoding utf8"

[private]
_clangd-web:
    @powershell -NoProfile -Command "Set-Content -Path .clangd -Value 'CompilationDatabase: build/web-release`n`nCompileFlags:`n  Add:`n    - -Wall`n    - -Wextra`n    - -Wpedantic`n    - -Wshadow`n    - -Wconversion`n    - -Wsign-conversion`n    - -Wold-style-cast`n    - -Wnon-virtual-dtor`n    - -Wunused`n    - -Wnull-dereference`n    - -Wdouble-promotion`n    - -Wextra-semi`n  Remove:`n    - /W*`n    - /w*`n    - /external:*' -Encoding utf8"

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