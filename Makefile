.PHONY: default help build run clean format check-format tidy desktop web check-emsdk

TARGET := $(if $(filter web,$(MAKECMDGOALS)),web,desktop)

EMSDK ?= $(firstword $(wildcard $(HOME)/.emsdk /opt/emsdk))
export EMSDK

default: help
help:
	@echo "Usage:"
	@echo "  make build desktop  - Compile desktop target"
	@echo "  make build web      - Compile WASM target"
	@echo "  make run desktop    - Build and run desktop demo"
	@echo "  make run web        - Build and run WASM demo in browser"
	@echo "  make format         - Format all engine and example C++ source code"
	@echo "  make check-format   - Check formatting without modifying files (CI)"
	@echo "  make tidy           - Run Clang-Tidy static analysis on all source files"
	@echo "  make clean          - Delete build directory and terminate processes"

build:
ifeq ($(TARGET),web)
	@$(MAKE) --no-print-directory check-emsdk
	@if [ ! -f "build/web-release/build.ninja" ]; then cmake --preset web; fi
	cmake --build --preset web
else
	@if [ ! -f "build/desktop-dev/build.ninja" ]; then cmake --preset desktop; fi
	cmake --build --preset desktop
endif

run: build
ifeq ($(TARGET),web)
	emrun build/web-release/example/SharkEngineExample.html
else
	./build/desktop-dev/example/SharkEngineExample
	@rm -f imgui.ini compile_commands.json
endif

clean:
	@-pkill -f SharkEngineExample 2>/dev/null || true
	@-pkill -f emrun 2>/dev/null || true
	rm -rf build

format:
	@find src example -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +
	@echo "Code formatting complete."

check-format:
	@find src example -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format --dry-run --Werror {} +
	@echo "All files passed formatting check."

tidy:
	@if [ ! -f "build/desktop-dev/compile_commands.json" ]; then \
		echo "Generating compilation database for Clang-Tidy..."; \
		cmake --preset desktop > /dev/null; \
	fi
	@echo "Running Clang-Tidy analysis (read-only)..."
	@find src example -type f -name "*.cpp" -exec clang-tidy -p build/desktop-dev --quiet {} +

desktop web:
	@:

check-emsdk:
	@TOOLCHAIN="$${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"; \
	if [ -z "$$EMSDK" ] || [ ! -f "$$TOOLCHAIN" ]; then \
		echo ""; \
		echo "=================================================================="; \
		echo " [!] Emscripten SDK not activated in your environment!"; \
		echo ""; \
		echo " To finish setting up emsdk:"; \
		echo "   1. emsdk install latest"; \
		echo "   2. emsdk activate latest"; \
		echo "   3. Source the environment variables in your terminal:"; \
		echo "      - If in ~/.emsdk:   source ~/.emsdk/emsdk_env.sh"; \
		echo "      - If in /opt/emsdk: source /opt/emsdk/emsdk_env.sh"; \
		echo "=================================================================="; \
		echo ""; \
		exit 1; \
	fi
