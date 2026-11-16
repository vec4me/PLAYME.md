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

ContainsModule() {
    for module in $2; do
        if [ "$module" = "$1" ]; then
            return 0
        fi
    done
    return 1
}

ModuleDependencies() {
    source=$(ModuleSource "$1") || return 1
    modules=$(sed -n 's/^\(export \)\{0,1\}import \([^;]*\);/\2/p' "$source") || return 1
    for dep in $modules; do
        if ModuleSource "$dep" >/dev/null; then
            printf '%s\n' "$dep"
        fi
    done
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

build_module() {
    source=$(ModuleSource "$1") || return 1
    $CXX \
        $CPPFLAGS \
        $CXXFLAGS \
        -fprebuilt-module-path=build \
        -x c++-module \
        -fmodule-output="build/$1.pcm" \
        -c "$source" \
        -o "build/$1.o" || return 1
}

build_program() {
    $CC \
        $CPPFLAGS \
        $CFLAGS \
        -c "$1/main.c" \
        -o "build/$2.o" || return 1
}

build_tree() {
    if ContainsModule "$1" "$builtmodules"; then
        return 0
    fi
    deps=$(ModuleDependencies "$1") || return 1
    for dep in $deps; do
        build_tree "$dep" || return 1
    done
    build_module "$1" || return 1
    builtmodules="$builtmodules $1"
    objects="$objects build/$1.o"
    sources="$sources $(ModuleSource "$1")"
}

builtmodules=""
objects=""
sources=""

mkdir -p build/ || fail "could not create build directory"

$CC $CPPFLAGS $CFLAGS -c server/network_channel/channel.c -o build/network_channel_c.o || fail "could not build network channel"
$CC $CPPFLAGS $CFLAGS -c server/network_channel/socket.c -o build/network_socket.o || fail "could not build network socket"
$CC $CPPFLAGS $CFLAGS -c server/network_channel/http.c -o build/http.o || fail "could not build HTTP"
$CC $CPPFLAGS $CFLAGS -c playme/media/encoded_image.c -o build/encoded_image.o || fail "could not build encoded image"
$CC $CPPFLAGS $CFLAGS -c playme/media/gif.c -o build/gif_c.o || fail "could not build GIF encoder"
$CC $CPPFLAGS $CFLAGS -c playme/media/jpeg.c -o build/jpeg_c.o || fail "could not build JPEG encoder"
$CC $CPPFLAGS $CFLAGS -c playme/media/encoding.c -o build/media_c.o || fail "could not build media library"
$CC $CPPFLAGS $CFLAGS -c playme/renderer/world.c -o build/world.o || fail "could not build world"
$CC $CPPFLAGS $CFLAGS -c playme/numeric.c -o build/numeric_c.o || fail "could not build numeric module"
$CC $CPPFLAGS $CFLAGS -c playme/game/state.c -o build/game_c.o || fail "could not build game"
$CC $CPPFLAGS $CFLAGS -c playme/renderer/rasterizer.c -o build/rasterizer_c.o || fail "could not build rasterizer"
$CC $CPPFLAGS $CFLAGS -c playme/renderer/text.c -o build/text_c.o || fail "could not build renderer text"
$CC $CPPFLAGS $CFLAGS -c playme/renderer/frame.c -o build/renderer_c.o || fail "could not build renderer"
$CC $CPPFLAGS $CFLAGS -c playme/capture/frame.c -o build/capture_c.o || fail "could not build capture coordinator"
$CC $CPPFLAGS $CFLAGS -c server/game_session/state.c -o build/game_session_c.o || fail "could not build game session"
$CC $CPPFLAGS $CFLAGS -c server/runtime.c -o build/server_runtime.o || fail "could not build server runtime"
objects="build/network_channel_c.o build/network_socket.o build/http.o build/encoded_image.o build/gif_c.o build/jpeg_c.o build/media_c.o build/world.o build/numeric_c.o build/game_c.o build/rasterizer_c.o build/text_c.o build/renderer_c.o build/capture_c.o build/game_session_c.o build/server_runtime.o"

sh tools/check_imports.sh || fail "invalid authority import"
build_program server cli_s || fail "could not build server program"
objects="$objects build/cli_s.o"
sources="$sources server/main.c"

$CC \
    $LDFLAGS \
    $objects \
    $LDLIBS \
    -o build/playme_server || fail "could not link Playme server"
