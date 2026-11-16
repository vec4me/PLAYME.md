fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

CC=${CC:-cc}
CPPFLAGS=${CPPFLAGS:-}
CFLAGS=${CFLAGS:-}

sh tools/check_imports.sh || fail "invalid mutation-authority import"
sources=$(find benchmark client playme server tests -type f -name '*.c' -print) || fail "could not find owned C sources"
checksources=$(find benchmark client playme server tests -type f -name '*.c' ! -path 'client/window/window.c' -print) || fail "could not find directly analyzable C sources"

cppcheck \
    --platform=unix64 \
    --check-level=exhaustive \
    --enable=warning,performance,portability \
    --error-exitcode=1 \
    --language=c \
    --std=c89 \
    $checksources || fail "cppcheck failed"

for source in $sources; do
    case "$source" in
        client/window/window.c)
            include-what-you-use $CPPFLAGS -Ivendor/x11/include -c "$source" -o /tmp/playme_iwyu.o || fail "include-what-you-use failed for $source"
            ;;
        *)
            include-what-you-use $CPPFLAGS $CFLAGS -c "$source" -o /tmp/playme_iwyu.o || fail "include-what-you-use failed for $source"
            ;;
    esac
done
