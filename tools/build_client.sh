fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

CC=${CC:-cc}
CPPFLAGS=${CPPFLAGS:-}
CFLAGS=${CFLAGS:-}
LDFLAGS=${LDFLAGS:-}
LDLIBS=${LDLIBS:-}
target=$($CC -dumpmachine) || fail "could not identify compiler target"

case "$target" in
    *mingw*)
        clientlibraries="-lgdi32 -lshell32"
        clientprogram=build/playme_client.exe
        ;;
    *)
        clientlibraries="-ldl"
        clientprogram=build/playme_client
        ;;
esac

build_program() {
    $CC \
        $CPPFLAGS \
        $CFLAGS \
        -c "$1/main.c" \
        -o "build/$2.o" || return 1
}

objects=""
sources=""

mkdir -p build/ || fail "could not create build directory"

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
$CC $CPPFLAGS -Ivendor/x11/include -c client/window/window.c -o build/window.o || fail "could not build window authority"
$CC $CPPFLAGS $CFLAGS -c client/runtime.c -o build/client_runtime.o || fail "could not build client runtime"
objects="build/encoded_image.o build/gif_c.o build/jpeg_c.o build/media_c.o build/world.o build/numeric_c.o build/game_c.o build/rasterizer_c.o build/text_c.o build/renderer_c.o build/capture_c.o build/window.o build/client_runtime.o"

sh tools/check_imports.sh || fail "invalid authority import"
build_program client cli_c || fail "could not build client program"
objects="$objects build/cli_c.o"
sources="$sources client/main.c"

$CC \
    $LDFLAGS \
    $objects \
    $clientlibraries \
    $LDLIBS \
    -o "$clientprogram" || fail "could not link Playme client"
