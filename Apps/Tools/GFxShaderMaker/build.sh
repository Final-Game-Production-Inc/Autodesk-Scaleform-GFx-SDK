#!/usr/bin/env bash
# GFxShaderMaker build script (Linux/macOS)
# Usage:
#   ./build.sh              - build net8.0 (default)
#   ./build.sh --publish    - publish single-file executable for current OS
#   ./build.sh --framework net35  - not supported on Linux (net35 is Windows-only)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
PROJECT_PATH="$PROJECT_ROOT/GFxShaderMaker.csproj"
FRAMEWORK="${FRAMEWORK:-net8.0}"
CONFIGURATION="${CONFIGURATION:-Release}"

# Detect runtime if publishing
detect_runtime() {
    case "$(uname -s)" in
        Linux)   echo "linux-x64" ;;
        Darwin)  echo "osx-x64" ;;
        *)      echo "linux-x64" ;;
    esac
}

PUBLISH=false
while [[ $# -gt 0 ]]; do
    case "$1" in
        --publish|-p) PUBLISH=true; shift ;;
        --framework|-f) FRAMEWORK="$2"; shift 2 ;;
        --configuration|-c) CONFIGURATION="$2"; shift 2 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

if [[ ! -f "$PROJECT_PATH" ]]; then
    echo "Project not found: $PROJECT_PATH"
    exit 1
fi

if [[ "$PUBLISH" == true ]]; then
    RUNTIME=$(detect_runtime)
    echo "Publishing GFxShaderMaker -f $FRAMEWORK -r $RUNTIME -c $CONFIGURATION"
    dotnet publish "$PROJECT_PATH" \
        -p:ReleaseOnly=true \
        -f "$FRAMEWORK" \
        -r "$RUNTIME" \
        -c "$CONFIGURATION" \
        --self-contained true \
        -p:PublishSingleFile=true \
        -p:IncludeNativeLibrariesForSelfExtract=true
    OUT_DIR="$PROJECT_ROOT/bin/$CONFIGURATION/$FRAMEWORK/$RUNTIME/publish"
    echo "Output: $OUT_DIR"
else
    echo "Building GFxShaderMaker -f $FRAMEWORK -c $CONFIGURATION"
    dotnet build "$PROJECT_PATH" -f "$FRAMEWORK" -c "$CONFIGURATION"
    OUT_DIR="$PROJECT_ROOT/bin/$CONFIGURATION/$FRAMEWORK"
    echo "Output: $OUT_DIR"
fi
