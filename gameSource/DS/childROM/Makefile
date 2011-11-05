TWLSDK_PLATFORM = NITRO
#TWLSDK_PLATFORM = TWL
#TWL_DEBUG = TRUE
TWL_FINALROM = TRUE

#TARGET_PLATFORM = TWL NITRO
TARGET_PLATFORM = NITRO

# make invoked automatically in these dirs
# this makes our romfiles 
SUBDIRS = ../..


MBP_DIR = $(ROOT)/build/demos/wireless_shared/mbp

DS_SOURCE = ../main.cpp ../measureChannel.cpp ../cloneBoot.cpp \
../cloneBootChild.cpp $(MBP_DIR)/mbp.c



ROM_NAME = childROM


# NOTE:  if you see linker errors after adding files to ../makeDefs, try
#   touch Makefile
# To let build system know that object list has changed.

# for GAME_SOURCE definition
include ../../makeDefs


MINOR_GEMS_SOURCE = \
util/stringUtils.cpp \
graphics/formats/jri/jri.cpp \


# prepend paths
PATH_GAME_SOURCE = $(patsubst %,../../%,$(GAME_SOURCE))

PATH_MINOR_GEMS_SOURCE = $(patsubst %,../../../../minorGems/%,$(MINOR_GEMS_SOURCE))



SRCS = $(DS_SOURCE) $(PATH_GAME_SOURCE) $(PATH_MINOR_GEMS_SOURCE)


TARGET_BIN = $(ROM_NAME)_toSign.srl



INCDIR = ../.. ../../../.. $(MBP_DIR)


# since this is mulitboot child, and not cloneboot child, don't need
# special libsyscall files or LCFILE or ROM_SPEC file


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:   $(TARGETS)


#replaceLogo: replaceLogo.cpp
#	g++ -o replaceLogo replaceLogo.cpp

#bin/$(TWL_BUILDTYPE)/$(ROM_NAME)_toSign.srl: bin/$(TWL_BUILDTYPE)/$(ROM_NAME)_defaultLogo.srl replaceLogo rohrendo.bmp
#	ndstool -x bin/$(TWL_BUILDTYPE)/$(ROM_NAME)_defaultLogo.srl -9 arm9.bin
#	ndstool -c rohrendoDummyROM.nds -9 arm9.bin -o rohrendo.bmp
#	replaceLogo bin/$(TWL_BUILDTYPE)/$(ROM_NAME)_defaultLogo.srl bin/$(TWL_BUILDTYPE)/$(ROM_NAME)_toSign.srl rohrendoDummyROM.nds
#	rm arm9.bin
#	rm rohrendoDummyROM.nds


# need only post-sign rule for multiboot child rom

postsign:
	$(ATTACHSIGN) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN) \
		bin/$(TWL_BUILDTYPE)/$(ROM_NAME)_toSign.sgn  \
		bin/$(TWL_BUILDTYPE)/$(ROM_NAME).srl
	cp bin/$(TWL_BUILDTYPE)/$(ROM_NAME).srl ../../gameData



include	$(TWLSDK_ROOT)/build/buildtools/modulerules
