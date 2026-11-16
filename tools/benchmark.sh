fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

CC=${CC:-cc}
CXX=${CXX:-c++}
CPPFLAGS=${CPPFLAGS:-}
CFLAGS=${CFLAGS:-}
CXXFLAGS=${CXXFLAGS:-}
LDFLAGS=${LDFLAGS:-}
LDLIBS=${LDLIBS:-}
export CC CXX CPPFLAGS CFLAGS CXXFLAGS LDFLAGS LDLIBS

label=$1
case "$label" in
    ""|*[!a-z0-9_]*) fail "benchmark label must contain lowercase letters, digits, or underscores" ;;
esac

snapshot="measurements/$label"
if [ -e "$snapshot" ]; then
    fail "benchmark snapshot already exists: $snapshot"
fi

sh tools/build_server.sh || fail "could not build server"
mkdir -p "$snapshot" || fail "could not create benchmark snapshot"

$CC \
    $CPPFLAGS \
    $CFLAGS \
    -c benchmark/workload.c \
    -o "$snapshot/capture_workload.o" || fail "could not build capture workload"

$CC \
    $CPPFLAGS \
    $CFLAGS \
    -c benchmark/main.c \
    -o "$snapshot/main.o" || fail "could not build capture benchmark entry"

$CC \
    $LDFLAGS \
    "$snapshot/main.o" \
    "$snapshot/capture_workload.o" \
    build/gif_c.o \
    build/game_c.o \
    build/encoded_image.o \
    build/capture_c.o \
    build/jpeg_c.o \
    build/media_c.o \
    build/world.o \
    build/numeric_c.o \
    build/renderer_c.o \
    build/rasterizer_c.o \
    build/text_c.o \
    $LDLIBS \
    -o "$snapshot/capture_benchmark" || fail "could not build capture benchmark"

cp build/playme_server "$snapshot/playme_server" || fail "could not preserve server binary"
size "$snapshot/playme_server" "$snapshot/capture_benchmark" > "$snapshot/size.txt" || fail "could not measure binary sizes"
sha256sum "$snapshot/playme_server" "$snapshot/capture_benchmark" > "$snapshot/sha256.txt" || fail "could not identify benchmark binaries"
printf '%s\n' "CC=$CC" "CXX=$CXX" "CPPFLAGS=$CPPFLAGS" "CFLAGS=$CFLAGS" "CXXFLAGS=$CXXFLAGS" "LDFLAGS=$LDFLAGS" "LDLIBS=$LDLIBS" '30 runs' > "$snapshot/build.txt" || fail "could not record build configuration"
perf stat \
    -r 30 \
    -e task-clock,cycles,instructions,branches,branch-misses,cache-references,cache-misses \
    -o "$snapshot/perf.txt" \
    "$snapshot/capture_benchmark" || fail "could not measure capture benchmark"

cat "$snapshot/size.txt" "$snapshot/perf.txt"
