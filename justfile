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
    cmake -E echo "CompilationDatabase: build/desktop-dev" > .clangd

[private]
_clangd-web:
    cmake -E echo "CompilationDatabase: build/web-release" > .clangd

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
_run-web:
    emrun --no-browser build/web-release/demo/SharkEngineDemo.html

# Clean build artifacts
clean:
    cmake -E rm -rf build compile_commands.json .clangd