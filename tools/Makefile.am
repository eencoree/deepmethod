
if ENABLE_MPI_BUILD_TARGET
MPI_BUILD_TARGET = dpdeepctl_mpi
else
MPI_BUILD_TARGET =
endif

bin_PROGRAMS = \
	dpdeepctl $(MPI_BUILD_TARGET)

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

dpdeepctl_mpi_SOURCES = $(dpdeepctl_SOURCES)

dpdeepctl_mpi_LDADD = \
	$(MPI_CLIBS) \
	$(DEEP_LIBS) \
	$(top_builddir)/libdeep/libdeep_mpi.la \
	$(top_builddir)/libxmod/libxmod.la

dpdeepctl_mpi_CFLAGS = \
	-DMPIZE \
	$(dpdeepctl_CFLAGS) \
	$(MPI_CFLAGS)

## File created by the gnome-build tools
