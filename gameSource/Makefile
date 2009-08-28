

GAME_GRAPHICS = \
gameData/testTexture.tga \




all: ${GAME_GRAPHICS}


clean:
	rm -f ${GAME_GRAPHICS}


# 
# Generic:
#
# Map all png files into .tga files
#
# $@   represents the name.tga file
# $<   represents the name.png file
#
gameData/%.tga: %.png
	convert $< $@