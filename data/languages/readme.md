Maxr uses gettext po files for translation ([quick tutorial](https://www.labri.fr/perso/fleury/posts/programming/a-quick-gettext-tutorial.html))

- For extraction we use:

	```
	xgettext --c++ -o data/languages/maxr.pot -s -k -ki18n -kplural:2,3 --no-location `find . -name "*.cpp"` `find . -name "*.h"`
	```

- For new LANG (change with your own), use

	```
	msginit --input=data/languages/maxr.pot --locale=LANG --output=data/languages/LANG/maxr.po
	```

	Maxr doesn't auto detect new languages, so it would require to also add it in preference selection manually


- To update LANG, use

	```
	msgmerge --update -s --no-location data/languages/LANG/maxr.po data/languages/maxr.pot
	```


- And the final step to binarize result for LANG 

	```
	msgfmt --output-file=data/languages/LANG/LC_MESSAGES/maxr.mo data/languages/LANG/maxr.po
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
