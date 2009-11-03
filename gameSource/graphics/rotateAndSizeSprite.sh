tempFileA="$2"_temp.png
tempFileB="$2"_temp2.png

convert -rotate $1 -background black $2 $2_temp.png

w=`identify -format "%w" $tempFileA`
h=`identify -format "%h" $tempFileA`

targetW=`identify -format "%w" $2`
targetH=`identify -format "%h" $2`

extraW=$[$w - $targetW]
extraH=$[$h - $targetH]

xOffset=$[$extraW / 2]
yOffset=$[$extraH / 2]

convert -crop "$targetW"x"$targetH"+"$xOffset"+"$yOffset" $tempFileA $tempFileB

mv $tempFileB $tempFileA

convert -resize 32 $tempFileA $tempFileB

mv $tempFileB $tempFileA
