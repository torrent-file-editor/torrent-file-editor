#!/usr/bin/bash

set -x
set -e

trim()
{
    local trimmed="$1"
    trimmed=$(echo $trimmed | sed -e 's|^ *\(.*\) *$|\1|')
    echo "$trimmed"
}

src=$1

if [ ! -d "$src" ]; then
    echo "Source dir is not set"
    exit 1
fi

# af_ZA: ar_SA: bn_BD: ca_ES: cs_CZ: da_DK: de_DE: el_GR: en_US: es_ES: fi_FI: fr_FR: he_IL: hu_HU: id_ID: it_IT: ja_JP: ko_KR: nl_NL: no_NO: pl_PL: pt_BR: pt_PT: ro_RO: ru_RU: sr_SP: sv_SE: tr_TR: uk_UA: vi_VN: zh_CN: zh_TW:

langsmap="ar_SA:ar:Arabic - اللغة العربية
          bn_BD:bn:Bengali - বাংলা
          cs_CZ:cs:Czech – Český Jazyk, Čeština
          de_DE:de:German – Deutsch
          en_US:en:English - English
          es_ES:es:Spanish – Español
          fr_FR:fr:French – Français
          he_IL:he:Hebrew - עברית‎
          id_ID:id:Indonesian – Bahasa Indonesia
          it_IT:it:Italian – Italiano
          ja_JP:ja:Japanese - 日本語
          ko_KR:ko:Korean - 조선말, 한국어
          nl_NL:nl:Dutch - Nederlands
          pl_PL:pl:Polish – Język polski, polski, or polszczyzna
          pt_BR:pt_BR:Brazilian Portuguese – Português brasileiro
          ru_RU:ru:Russian - Русский
          tr_TR:tr:Turkish – Türkçe
          vi_VN:vi:Vietnamese - Tiếng Việt
          zh_CN:zh_CN:Simplified Chinese - 简化字
          zh_TW:zh_TW:Traditional Chinese - 正體字/繁體字"

# Check source files
[ -f all_languages.txt ] && rm all_languages.txt
IFS=$'\n'
for lang in $langsmap; do
    srclang=$(trim $(echo $lang | cut -f1 -d:))

    if [ ! -f  "$src/torrentfileeditor_${srclang}.ts" ]; then
        echo "Source file is not exist"
        exit 1
    fi
done

# Copy source files
for lang in $langsmap; do
    srclang=$(trim $(echo $lang | cut -f1 -d:))
    dstlang=$(trim $(echo $lang | cut -f2 -d:))
    humanlang=$(trim $(echo $lang | cut -f3 -d:))
    country=$(echo $srclang | cut -f2 -d_ | tr '[:upper:]' '[:lower:]')

    echo "<img src=\"https://lipis.github.io/flag-icon-css/flags/4x3/$country.svg\" width=\"24\" height=\"24\" /> ${humanlang}  " >> all_languages.txt
    cp "$src/torrentfileeditor_${srclang}.ts" "translations/torrentfileeditor_${dstlang}.ts"
done
unset IFS
