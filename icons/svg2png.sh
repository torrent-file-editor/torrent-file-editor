#!/usr/bin/env bash

# SPDX-FileCopyrightText: NONE
# SPDX-License-Identifier: CC0-1.0

set -e

[ -d unix ] || mkdir unix
[ -d windows ] || mkdir windows

inkscape app.svg --export-area-drawing --export-filename=app_trim.svg

for i in 16 22 32 44 48 64 128 150 256 310 512 1024; do
    inkscape app.svg -w $i -o unix/app_$i.png
    inkscape app_trim.svg -w $i -o windows/app_$i.png
done

rm app_trim.svg
