#!/bin/bash

# ╔══════════════════════════════════════════════════════════════╗
# ║                  LightOS Dependency Util                     ║
# ║   Checks tool presence, versions, and installs missing       ║
# ╚══════════════════════════════════════════════════════════════╝

VERSIONED_TOOLS=(
    "gcc:12:13"
    "g++:12:13"
    "ld:2.36:2.46.0"
    "as:2.36:2.46.0"
    "nasm:2.14:"
    "make:4.0:"
    "bison:3.0:"
    "flex:2.6:"
    "meson:0.60:"
    "ninja:1.10:"
)

PLAIN_TOOLS=(
    "awk"
    "ctags"
    "curl"
    "grep"
    "gzip"
    "sed"
    "tar"
    "xz"
    "python3"
    "git"
)

MISSING_PACKAGES=()
WARN_PACKAGES=()

RED='\033[0;31m'
YEL='\033[0;33m'
GRN='\033[0;32m'
CYN='\033[0;36m'
BLD='\033[1m'
RST='\033[0m'

ok()   { echo -e "  ${GRN}[OK]${RST}  $*"; }
warn() { echo -e "  ${YEL}[WARN]${RST}  $*"; }
err()  { echo -e "  ${RED}[ERR]${RST}  $*"; }
info() { echo -e "  ${CYN}->${RST}  $*"; }

echo ""
echo -e "${BLD}╔══════════════════════════════════════════╗${RST}"
echo -e "${BLD}║          LightOS Dependency Util         ║${RST}"
echo -e "${BLD}╚══════════════════════════════════════════╝${RST}"
echo ""
echo -e "${BLD}[1/4] Detecting package manager...${RST}"

if command -v apt-get &>/dev/null; then
    PKG_MGR="apt"
    UPDATE_CMD=(sudo apt-get update)
    INSTALL_CMD=(sudo apt-get install -y)
elif command -v pacman &>/dev/null; then
    PKG_MGR="pacman"
    UPDATE_CMD=(sudo pacman -Sy)
    INSTALL_CMD=(sudo pacman -S --noconfirm)
elif command -v dnf &>/dev/null; then
    PKG_MGR="dnf"
    UPDATE_CMD=(sudo dnf makecache)
    INSTALL_CMD=(sudo dnf install -y)
elif command -v zypper &>/dev/null; then
    PKG_MGR="zypper"
    UPDATE_CMD=(sudo zypper refresh)
    INSTALL_CMD=(sudo zypper install -y)
else
    err "No supported package manager found (apt, pacman, dnf, zypper)."
    exit 1
fi

ok "Package manager: ${BLD}$PKG_MGR${RST}"

extract_version() {
    echo "$1" | grep -oP '\d+\.\d+(\.\d+)?' | head -1
}

version_ge() {
    [ "$(printf '%s\n%s' "$1" "$2" | sort -V | head -1)" = "$2" ]
}

version_le() {
    [ "$(printf '%s\n%s' "$1" "$2" | sort -V | tail -1)" = "$2" ]
}

get_pkg_name() {
    local tool=$1
    case "$tool" in
        gcc)
            case "$PKG_MGR" in
                apt)    echo "gcc-13" ;;
                pacman) echo "gcc13" ;;
                dnf)    echo "gcc" ;;
                *)      echo "gcc" ;;
            esac ;;
        g++)
            case "$PKG_MGR" in
                apt)    echo "g++-13" ;;
                pacman) echo "gcc13" ;;
                dnf)    echo "gcc-c++" ;;
                *)      echo "g++" ;;
            esac ;;
        binutils|ld|as)
            case "$PKG_MGR" in
                apt)    echo "binutils" ;;
                pacman) echo "binutils" ;;
                dnf)    echo "binutils" ;;
                *)      echo "binutils" ;;
            esac ;;
        awk)    echo "gawk" ;;
        ninja)
            case "$PKG_MGR" in
                apt|dnf|zypper) echo "ninja-build" ;;
                *) echo "ninja" ;;
            esac ;;
        ctags)
            case "$PKG_MGR" in
                apt) echo "universal-ctags" ;;
                *)   echo "ctags" ;;
            esac ;;
        xz)
            case "$PKG_MGR" in
                apt) echo "xz-utils" ;;
                *)   echo "xz" ;;
            esac ;;
        python3) echo "python3" ;;
        *)  echo "$tool" ;;
    esac
}

get_tool_version() {
    local tool=$1
    local ver=""
    case "$tool" in
        gcc*)      ver=$($tool --version 2>/dev/null | head -1) ;;
        g++*)      ver=$($tool --version 2>/dev/null | head -1) ;;
        binutils)  ver=$(ld --version 2>/dev/null | head -1) ;;
        ld)        ver=$(ld --version 2>/dev/null | head -1) ;;
        as)        ver=$(as --version 2>/dev/null | head -1) ;;
        nasm)      ver=$(nasm --version 2>/dev/null | head -1) ;;
        make)      ver=$(make --version 2>/dev/null | head -1) ;;
        bison)     ver=$(bison --version 2>/dev/null | head -1) ;;
        flex)      ver=$(flex --version 2>/dev/null | head -1) ;;
        python3)   ver=$(python3 --version 2>/dev/null) ;;
        git)       ver=$(git --version 2>/dev/null) ;;
        *)         ver=$($tool --version 2>/dev/null | head -1) ;;
    esac
    extract_version "$ver"
}

echo ""
echo -e "${BLD}[2/4] Checking versioned tools...${RST}"
echo ""

for entry in "${VERSIONED_TOOLS[@]}"; do
    IFS=':' read -r tool min_ver max_ver <<< "$entry"

    cmd="$tool"
    if [[ "$tool" == "gcc" && -n "$CC" ]]; then cmd="$CC"; fi
    if [[ "$tool" == "g++" && -n "$CXX" ]]; then cmd="$CXX"; fi

    if ! command -v "$cmd" &>/dev/null; then
        err "${BLD}$cmd${RST} not found в окружении"
        pkg=$(get_pkg_name "$tool")
        if [[ ! " ${MISSING_PACKAGES[*]} " =~ " $pkg " ]]; then
            MISSING_PACKAGES+=("$pkg")
        fi
        continue
    fi

    ver=$(get_tool_version "$cmd")

    if [ -z "$ver" ]; then
        warn "${BLD}$cmd${RST} - installed but version undetectable"
        continue
    fi

    if [[ "$tool" == "gcc" || "$tool" == "g++" ]]; then
        major_ver="${ver%%.*}"
        if [[ "$major_ver" != "12" && "$major_ver" != "13" ]]; then
            err "${BLD}$cmd${RST} - version ${BLD}$ver${RST} is ${RED}unsupported${RST} (strictly need 12.x or 13.x)"
            pkg=$(get_pkg_name "$tool")
            if [[ ! " ${MISSING_PACKAGES[*]} " =~ " $pkg " ]]; then
                MISSING_PACKAGES+=("$pkg")
            fi
            continue
        fi
    else
        if [ -n "$min_ver" ] && ! version_ge "$ver" "$min_ver"; then
            err "${BLD}$cmd${RST} - version ${BLD}$ver${RST} is ${RED}too old${RST} (need >= $min_ver)"
            pkg=$(get_pkg_name "$tool")
            if [[ ! " ${MISSING_PACKAGES[*]} " =~ " $pkg " ]]; then
                MISSING_PACKAGES+=("$pkg")
            fi
            continue
        fi

        if [ -n "$max_ver" ] && ! version_le "$ver" "$max_ver"; then
            warn "${BLD}$cmd${RST} - version ${BLD}$ver${RST} is ${YEL}too new${RST} (recommended <= $max_ver)"
            WARN_PACKAGES+=("$tool=$ver")
            continue
        fi
    fi

    if [ -z "$ver" ]; then
        warn "${BLD}$tool${RST} - installed but version undetectable"
        continue
    fi

    if [[ "$tool" == "gcc" || "$tool" == "g++" ]]; then
        major_ver="${ver%%.*}"
        if [[ "$major_ver" != "12" && "$major_ver" != "13" ]]; then
            err "${BLD}$tool${RST} - version ${BLD}$ver${RST} is ${RED}unsupported${RST} (strictly need 12.x or 13.x)"
            pkg=$(get_pkg_name "$tool")
            if [[ ! " ${MISSING_PACKAGES[*]} " =~ " $pkg " ]]; then
                MISSING_PACKAGES+=("$pkg")
            fi
            continue
        fi
    else
        if [ -n "$min_ver" ] && ! version_ge "$ver" "$min_ver"; then
            err "${BLD}$tool${RST} - version ${BLD}$ver${RST} is ${RED}too old${RST} (need >= $min_ver)"
            pkg=$(get_pkg_name "$tool")
            if [[ ! " ${MISSING_PACKAGES[*]} " =~ " $pkg " ]]; then
                MISSING_PACKAGES+=("$pkg")
            fi
            continue
        fi

        if [ -n "$max_ver" ] && ! version_le "$ver" "$max_ver"; then
            warn "${BLD}$tool${RST} - version ${BLD}$ver${RST} is ${YEL}too new${RST} (recommended <= $max_ver)"
            WARN_PACKAGES+=("$tool=$ver")
            continue
        fi
    fi

    range=""
    if [[ "$tool" == "gcc" || "$tool" == "g++" ]]; then
        range="12 or 13"
    else
        [ -n "$min_ver" ] && range+=" >= $min_ver"
        [ -n "$max_ver" ] && range+=" <= $max_ver"
    fi
    ok "${BLD}$tool${RST} — ${GRN}$ver${RST}${range:+  (req:$range)}"
done

echo ""
echo -e "${BLD}[3/4] Checking plain tools...${RST}"
echo ""

for tool in "${PLAIN_TOOLS[@]}"; do
    if ! command -v "$tool" &>/dev/null; then
        err "${BLD}$tool${RST} not found"
        pkg=$(get_pkg_name "$tool")
        if [[ ! " ${MISSING_PACKAGES[*]} " =~ " $pkg " ]]; then
            MISSING_PACKAGES+=("$pkg")
        fi
    else
        ver=$(get_tool_version "$tool")
        if [ -n "$ver" ]; then
            ok "${BLD}$tool${RST} — $ver"
        else
            ok "${BLD}$tool${RST} — installed"
        fi
    fi
done

echo ""
echo -e "${BLD}[4/4] Summary${RST}"
echo ""

if [ ${#WARN_PACKAGES[@]} -gt 0 ]; then
    warn "Version warnings (these may cause issues building the GCC 11.1.0 cross-compiler):"
    for w in "${WARN_PACKAGES[@]}"; do
        info "  $w"
    done
    echo ""
fi

if [ ${#MISSING_PACKAGES[@]} -eq 0 ]; then
    ok "All required dependencies are present!"
    exit 0
fi

err "Missing or outdated packages: ${BLD}${MISSING_PACKAGES[*]}${RST}"
echo ""
read -rp "  Install them now? [y/N] " answer
if [[ "$answer" =~ ^[Yy]$ ]]; then
    echo ""
    info "Running: ${UPDATE_CMD[*]}"
    "${UPDATE_CMD[@]}"
    echo ""
    info "Running: ${INSTALL_CMD[*]} ${MISSING_PACKAGES[*]}"
    "${INSTALL_CMD[@]}" "${MISSING_PACKAGES[@]}"
    echo ""
    ok "Done. Re-run this script to verify everything is correct. (make utils)"
else
    echo ""
    info "Skipped. Install manually:"
    echo ""
    echo "  ${INSTALL_CMD[*]} ${MISSING_PACKAGES[*]}"
    echo ""
fi
