SUBDIRS= . src

maxr_DATA=
maxrdir=$(DESTDIR)$(datadir)/maxr

install-data-hook:

	$(mkinstalldirs) $(maxrdir)
	cp -a $(top_srcdir)/data/* $(maxrdir)/

uninstall-hook:

	rm -rf $(maxrdir) 

all-local:
	$(SHELL) $(top_srcdir)/buildinfo.sh $(datadir)/maxr

CLEANFILES = src/autoversion.h
