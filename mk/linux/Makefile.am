SUBDIRS= . src

maxr_DATA=
maxrdir=$(datadir)/maxr

install-data-hook:

	$(mkinstalldirs) $(DESTDIR)$(datadir)/maxr
	cp -a $(top_srcdir)/data/* $(DESTDIR)$(maxrdir)/

uninstall-hook:

	rm -rf $(DESTDIR)$(maxrdir) 

all-local:
	$(SHELL) $(top_srcdir)/buildinfo.sh $(datadir)/maxr

CLEANFILES = src/autoversion.h
