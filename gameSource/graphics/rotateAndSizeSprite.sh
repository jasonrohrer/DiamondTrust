#!/bin/bash

tempFileA="$1"_temp.png
tempFileB="$1"_temp2.png


fileNumber=0
netRotation=0
rotationDelta=11.25

until [ $fileNumber -gt 7 ]; do

	rotation=$(printf %.0f $netRotation)
	
	#echo "Using rotation $rotation"
	#echo "Net rotation $netRotation"
	convert -rotate $rotation -background black +repage $1 $tempFileA
	
	w=`identify -format "%w" $tempFileA`
	h=`identify -format "%h" $tempFileA`
	
	targetW=`identify -format "%w" $1`
	targetH=`identify -format "%h" $1`

	extraW=$[$w - $targetW]
	extraH=$[$h - $targetH]
	
	xOffset=$[$extraW / 2]
	yOffset=$[$extraH / 2]
	
	convert -crop "$targetW"x"$targetH"+"$xOffset"+"$yOffset" +repage $tempFileA $tempFileB

	val=1

	

	mv $tempFileB $tempFileA
	
	mogrify -filter hermite -resize 32 $tempFileA

	

    # threshold 0 finds all non-black pixels and makes them white
    # blur to expand a bit
    # threshold again to sharpen
	convert -threshold 0  -blur 0.12 -threshold 0 +repage $tempFileA temp_mask.png

	convert $tempFileA  temp_mask.png \
          +matte -compose CopyOpacity -composite \
          +repage temp_maskedImage.png

	convert temp_maskedImage.png -background \#00FF00 -flatten $tempFileA
	
	#if [ $fileNumber -eq $val ]; then
		#exit
	#fi

	rm temp_mask.png temp_maskedImage.png

	#convert color_test.png   -fill white
    #      -draw 'color 30,20 floodfill'
	#mogrify -fuzz 0 -fill green -draw 'color 1,1 replace' $tempFileA
	
	newFileName=temp_"$fileNumber".png
	mv $tempFileA $newFileName
	
	mogrify -border 1 -bordercolor red $newFileName
	mogrify -crop 32x33+1+1 +repage $newFileName
	echo "Done with $newFileName, angle $rotation"

	let fileNumber=fileNumber+1

	newRotation=`echo "$netRotation + $rotationDelta" | bc`
	netRotation=$newRotation
done 


montage temp_[0-7].png  -tile 1x8  -geometry 32x33 -background red temp_tiled.png

# trim off last line of red at bottom
mogrify -crop 32x263+0+0 +repage temp_tiled.png

mv temp_tiled.png $2

rm temp_*.png
