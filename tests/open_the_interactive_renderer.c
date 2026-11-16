#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

sh tools/build_client.sh || fail "could not build Playme client"
build/playme_client || fail "could not run Playme client"
exit 0
#endif

int open_the_interactive_renderer_is_exercised_by_the_client_program;
