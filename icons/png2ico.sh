#!/usr/bin/env bash

# SPDX-FileCopyrightText: NONE
# SPDX-License-Identifier: CC0-1.0

set -e

SIZES=(16 32 48 64 128 512)

INPUTS=""
for SIZE in "${SIZES[@]}"; do
    INPUTS="$INPUTS windows/app_${SIZE}.png"
done

magick $INPUTS app.ico
