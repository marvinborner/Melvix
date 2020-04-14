#!/bin/sh
# bdf2c output formatter
# Warning! This code is really ugly and shall not be changed!

printf "// Generated using the Spleen font and the bdf2c converter (modified using the conv.sh script)\n// Spleen font: (c) 2018-2019, Frederic Cambus, License: MIT\n // bdf2c: (c) 2009-2010 Lutz Sammer, License: AGPLv3\n\n" >font.c

printf "#include <stdint.h>\n\nuint32_t magic=0xf0f0f0f0;\n\nuint16_t cursor[19]={0b100000000000,0b110000000000,0b111000000000,0b111100000000,0b111110000000,0b111111000000,0b111111100000,0b111111110000,0b111111111000,0b111111111100,0b111111111110,0b111111111111,0b111111111111,0b111111110000,0b111101111000,0b111001111000,0b110000111100,0b000000111100,0b000000011000};\n" >>font.c

generate() {
	font_url="https://raw.githubusercontent.com/fcambus/spleen/master/spleen-$1x$2.bdf"
	character_count=758
	font_height=$2
	top_lines="\nuint$1_t font_$font_height\[$character_count][$font_height] = {"

	mkdir -p font
	cd font || exit
	wget "$font_url" -O temp.bdf
	bdf2c -C font.h
	bdf2c -b <temp.bdf >temp.c
	gcc -E -CC temp.c >final.h

	sed -i -e 1,74d final.h                          # Remove first 74 lines
	sed -i -e :a -e '$d;N;2,1533ba' -e 'P;D' final.h # Remove last 1533 lines

	sed -i "1s/.*/$top_lines/" final.h                       # Prepend header
	sed -i -z 's/\,\n\/\//\}\,\n\/\//g' final.h              # Add closing brackets
	sed -i -z "s/$font_height\n /$font_height\n\{/g" final.h # Add opening bracket
	sed -i -z 's/\,\n\}/\}\n\}/g' final.h                    # Add closing bracket in last-1 line
	sed -i 's/\/\/.*$//g' final.h                            # Remove all comments
	sed -i -z 's/\n\n/\n/g' final.h                          # Remove all double new lines
	sed -i -z 's/\,\n/,/g' final.h                           # Aaand into single lines
	sed -i 's/\,0x//g' final.h                               # Fix bytes
	sed -i 's/uint12/uint16/g' final.h                       # Fix non-existent 12 bit bound

	mv final.h "font_$2.c"
	rm font.h
	rm temp.*
	cd ..
}

generate 8 16
generate 12 24
generate 16 32

cat font/font* >>font.c
rm -rf font/