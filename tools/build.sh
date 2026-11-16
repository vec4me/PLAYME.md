fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

sh tools/build_server.sh || fail "could not build server"
sh tools/build_client.sh || fail "could not build client"
