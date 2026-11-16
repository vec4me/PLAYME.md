fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

ContainsModule() {
    for item in $2; do
        if [ "$item" = "$1" ]; then
            return 0
        fi
    done
    return 1
}

collect_tree() {
    if ContainsModule "$1" "$collected"; then
        return 0
    fi
    source=$(ModuleSource "$1") || return 1
    for dependency in $(sed -n 's/^\(export \)\{0,1\}import \([^;]*\);/\2/p' "$source"); do
        if ModuleSource "$dependency" >/dev/null; then
            collect_tree "$dependency" || return 1
        fi
    done
    collected="$collected $1"
    sources="$sources $source"
}

ModuleSource() {
    for candidate in $(find benchmark client playme server -name '*.ccm' -print); do
        module=$(sed -n 's/^export module \([^;]*\);/\1/p' "$candidate") || return 1
        if [ "$module" = "$1" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done
    return 1
}

collected=""
sources=""

collect_tree playme_s || fail "could not collect server sources"
sources="$sources server/main.c"

cppcheck \
    --platform=unix64 \
    --check-level=exhaustive \
    --enable=warning,performance,portability \
    --error-exitcode=1 \
    --language=c++ \
    --std=c++20 \
    $sources || fail "cppcheck failed for server"
