#!/bin/bash

set -euo pipefail

usage() {
  cat <<'EOF'
Usage: package-windows-portable.sh [options]

Build iptux on MSYS2/CLANG64, stage the install tree, copy required runtime
DLLs/GTK files, and optionally produce a zip archive.

Options:
  --build-dir DIR    Meson build directory (default: ./build)
  --output-dir DIR   Portable package directory (default: ./dist/iptux-portable)
  --skip-build       Reuse the existing build directory without compiling
  --no-archive       Do not create a zip archive next to the output directory
  -h, --help         Show this help
EOF
}

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build"
OUTPUT_DIR="${REPO_ROOT}/dist/iptux-portable"
DIST_DIR="${REPO_ROOT}/dist"
STAGE_DIR="${DIST_DIR}/.portable-stage"
SKIP_BUILD=0
CREATE_ARCHIVE=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --output-dir)
      OUTPUT_DIR="$2"
      shift 2
      ;;
    --skip-build)
      SKIP_BUILD=1
      shift
      ;;
    --no-archive)
      CREATE_ARCHIVE=0
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ -z "${MSYSTEM:-}" || -z "${MINGW_PREFIX:-}" ]]; then
  echo "This script must be run inside an MSYS2 MinGW shell." >&2
  exit 1
fi

normalize_path() {
  case "$1" in
    /*|[A-Za-z]:/*|[A-Za-z]:\\*)
      printf '%s\n' "$1"
      ;;
    *)
      printf '%s/%s\n' "${REPO_ROOT}" "$1"
      ;;
  esac
}

BUILD_DIR="$(normalize_path "${BUILD_DIR}")"
OUTPUT_DIR="$(normalize_path "${OUTPUT_DIR}")"

require_tool() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Required tool not found: $1" >&2
    exit 1
  fi
}

require_tool meson
require_tool cp
require_tool find

copy_file() {
  local src="$1"
  local dst="$2"
  if [[ -f "$src" ]]; then
    mkdir -p "$(dirname -- "$dst")"
    cp -f "$src" "$dst"
  fi
}

copy_tree() {
  local src="$1"
  local dst="$2"
  if [[ -d "$src" ]]; then
    mkdir -p "$dst"
    cp -R "$src"/. "$dst"/
  fi
}

parse_dependencies() {
  sed -n \
    -e 's#.*=> \(/[^[:space:]]*\) (0x[[:xdigit:]]\+).*#\1#p' \
    -e 's#^[[:space:]]*\(/[^[:space:]]*\) (0x[[:xdigit:]]\+).*#\1#p'
}

dependency_report() {
  local target="$1"
  if command -v ntldd >/dev/null 2>&1; then
    ntldd -R "$target" 2>/dev/null || true
  else
    ldd "$target" 2>/dev/null || true
  fi
}

copy_runtime_dlls() {
  local package_dir="$1"
  local mingw_bin
  local -A seen_targets=()
  local -A copied_dlls=()
  local queue=()
  local target
  local dep
  local dep_unix

  mingw_bin="$(cd -- "${MINGW_PREFIX}/bin" && pwd)"

  while IFS= read -r target; do
    queue+=("$target")
  done < <(find "$package_dir" -type f \( -name '*.exe' -o -name '*.dll' \) | sort)

  while [[ ${#queue[@]} -gt 0 ]]; do
    target="${queue[0]}"
    queue=("${queue[@]:1}")
    if [[ -n "${seen_targets[$target]:-}" ]]; then
      continue
    fi
    seen_targets["$target"]=1

    while IFS= read -r dep; do
      [[ -n "$dep" ]] || continue
      dep_unix="$(cd -- "$(dirname -- "$dep")" && pwd)/$(basename -- "$dep")"
      case "$dep_unix" in
        "${mingw_bin}"/*.dll)
          if [[ -z "${copied_dlls[$dep_unix]:-}" ]]; then
            cp -f "$dep_unix" "${package_dir}/bin/"
            copied_dlls["$dep_unix"]=1
            queue+=("${package_dir}/bin/$(basename -- "$dep_unix")")
          fi
          ;;
      esac
    done < <(dependency_report "$target" | parse_dependencies | sort -u)
  done
}

generate_launcher() {
  local package_dir="$1"
  cat > "${package_dir}/iptux-portable.cmd" <<'EOF'
@echo off
setlocal EnableDelayedExpansion
set "APP_ROOT=%~dp0"
if "%APP_ROOT:~-1%"=="\" set "APP_ROOT=%APP_ROOT:~0,-1%"
set "PATH=%APP_ROOT%\bin;%APP_ROOT%\lib;%PATH%"
set "XDG_DATA_DIRS=%APP_ROOT%\share;%XDG_DATA_DIRS%"
set "GSETTINGS_SCHEMA_DIR=%APP_ROOT%\share\glib-2.0\schemas"
set "GTK_DATA_PREFIX=%APP_ROOT%"
set "GTK_EXE_PREFIX=%APP_ROOT%"
set "GTK_PATH=%APP_ROOT%"
set "GDK_PIXBUF_MODULEDIR=%APP_ROOT%\lib\gdk-pixbuf-2.0\2.10.0\loaders"
set "GDK_PIXBUF_MODULE_FILE=%APP_ROOT%\lib\gdk-pixbuf-2.0\2.10.0\loaders.cache"
if exist "%APP_ROOT%\bin\gdk-pixbuf-query-loaders.exe" if exist "%GDK_PIXBUF_MODULEDIR%" (
  set "GDK_PIXBUF_ARGS="
  for %%F in ("%GDK_PIXBUF_MODULEDIR%\*.dll") do (
    set "GDK_PIXBUF_ARGS=!GDK_PIXBUF_ARGS! "%%~fF""
  )
  if defined GDK_PIXBUF_ARGS (
    "%APP_ROOT%\bin\gdk-pixbuf-query-loaders.exe" !GDK_PIXBUF_ARGS! > "%GDK_PIXBUF_MODULE_FILE%"
  )
)
"%APP_ROOT%\bin\iptux.exe" %*
endlocal
EOF
}

prepare_build() {
  if [[ ! -f "${BUILD_DIR}/build.ninja" ]]; then
    meson setup "${BUILD_DIR}" "${REPO_ROOT}"
  fi

  if [[ "${SKIP_BUILD}" -eq 0 ]]; then
    meson compile -C "${BUILD_DIR}"
  fi
}

stage_install() {
  local exe_path
  local install_root

  rm -rf "${STAGE_DIR}" "${OUTPUT_DIR}" "${OUTPUT_DIR}.zip"
  mkdir -p "${DIST_DIR}"

  meson install -C "${BUILD_DIR}" --destdir "${STAGE_DIR}"

  exe_path="$(find "${STAGE_DIR}" -path '*/bin/iptux.exe' -print -quit)"
  if [[ -z "${exe_path}" ]]; then
    echo "Failed to locate staged iptux.exe under ${STAGE_DIR}" >&2
    exit 1
  fi

  install_root="$(cd -- "$(dirname -- "${exe_path}")/.." && pwd)"
  mkdir -p "${OUTPUT_DIR}"
  cp -R "${install_root}"/. "${OUTPUT_DIR}"/
}

copy_runtime_files() {
  local package_dir="$1"
  local loader_dir="${package_dir}/lib/gdk-pixbuf-2.0/2.10.0/loaders"
  local loader_cache="${package_dir}/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache"

  copy_tree "${MINGW_PREFIX}/share/glib-2.0/schemas" "${package_dir}/share/glib-2.0/schemas"
  copy_tree "${MINGW_PREFIX}/share/icons/Adwaita" "${package_dir}/share/icons/Adwaita"
  copy_tree "${MINGW_PREFIX}/share/icons/hicolor" "${package_dir}/share/icons/hicolor"
  copy_tree "${MINGW_PREFIX}/share/themes/Default" "${package_dir}/share/themes/Default"
  copy_tree "${MINGW_PREFIX}/share/mime" "${package_dir}/share/mime"
  copy_tree "${MINGW_PREFIX}/lib/gdk-pixbuf-2.0/2.10.0/loaders" "$loader_dir"
  copy_file "${MINGW_PREFIX}/bin/gdk-pixbuf-query-loaders.exe" \
    "${package_dir}/bin/gdk-pixbuf-query-loaders.exe"
  copy_file "${MINGW_PREFIX}/bin/gdk-pixbuf-query-loaders-cache.exe" \
    "${package_dir}/bin/gdk-pixbuf-query-loaders-cache.exe"

  copy_runtime_dlls "$package_dir"

  if command -v glib-compile-schemas >/dev/null 2>&1 \
    && [[ -d "${package_dir}/share/glib-2.0/schemas" ]]; then
    glib-compile-schemas "${package_dir}/share/glib-2.0/schemas"
  fi

  if command -v gdk-pixbuf-query-loaders >/dev/null 2>&1 \
    && [[ -d "${loader_dir}" ]]; then
    local loaders=()
    shopt -s nullglob
    loaders=("${loader_dir}"/*.dll)
    shopt -u nullglob
    if [[ ${#loaders[@]} -gt 0 ]]; then
      gdk-pixbuf-query-loaders "${loaders[@]}" > "${loader_cache}"
    fi
  fi

  generate_launcher "$package_dir"
}

create_archive() {
  local package_dir="$1"
  local archive_path="${package_dir}.zip"
  local package_name
  package_name="$(basename -- "$package_dir")"

  if command -v 7z >/dev/null 2>&1; then
    (
      cd -- "$(dirname -- "$package_dir")"
      7z a -tzip "${archive_path}" "${package_name}" >/dev/null
    )
  else
    (
      cd -- "$(dirname -- "$package_dir")"
      tar -a -cf "${archive_path}" "${package_name}"
    )
  fi
}

prepare_build
stage_install
copy_runtime_files "${OUTPUT_DIR}"

if [[ "${CREATE_ARCHIVE}" -eq 1 ]]; then
  create_archive "${OUTPUT_DIR}"
fi

echo "Portable package directory: ${OUTPUT_DIR}"
if [[ "${CREATE_ARCHIVE}" -eq 1 ]]; then
  echo "Portable package archive: ${OUTPUT_DIR}.zip"
fi
