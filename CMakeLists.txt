cmake_minimum_required(VERSION 3.15)
project(deepmethod C)

set(CMAKE_C_STANDARD 11)

## Required for FindIntltool module
#set(GETTEXT_PACKAGE ${PROJECT_NAME})
#set(GETTEXT_PO_DIR ${CMAKE_SOURCE_DIR}/po)

include_directories(.)
include_directories(libdeep)
include_directories(libxmod)
include_directories(tools)

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(GLIB2 REQUIRED glib-2.0>=2.36.0)
INCLUDE_DIRECTORIES(${GLIB2_INCLUDE_DIRS})

ADD_COMPILE_DEFINITIONS(DEEPMETHOD_LOCALEDIR=${CMAKE_INSTALL_PREFIX}/share/locale)

add_executable(deepmethod
        libdeep/dpdeep.c
        libdeep/dpdeep.h
        libdeep/dpevaluation.c
        libdeep/dpevaluation.h
        libdeep/dpindivid.c
        libdeep/dpindivid.h
        libdeep/dploop.c
        libdeep/dploop.h
        libdeep/dpmutation.c
        libdeep/dpmutation.h
        libdeep/dpopt.c
        libdeep/dpopt.h
        libdeep/dposda.c
        libdeep/dposda.h
        libdeep/dppopulation.c
        libdeep/dppopulation.h
        libdeep/dprecombination.c
        libdeep/dprecombination.h
        libdeep/dpsettings.c
        libdeep/dpsettings.h
        libdeep/dptarget.c
        libdeep/dptarget.h
        libxmod/xmmodel.c
        libxmod/xmmodel.h
        tools/deepmethod.c
        tools/deepmethod.h
        tools/xmtranslate.c
        tools/xmtranslate.h
        config.h)

TARGET_LINK_LIBRARIES(deepmethod  ${GLIB2_LIBRARIES} m gio-2.0 gobject-2.0)
LINK_DIRECTORIES(${GLIB2_LIBRARY_DIRS})