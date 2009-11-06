

GAME_GRAPHICS = \
testTexture.tga \
testTexture2.tga \
parentButton.tga \
childButton.tga \
button.tga \
longButton.tga \
angola_map.tga \
font8.tga \
font16_sans.tga \
testDirectColor.tga \
test250Color.tga \
units_simple.tga \
units.tga \
unit_waving.tga \
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
unitInfoPanel.tga \
bribedMarker.tga \
miniDiamond.tga \
vault.tga \
salePicker.tga \
map_names.tga \
paper.tga \
jet_rotated.tga \
piper_rotated.tga \
jeep_rotated.tga \
puma_rotated.tga \
angola_satellite.tga \
title.tga \
rolodex.tga \

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