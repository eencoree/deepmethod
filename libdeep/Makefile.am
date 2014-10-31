
if ENABLE_MPI_BUILD_TARGET
MPI_BUILD_LIBTARGET = libdeep_mpi.la
else
MPI_BUILD_LIBTARGET =
endif

lib_LTLIBRARIES = \
	libdeep.la $(MPI_BUILD_LIBTARGET)

libdeep_la_SOURCES = \
	dpindivid.c \
	dpindivid.h \
	dppopulation.c \
	dppopulation.h \
	dpmutation.c \
	dpmutation.h \
	dprecombination.c \
	dprecombination.h \
	dpevaluation.c \
	dpevaluation.h \
	dpdeep.c \
	dpdeep.h \
	dpopt.c \
	dpopt.h \
	dptarget.c \
	dptarget.h \
	dploop.c \
	dploop.h \
	dpsettings.c \
	dpsettings.h \
	dposda.c \
	dposda.h

libdeep_la_LIBADD = \
	$(DEEP_LIBS)

AM_CPPFLAGS = \
	$(DEEP_CFLAGS)

libdeep_mpi_la_SOURCES = $(libdeep_la_SOURCES)

libdeep_mpi_la_CFLAGS = \
	-DMPIZE \
	$(libdeep_la_CFLAGS) \
	$(MPI_CFLAGS)

## File created by the gnome-build tools

