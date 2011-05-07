

GAME_GRAPHICS = \
button.tga \
longButton.tga \
buttonLeftEnd.tga \
buttonRightEnd.tga \
buttonMiddle.tga \
angola_map.tga \
font8.tga \
font16_sans.tga \
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

JRI_CONVERTER_DIR = ../../minorGems/graphics/formats/jri/
JRI_CONVERTER = ${JRI_CONVERTER_DIR}tgaToJri


all: ${JRI_CONVERTER} gameData ${JRI_GAME_GRAPHICS}
	cp languages/*.txt gameData


gameData:
	mkdir gameData

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
gameData/%.tga: graphics/%.png
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
gameData/%.jri: gameData/%.tga
	${JRI_CONVERTER} $< $@
	rm $<



#graphics/jet_rotated.png: graphics/jet.png
#	cd graphics; ./rotateAndSizeSprite.sh jet.png jet_rotated.png; cd ..

graphics/%_rotated.png: graphics/%.png
	graphics/rotateAndSizeSprite.sh $< $@