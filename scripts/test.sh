#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
MODE=debug
FILTER=""
CLEAN=0
JOBS=2

usage() {
  cat <<'USAGE'
Usage: ./scripts/test.sh [--release | --asan] [--filter PATTERN] [--clean] [--jobs N]

  (no options)     Build and run all tests in Debug mode
  --release        Build and run all tests in Release mode
  --asan           Build and run all tests with ASan + UBSan (GCC/Clang)
  --filter PATTERN Only build and run tests whose CTest names match PATTERN
  --clean          Remove only the selected mode's build directory first
  --jobs N         Parallel build jobs (default: 2)

Examples:
  ./scripts/test.sh
  ./scripts/test.sh --filter token
  ./scripts/test.sh --release --jobs 4
  ./scripts/test.sh --asan --clean
USAGE
}

while (($#)); do
  case "$1" in
    --release|--asan)
      if [[ "$MODE" != debug ]]; then echo 'Only one build mode may be selected' >&2; exit 2; fi
      MODE="${1#--}"; shift ;;
    --filter)
      if (($# < 2)) || [[ -z "$2" ]]; then echo '--filter requires a pattern' >&2; exit 2; fi
      FILTER="$2"; shift 2 ;;
    --clean) CLEAN=1; shift ;;
    --jobs)
      if (($# < 2)) || [[ ! "$2" =~ ^[1-9][0-9]*$ ]]; then echo '--jobs requires a positive integer' >&2; exit 2; fi
      JOBS="$2"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

BUILD_DIR="$ROOT/build/$MODE"
if ((CLEAN)); then rm -rf -- "$BUILD_DIR"; fi

args=(-S "$ROOT" -B "$BUILD_DIR" -DBUILD_TESTING=ON)
case "$MODE" in
  debug) args+=(-DCMAKE_BUILD_TYPE=Debug) ;;
  release) args+=(-DCMAKE_BUILD_TYPE=Release) ;;
  asan)
    args+=(-DCMAKE_BUILD_TYPE=Debug
      '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer'
      '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined') ;;
esac
cmake "${args[@]}"

if [[ -z "$FILTER" ]]; then
  cmake --build "$BUILD_DIR" --target upe_tests --parallel "$JOBS"
  ctest --test-dir "$BUILD_DIR" --output-on-failure
else
  # The focused-test convention is: CTest 'token.trivia' -> CMake 'test_token_trivia'.
  # Enumerate CTest registrations without requiring their executables to exist yet.
  mapfile -t names < <(ctest --test-dir "$BUILD_DIR" -N -R "$FILTER" 2>/dev/null \
    | sed -nE 's/^[[:space:]]*Test[[:space:]]*#[0-9]+: (.*)$/\1/p')
  if ((${#names[@]} == 0)); then
    echo "No CTest tests matched filter: $FILTER" >&2
    exit 1
  fi
  targets=()
  for name in "${names[@]}"; do
    target="test_${name//./_}"
    targets+=("$target")
  done
  cmake --build "$BUILD_DIR" --target "${targets[@]}" --parallel "$JOBS"
  ctest --test-dir "$BUILD_DIR" -R "$FILTER" --output-on-failure
fi
