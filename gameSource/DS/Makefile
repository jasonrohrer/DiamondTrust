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



ROM_SPEC  =  ROM-TS.rsf


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


# (for final signing only):  

# need libsyscall definitions

# get a replacement from Nintendo (via publisher)
LIBSYSCALL			 = ./etc/libsyscall.a
# okay to use the one for the child from the SDK, even in a final build
# find out about this!
LIBSYSCALL_CHILD	 = ./etc/libsyscall_child.bin





include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:  ../gameData/icon.nbfc ../gameData/icon.nbfp $(TARGETS) 


# need presign and post-sign rules


presign:
	$(EMUCHILD) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN) \
		$(LIBSYSCALL_CHILD) \
		bin/$(TWL_BUILDTYPE)/sign.srl

postsign:
	$(ATTACHSIGN) \
		bin/$(TWL_BUILDTYPE)/$(TARGET_BIN) \
		bin/$(TWL_BUILDTYPE)/sign.sgn  \
		bin/$(TWL_BUILDTYPE)/mainSigned.srl




# force rebuild of targets if romfiles change
$(TARGETS): $(MAKEROM_ROMROOT)/$(MAKEROM_ROMFILES)




../gameData/icon.nbfc ../gameData/icon.nbfp: icon.bmp
	$(NTEXCONV) -no -bg -bgb -bgnc -f palette16 -tc=255,0,0 icon.bmp
	mv icon.nbfc icon.nbfp ../gameData

icon.bmp: icon.png
	convert icon.png icon.bmp


banner.bnr: banner.bsf banner.nbfc banner.nbfp
	$(MAKEBANNER) banner.bsf banner.bnr

banner.nbfc banner.nbfp: icon.bmp
	$(NTEXCONV) -no -bg -bgb -bgnc -f palette16 -tc=255,0,0 icon.bmp
	mv icon.nbfc banner.nbfc
	mv icon.nbfp banner.nbfp


# make sure ROM is rebuilt if banner file changes
# touch ROM spec file to force ROM to be rebuilt
$(ROM_SPEC):  banner.bnr
	touch $(ROM_SPEC)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules
