fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

for testfile in tests/*.c playme/*/tests/*.c server/*/tests/*.c; do
    if test ! -f "$testfile"; then
        continue
    fi
    sh "$testfile" || fail "behavioral test failed: $testfile"
done

for testfile in tests/*.sh; do
    sh "$testfile" || fail "behavioral test failed: $testfile"
done
