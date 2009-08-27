

TARGET_PLATFORM = TWL NITRO

SUBDIRS = 

DS_SOURCE = main.cpp

GAME_SOURCE = game.cpp
# append path
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


do-build:  $(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules