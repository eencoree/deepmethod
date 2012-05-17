
bin_PROGRAMS = \
	dpdeepctl

dpdeepctl_SOURCES = \
	dpdeepctl.c\
	dpdeepctl.h \
	xmtranslate.c \
	xmtranslate.h

dpdeepctl_CFLAGS = \
	-I$(top_builddir)/libdeep\
	-I$(top_builddir)/libxmod

dpdeepctl_LDFLAGS = \
	-lm

dpdeepctl_LDADD = \
	$(DEEP_LIBS) \
	$(top_builddir)/libdeep/libdeep.la \
	$(top_builddir)/libxmod/libxmod.la

AM_CPPFLAGS = \
	$(DEEP_CFLAGS)

## File created by the gnome-build tools
