fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

sourcefolders=$(find benchmark client playme server -type f -name '*.c' ! -path '*/tests/*' -exec dirname {} \; | sort -u) || fail "could not find source folders"

for folder in $sourcefolders; do
    case "$folder" in
        benchmark|client|server)
            entry=main.c
            ;;
        playme)
            entry=
            ;;
        *)
            if [ ! -f "$folder/lib.h" ]; then
                fail "$folder/ must have one lib.h public boundary"
            fi
            entry=
            ;;
    esac
    if [ -n "$entry" ] && [ ! -f "$folder/$entry" ]; then
        fail "$folder/ must have exactly one $entry entry"
    fi
    nitems=$(find "$folder" -mindepth 1 -maxdepth 1 \( -type f \( -name '*.ccm' -o -name '*.c' -o -name '*.h' \) -o -type d ! -name tests \) -print | wc -l) || fail "could not count meaningful items in $folder/"
    if [ "$nitems" -lt 2 ]; then
        fail "$folder/ contains only one meaningful item and must be collapsed"
    fi
done
