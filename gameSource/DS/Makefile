TWLSDK_PLATFORM = NITRO
#TWLSDK_PLATFORM = TWL
TWL_DEBUG = TRUE

#TARGET_PLATFORM = TWL NITRO
TARGET_PLATFORM = NITRO

# make invoked automatically in these dirs
# this makes our romfiles 
SUBDIRS = ..


MBP_DIR = $(ROOT)/build/demos/wireless_shared/mbp

DS_SOURCE = main.cpp measureChannel.cpp cloneBoot.cpp \
cloneBootChild.cpp $(MBP_DIR)/mbp.c



# NOTE:  if you see linker errors after adding files to ../makeDefs, try
#   touch Makefile
# To let build system know that object list has changed.

# for GAME_SOURCE definition
include ../makeDefs


MINOR_GEMS_SOURCE = \
util/stringUtils.cpp \


# prepend paths
PATH_GAME_SOURCE = $(patsubst %,../%,$(GAME_SOURCE))

PATH_MINOR_GEMS_SOURCE = $(patsubst %,../../../minorGems/%,$(MINOR_GEMS_SOURCE))



SRCS = $(DS_SOURCE) $(PATH_GAME_SOURCE) $(PATH_MINOR_GEMS_SOURCE)


TARGET_BIN = main.srl

MAKEROM_ROMROOT = ../gameData 
MAKEROM_ROMFILES = * 



INCDIR = .. ../../.. $(MBP_DIR)

# use defaults for these
# SRCDIR =
# LCFILE =


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

# force rebuild of targets if romfiles change
$(TARGETS): $(MAKEROM_ROMROOT)/$(MAKEROM_ROMFILES)


do-build:  $(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules
