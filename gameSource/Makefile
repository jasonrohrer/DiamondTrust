

GAME_GRAPHICS = \
testTexture.tga \
testTexture2.tga \
parentButton.tga \
childButton.tga \
angola_map.tga \
font8.tga \
font16.tga \
testDirectColor.tga \
test250Color.tga \



# prepend path
PATH_GAME_GRAPHICS = $(patsubst %,gameData/%,$(GAME_GRAPHICS))



all: ${PATH_GAME_GRAPHICS}


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