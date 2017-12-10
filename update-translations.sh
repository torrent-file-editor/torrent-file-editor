#!/usr/bin/bash

set -x
set -e

src=$1

if [ ! -d "$src" ]; then
    echo "Source dir is not set"
    exit 1
fi

# af_ZA: ar_SA: bn_BD: ca_ES: cs_CZ: da_DK: de_DE: el_GR: en_US: es_ES: fi_FI: fr_FR: he_IL: hu_HU: id_ID: it_IT: ja_JP: ko_KR: nl_NL: no_NO: pl_PL: pt_BR: pt_PT: ro_RO: ru_RU: sr_SP: sv_SE: tr_TR: uk_UA: vi_VN: zh_CN: zh_TW:

langsmap="ar_SA:ar
          bn_BD:bn
          cs_CZ:cs
          de_DE:de
          en_US:en
          es_ES:es
          fr_FR:fr
          he_IL:he
          ko_KR:ko
          ru_RU:ru
          hu_HU:hu
          id_ID:id
          it_IT:it
          nl_NL:nl
          pl_PL:pl
          pt_BR:pt_BR
          tr_TR:tr
          zh_CN:zh_CN"

# Check source files
for lang in $langsmap; do
    srclang=$(echo $lang | cut -f1 -d:)
    dstlang=$(echo $lang | cut -f2 -d:)

    if [ ! -f  "$src/torrentfileeditor_${srclang}.ts" ]; then
        echo "Source file is not exist"
        exit 1
    fi
done

# Copy source files
for lang in $langsmap; do
    srclang=$(echo $lang | cut -f1 -d:)
    dstlang=$(echo $lang | cut -f2 -d:)

    cp "$src/torrentfileeditor_${srclang}.ts" "translations/torrentfileeditor_${dstlang}.ts"
done
