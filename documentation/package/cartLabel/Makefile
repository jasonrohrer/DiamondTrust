all: labelText_withFonts.eps

labelText_withFonts.eps:  labelText.eps
	eps2eps -dEmbedAllFonts=true labelText.eps labelText_withFonts.eps

labelText.eps: labelText.sk
	sk2ps labelText.sk labelText.eps