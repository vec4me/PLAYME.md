fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

sh tools/build_server.sh || fail "could not build server"

build/playme_server 2>/dev/null
if [ "$?" -ne 1 ]; then
    fail "server accepted a missing port"
fi

for port in invalid 0 65536; do
    build/playme_server "$port" 2>/dev/null
    if [ "$?" -ne 1 ]; then
        fail "server accepted invalid port: $port"
    fi
done
