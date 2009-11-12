

GAME_GRAPHICS = \
parentButton.tga \
childButton.tga \
button.tga \
longButton.tga \
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
paper16.tga \
jet_rotated16.tga \
piper_rotated16.tga \
puma_rotated16.tga \
angola_satellite.tga \
title16.tga \
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




all: ${PATH_GAME_GRAPHICS} gameData
	cp languages/*.txt gameData


gameData:
	mkdir gameData

clean:
	rm -f ${PATH_GAME_GRAPHICS}


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



#graphics/jet_rotated.png: graphics/jet.png
#	cd graphics; ./rotateAndSizeSprite.sh jet.png jet_rotated.png; cd ..

graphics/%_rotated.png: graphics/%.png
	graphics/rotateAndSizeSprite.sh $< $@