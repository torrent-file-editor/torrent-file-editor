#!/usr/bin/bash

set -x
set -e

if [ -z "$CROWDIN_TFE_KEY" ]; then
    echo "CROWDIN_TFE_KEY not set"
    exit 1
fi

trim()
{
    local trimmed="$1"
    trimmed=$(echo $trimmed | sed -e 's|^ *\(.*\) *$|\1|')
    echo "$trimmed"
}


# af_ZA: ar_SA: bn_BD: ca_ES: cs_CZ: da_DK: de_DE: el_GR: en_US: es_ES: fi_FI: fr_FR: he_IL: hu_HU: id_ID: it_IT: ja_JP: ko_KR: nl_NL: no_NO: pl_PL: pt_BR: pt_PT: ro_RO: ru_RU: sr_SP: sv_SE: tr_TR: uk_UA: vi_VN: zh_CN: zh_TW: uk_UA

langsmap="af_ZA:af:Afrikaans:Afrikaans
          ar_SA:ar:Arabic:العربية
          bn_BD:bn:Bengali:বাংলা
          zh_CN:zh_CN:Chinese Simplified:简体中文
          zh_TW:zh_TW:Chinese Traditional:繁體中文
          cs_CZ:cs:Czech:Čeština
          nl_NL:nl:Dutch:Nederlands
          en_US:en:English:English
          fi_FI:fi:Finnish:Suomi
          fr_FR:fr:French:Français
          de_DE:de:German:Deutsch
          he_IL:he:Hebrew:עברית‎
          hu_HU:hu:Hungarian:Magyar
          id_ID:id:Indonesian:Indonesia
          it_IT:it:Italian:Italiano
          ja_JP:ja:Japanese:日本語
          ko_KR:ko:Korean:한국어
          pl_PL:pl:Polish:Polski
          pt_BR:pt:Portuguese (Brazil):Português  (Brasil)
          ro_RO:ro:Romanian:Română
          ru_RU:ru:Russian:Русский
          es_ES:es:Spanish:Español
          tr_TR:tr:Turkish:Türkçe
          vi_VN:vi:Vietnamese:Tiếng Việt
          uk_UA:uk:Ukrainian:Украї́нська"

pushd translations
[ -d tmp ] && rm -fr tmp
mkdir tmp
wget "https://api.crowdin.com/api/project/torrent-file-editor/download/all.zip?key=$CROWDIN_TFE_KEY" -O tmp/torrent-file-editor.zip
unzip -d tmp tmp/torrent-file-editor.zip

# Check source files
IFS=$'\n'
for lang in $langsmap; do
    srclang=$(trim $(echo $lang | cut -f1 -d:))

    if [ ! -f  "tmp/torrentfileeditor_${srclang}.ts" ]; then
        echo "Source file is not exist"
        exit 1
    fi
done

# Delete previously generated files
[ -f ../all_languages.txt ] && rm ../all_languages.txt
[ -f ../all_languages.yaml ] && rm ../all_languages.yaml

# Copy source files
for lang in $langsmap; do
    srclang=$(trim $(echo $lang | cut -f1 -d:))
    dstlang=$(trim $(echo $lang | cut -f2 -d:))
    englishlang=$(trim $(echo $lang | cut -f3 -d:))
    nativelang=$(trim $(echo $lang | cut -f4 -d:))
    country=$(echo $srclang | cut -f2 -d_ | tr '[:upper:]' '[:lower:]')

    echo "<img src=\"https://lipis.github.io/flag-icon-css/flags/4x3/$country.svg\" width=\"24\" height=\"24\">  $nativelang - $englishlang  " >> ../all_languages.txt
    echo "  - {country: \"$country\", nativelang: \"$nativelang\", englishlang: \"$englishlang\"}" >> ../all_languages.yaml
    cp "tmp/torrentfileeditor_$srclang.ts" "torrentfileeditor_$dstlang.ts"
done
unset IFS

# Delete temporary folder
rm -fr tmp

popd
