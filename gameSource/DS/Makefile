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
graphics/formats/jri/jri.cpp \


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


# clone boot template
LCFILE_TEMPLATE	= $(NITRO_SPECDIR)/ARM9-TS-cloneboot-C.lcf.template


# FIXME (for final signing only):  

# need libsyscall definitions

# need presign and post-sign rules



include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:  ../gameData/icon.nbfc ../gameData/icon.nbfp $(TARGETS) 


# force rebuild of targets if romfiles change
$(TARGETS): $(MAKEROM_ROMROOT)/$(MAKEROM_ROMFILES)




../gameData/icon.nbfc ../gameData/icon.nbfp: icon.bmp
	$(NTEXCONV) -no -bg -bgb -bgnc -f palette16 -tc=255,0,0 icon.bmp
	mv icon.nbfc icon.nbfp ../gameData

icon.bmp: icon.png
	convert icon.png icon.bmp


include	$(TWLSDK_ROOT)/build/buildtools/modulerules
