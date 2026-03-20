#!/usr/bin/env bash
set -e

ICONSET="application.iconset"
OUTPUT="application.icns"

rm -rf "$ICONSET"
mkdir -p "$ICONSET"

copy_icon() {
    local SRC="unix/$1"
    local DST="$2"
    cp "$SRC" "$ICONSET/$DST"
}

copy_icon "app_16.png"   "icon_16x16.png"
copy_icon "app_32.png"   "icon_16x16@2x.png"

copy_icon "app_32.png"   "icon_32x32.png"
copy_icon "app_64.png"   "icon_32x32@2x.png"

copy_icon "app_128.png"  "icon_128x128.png"
copy_icon "app_256.png"  "icon_128x128@2x.png"

copy_icon "app_256.png"  "icon_256x256.png"
copy_icon "app_512.png"  "icon_256x256@2x.png"

copy_icon "app_512.png"  "icon_512x512.png"
copy_icon "app_1024.png" "icon_512x512@2x.png"

iconutil -c icns -o "$OUTPUT" "$ICONSET"

rm -rf "$ICONSET"
