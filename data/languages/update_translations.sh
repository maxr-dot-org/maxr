#!/bin/sh

xgettext --from-code=UTF-8 --c++ -o data/languages/maxr.pot -s -k -ki18n -kplural:1,1 --no-location `find ../../src "(" -name "*.cpp" -or -name "*.h" ")" -not -wholename "../../src/3rd/*.*"`

msgmerge --update -s --no-location --no-wrap ca/maxr.po maxr.pot
msgmerge --update -s --no-location --no-wrap de/maxr.po maxr.pot
msgmerge --update -s --no-location --no-wrap en/maxr.po maxr.pot
msgmerge --update -s --no-location --no-wrap es/maxr.po maxr.pot
msgmerge --update -s --no-location --no-wrap fr/maxr.po maxr.pot
msgmerge --update -s --no-location --no-wrap hu/maxr.po maxr.pot
msgmerge --update -s --no-location --no-wrap nl/maxr.po maxr.pot
msgmerge --update -s --no-location --no-wrap ru/maxr.po maxr.pot
msgmerge --update -s --no-location --no-wrap sl/maxr.po maxr.pot
