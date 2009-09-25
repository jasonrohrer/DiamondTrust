

GAME_GRAPHICS = \
testTexture.tga \
testTexture2.tga \
parentButton.tga \
childButton.tga \
button.tga \
angola_map.tga \
font8.tga \
font16.tga \
testDirectColor.tga \
test250Color.tga \
units_simple.tga \
activeUnitHalo.tga \
moveDot.tga \
diamond.tga \
bidPicker.tga \
bid.tga \
statsPanel.tga \
bribedMarker.tga \
miniDiamond.tga \
vault.tga \
salePicker.tga \




# prepend path
PATH_GAME_GRAPHICS = $(patsubst %,gameData/%,$(GAME_GRAPHICS))



all: ${PATH_GAME_GRAPHICS}
	cp languages/*.txt gameData


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