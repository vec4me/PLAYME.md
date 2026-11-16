fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

download() {
    curl \
        --fail \
        --location \
        --output "$1" \
        "$2" || fail "could not download $3"
    actualsha256=$(sha256sum "$1" | awk '{ print $1 }') || fail "could not identify $3 download"
    if [ "$actualsha256" != "$4" ]; then
        fail "$3 download did not match the pinned artifact"
    fi
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

rgfwcommit=d96684e6877d3ef11c731b122b8949942ed071c9
dependencyfolder=$(mktemp -d) || fail "could not create temporary dependency directory"
trap 'rm -rf "$dependencyfolder"' EXIT HUP INT TERM

download "$dependencyfolder/RGFW.h" "https://raw.githubusercontent.com/ColleagueRiley/RGFW/$rgfwcommit/RGFW.h" RGFW ece357894cf1e5d178c5343d877c4b8c66e6d0a42164b2b2e49220142700448d
download "$dependencyfolder/XDL.h" "https://raw.githubusercontent.com/ColleagueRiley/RGFW/$rgfwcommit/XDL.h" XDL b5a19e8b557357950953564abebe4423a0be99c0cd98de998f7a209e7f053c1c
download "$dependencyfolder/xorgproto.tar.xz" "https://www.x.org/releases/individual/proto/xorgproto-2024.1.tar.xz" xorgproto 372225fd40815b8423547f5d890c5debc72e88b91088fbfb13158c20495ccb59
download "$dependencyfolder/lib_x11.tar.xz" "https://www.x.org/releases/individual/lib/libX11-1.8.12.tar.xz" libX11 fa026f9bb0124f4d6c808f9aef4057aad65e7b35d8ff43951cef0abe06bb9a9a
download "$dependencyfolder/lib_xrandr.tar.xz" "https://www.x.org/releases/individual/lib/libXrandr-1.5.4.tar.xz" libXrandr 1ad5b065375f4a85915aa60611cc6407c060492a214d7f9daf214be752c3b4d3
download "$dependencyfolder/lib_xi.tar.xz" "https://www.x.org/releases/individual/lib/libXi-1.8.2.tar.xz" libXi d0e0555e53d6e2114eabfa44226ba162d2708501a25e18d99cfb35c094c6c104
download "$dependencyfolder/lib_xext.tar.xz" "https://www.x.org/releases/individual/lib/libXext-1.3.6.tar.xz" libXext edb59fa23994e405fdc5b400afdf5820ae6160b94f35e3dc3da4457a16e89753
download "$dependencyfolder/lib_xrender.tar.xz" "https://www.x.org/releases/individual/lib/libXrender-0.9.12.tar.xz" libXrender b832128da48b39c8d608224481743403ad1691bf4e554e4be9c174df171d1b97
download "$dependencyfolder/lib_xfixes.tar.xz" "https://www.x.org/releases/individual/lib/libXfixes-6.0.1.tar.xz" libXfixes b695f93cd2499421ab02d22744458e650ccc88c1d4c8130d60200213abc02d58

python3 -m tarfile -e "$dependencyfolder/xorgproto.tar.xz" "$dependencyfolder" || fail "could not extract xorgproto"
python3 -m tarfile -e "$dependencyfolder/lib_x11.tar.xz" "$dependencyfolder" || fail "could not extract libX11"
python3 -m tarfile -e "$dependencyfolder/lib_xrandr.tar.xz" "$dependencyfolder" || fail "could not extract libXrandr"
python3 -m tarfile -e "$dependencyfolder/lib_xi.tar.xz" "$dependencyfolder" || fail "could not extract libXi"
python3 -m tarfile -e "$dependencyfolder/lib_xext.tar.xz" "$dependencyfolder" || fail "could not extract libXext"
python3 -m tarfile -e "$dependencyfolder/lib_xrender.tar.xz" "$dependencyfolder" || fail "could not extract libXrender"
python3 -m tarfile -e "$dependencyfolder/lib_xfixes.tar.xz" "$dependencyfolder" || fail "could not extract libXfixes"

mkdir -p vendor/rgfw/ vendor/x11/include/X11/ vendor/x11/licenses/ || fail "could not create vendor directories"
cp "$dependencyfolder/RGFW.h" vendor/rgfw/RGFW.h || fail "could not install RGFW"
cp "$dependencyfolder/XDL.h" vendor/rgfw/XDL.h || fail "could not install XDL"
cp -R "$dependencyfolder/xorgproto-2024.1/include/X11/." vendor/x11/include/X11/ || fail "could not install xorgproto headers"
cp -R "$dependencyfolder/libX11-1.8.12/include/X11/." vendor/x11/include/X11/ || fail "could not install libX11 headers"
cp -R "$dependencyfolder/libXrandr-1.5.4/include/X11/." vendor/x11/include/X11/ || fail "could not install libXrandr headers"
cp -R "$dependencyfolder/libXi-1.8.2/include/X11/." vendor/x11/include/X11/ || fail "could not install libXi headers"
cp -R "$dependencyfolder/libXext-1.3.6/include/X11/." vendor/x11/include/X11/ || fail "could not install libXext headers"
cp -R "$dependencyfolder/libXrender-0.9.12/include/X11/." vendor/x11/include/X11/ || fail "could not install libXrender headers"
cp -R "$dependencyfolder/libXfixes-6.0.1/include/X11/." vendor/x11/include/X11/ || fail "could not install libXfixes headers"
cp "$dependencyfolder/libX11-1.8.12/COPYING" vendor/x11/licenses/lib_x11.txt || fail "could not install libX11 license"
cp "$dependencyfolder/libXrandr-1.5.4/COPYING" vendor/x11/licenses/lib_xrandr.txt || fail "could not install libXrandr license"
cp "$dependencyfolder/libXi-1.8.2/COPYING" vendor/x11/licenses/lib_xi.txt || fail "could not install libXi license"
cp "$dependencyfolder/libXext-1.3.6/COPYING" vendor/x11/licenses/lib_xext.txt || fail "could not install libXext license"
cp "$dependencyfolder/libXrender-0.9.12/COPYING" vendor/x11/licenses/lib_xrender.txt || fail "could not install libXrender license"
cp "$dependencyfolder/libXfixes-6.0.1/COPYING" vendor/x11/licenses/lib_xfixes.txt || fail "could not install libXfixes license"
cp "$dependencyfolder/xorgproto-2024.1/COPYING-"* vendor/x11/licenses/ || fail "could not install xorgproto licenses"

printf '%s\n' "installed RGFW 1.8.1 and pinned X11 headers"
