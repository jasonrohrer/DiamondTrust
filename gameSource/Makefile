

GAME_GRAPHICS = \
button.tga \
longButton.tga \
buttonLeftEnd.tga \
buttonRightEnd.tga \
buttonMiddle.tga \
helpToggle.tga \
angola_map.tga \
font8.tga \
font16_jcr.tga \
font16_handwriting2.tga \
playerUnits.tga \
enemyUnits.tga \
inspectorUnit.tga \
unit_arm_map.tga \
unit_arm_sourceMap.tga \
unit_noArm_mask.tga \
activeUnitHalo.tga \
diamond.tga \
diamondBorder.tga \
diamondBorder2.tga \
bidPicker.tga \
pickerBorder.tga \
bid.tga \
statsPanel.tga \
datePanel.tga \
bribedMarker.tga \
miniDiamond.tga \
vault.tga \
salePicker.tga \
map_names.tga \
paper.tga \
jet_rotated16.tga \
piper_rotated16.tga \
puma_rotated16.tga \
angola_satellite.tga \
titleBlack16.tga \
rolodex_manilla16.tga \
playerPictures.tga \
enemyPictures.tga \
sellingPanel.tga \
satPhone.tga \
satPhoneLight.tga \
signal0.tga \
signal1.tga \
signal2.tga \
signal3.tga \
wirelessOn.tga \
greenBarSheet.tga \
aiLevelSheet.tga \
nintendoLogo.tga \
watch.tga \
watchHandVertical.tga \
watchHandHorizontal.tga \
watchHandDiagonalLeft.tga \
watchHandDiagonalRight.tga \


LANGUAGE_FILES_SOURCE = $(wildcard languages/*.txt)
# language file names without paths
LANGUAGE_FILES = $(subst languages/,,$(LANGUAGE_FILES_SOURCE))



# these are needed by a Clone Boot child before a connection is even 
# established
# So, we must compile them into the binary image as code.
# Then, we can delete the files themselves from the gameData directory
FILES_TO_COMPILE_IN = \
${GAME_GRAPHICS} \
${LANGUAGE_FILES} \




# not ideal!
#playerPictures.tga \
#enemyPictures.tga \

# this change is less than ideal! \
#paper.tga \

# this change is less than ideal! \
#angola_satellite.tga \

# this change cannot be seen \
# though change to 16-color version isn't so great
#title.tga \
#title256.tga \
# dropping down to 4 colors changes the appearance completely
# title16.tga \

# this change cannot be seen, but change to 4-color not ideal
#rolodex_manilla.tga \
#rolodex_manilla16.tga \

# this change cannot be seen
#jet_rotated.tga \
#piper_rotated.tga \
#puma_rotated.tga \



# prepend path
PATH_GAME_GRAPHICS = $(patsubst %,gameData/%,$(GAME_GRAPHICS))

JRI_GAME_GRAPHICS = ${PATH_GAME_GRAPHICS:.tga=.jri}


# add path
PATH_FILES_TO_COMPILE_IN = $(patsubst %,gameDataSource/%,$(FILES_TO_COMPILE_IN))
# change to JRI files instead of TGA versions
JRI_FILES_TO_COMPILE_IN = ${PATH_FILES_TO_COMPILE_IN:.tga=.jri}



JRI_CONVERTER_DIR = ../../minorGems/graphics/formats/jri/
JRI_CONVERTER = ${JRI_CONVERTER_DIR}tgaToJri




all: ${JRI_CONVERTER} gameDataSource gameData dataFiles.cpp


gameData:
	mkdir gameData

gameDataSource:
	mkdir gameDataSource

clean:
	rm -f ${PATH_GAME_GRAPHICS}



${JRI_CONVERTER}:
	cd ${JRI_CONVERTER_DIR}; ./compileConverters


# 
# Generic:
#
# Map all png files into .tga files
#
# $@   represents the name.tga file
# $<   represents the name.png file
#
gameDataSource/%.tga: graphics/%.png
	convert $< $@


# 
# Generic:
#
# Map all .tga files into .jri files
# Remove .tga files.
#
# $@   represents the name.jri file
# $<   represents the name.tga file
#
gameDataSource/%.jri: gameDataSource/%.tga
	${JRI_CONVERTER} $< $@
	rm $<



#graphics/jet_rotated.png: graphics/jet.png
#	cd graphics; ./rotateAndSizeSprite.sh jet.png jet_rotated.png; cd ..

graphics/%_rotated.png: graphics/%.png
	graphics/rotateAndSizeSprite.sh $< $@


# don't rebuild these every time, unless needed
# this special target is described here: 
# http://www.gnu.org/s/hello/manual/make/Chained-Rules.html
#.INTERMEDIATE: ${JRI_FILES_TO_COMPILE_IN} 


dataFiles.cpp: ${JRI_FILES_TO_COMPILE_IN} compileFilesIn.pl
	./compileFilesIn.pl dataFiles.cpp ${JRI_FILES_TO_COMPILE_IN}




# 
# Generic:
#
# Map all language files into gameData dir files
#
# $@   represents the gameData file
# $<   represents the languages file
#
gameDataSource/%.txt: languages/%.txt
	cp $< $@
