#!/bin/bash

declare -A requirements=(
    ["awk"]="gawk"
    ["bison"]="bison"
    ["ctags"]="universal-ctags"
    ["curl"]="curl"
    ["flex"]="flex"
    ["g++"]="g++"
    ["grep"]="grep"
    ["gzip"]="gzip"
    ["make"]="make"
    ["nasm"]="nasm"
    ["sed"]="sed"
    ["tar"]="tar"
    ["xz"]="xz-utils"
)

MISSING_PACKAGES=()

echo "[!] Checking dependencies..."

for tool in "${!requirements[@]}"; do
    if ! command -v "$tool" &> /dev/null; then
        echo "[!] $tool not found. Adding to queue: ${requirements[$tool]}"
        MISSING_PACKAGES+=("${requirements[$tool]}")
    else
        echo "[!] $tool already installed"
    fi
done

if [ ${#MISSING_PACKAGES[@]} -eq 0 ]; then
    echo "[!] All dependencies already installed!"
    exit 0
fi

echo "[!] Installing missing packages: ${MISSING_PACKAGES[*]}"
sudo apt update
sudo apt install -y "${MISSING_PACKAGES[@]}"

echo "[!] Done."
