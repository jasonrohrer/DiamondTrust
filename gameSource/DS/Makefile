TWLSDK_PLATFORM = NITRO
TWL_DEBUG = TRUE

TARGET_PLATFORM = TWL NITRO

# make invoked automatically in these dirs
# this makes our romfiles 
SUBDIRS = ..


DS_SOURCE = main.cpp

GAME_SOURCE = \
game.cpp \
tga.cpp \

# prepend path
PATH_GAME_SOURCE = $(patsubst %,../%,$(GAME_SOURCE))


SRCS = $(DS_SOURCE) $(PATH_GAME_SOURCE)

TARGET_BIN = main.srl

MAKEROM_ROMROOT = ../gameData 
MAKEROM_ROMFILES = * 



INCDIR = ..

# use defaults for these
# SRCDIR =
# LCFILE =


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

# force rebuild of targets if romfiles change
$(TARGETS): $(MAKEROM_ROMROOT)/$(MAKEROM_ROMFILES)

do-build:  $(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules