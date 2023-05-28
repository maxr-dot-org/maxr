Maxr uses gettext po files for translation ([quick tutorial](https://www.labri.fr/perso/fleury/posts/programming/a-quick-gettext-tutorial.html)).
Maxr doesn't use libintl but [spiritless_po](https://github.com/oo13/spiritless_po) which uses directly po files instead of mo files


- For extraction we use:

	```
	xgettext --c++ -o data/languages/maxr.pot -s -k -ki18n -kplural:1,1 --no-location `find . -name "*.cpp"` `find . -name "*.h"`
	```

- For new LANG (change with your own), use

	```
	msginit --input=data/languages/maxr.pot --locale=LANG --output=data/languages/LANG/maxr.po
	```

- To update LANG, use

	```
	msgmerge --update -s --no-location data/languages/LANG/maxr.po data/languages/maxr.pot
	```

Current languages are:

ca: Catalan
de: German
en: English
es: Spanish
fr: French
hu: Hungarian
nl: Dutch
ru: Russian
sl: Slovenian
