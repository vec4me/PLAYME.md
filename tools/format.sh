fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

sources=$(find benchmark client playme server tests -type f \( -name '*.ccm' -o -name '*.c' -o -name '*.h' \) -print) || fail "could not find sources"

clang-format \
    -i \
    --style="{BasedOnStyle: LLVM, IndentWidth: 4, ColumnLimit: 200, AllowShortEnumsOnASingleLine: false, AllowShortFunctionsOnASingleLine: None, IndentCaseLabels: true, InsertBraces: true, BreakBeforeBraces: Custom, BraceWrapping: {AfterFunction: true}}" \
    $sources || fail "could not format sources"
