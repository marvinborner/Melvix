#!/bin/sh
# bdf2c output formatter
# TODO: Currently only works with 1 byte widths

font_url="https://raw.githubusercontent.com/fcambus/spleen/master/spleen-8x16.bdf"
character_count=758
font_height=16
top_lines="#include <stdint.h>\n\nuint16_t font_bitmap[$character_count][$font_height] = {"

rm -rf font
mkdir font
cd font || exit
wget $font_url -O temp.bdf
bdf2c -C font.h
bdf2c -b < temp.bdf > temp.c
gcc -E -CC temp.c > final.h

sed -i -e 1,74d final.h
sed -i -e :a -e '$d;N;2,1533ba' -e 'P;D' final.h

sed -i "1s/.*/$top_lines/" final.h
sed -i -z "s/$font_height\n /$font_height\n        \{/g" final.h
sed -i -z 's/\,\n\/\//\}\,\n\/\//g' final.h
sed -i 's/\/\/ [0-9]/        \/\/ /g' final.h
sed -i 's/\/\/\t/        \/\/ /g' final.h
sed -i 's/\/\/  /        \/\/ /g' final.h
sed -i -z 's/\,\n 0x/\, 0x/g' final.h

rm font.h
rm temp.*