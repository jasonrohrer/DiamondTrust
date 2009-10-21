

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