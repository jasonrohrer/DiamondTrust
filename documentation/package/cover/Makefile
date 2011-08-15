all: coverText_withFonts.eps

coverText_withFonts.eps:  coverText.eps
	eps2eps -dEmbedAllFonts=true coverText.eps coverText_withFonts.eps

coverText.eps: coverText.sk
	sk2ps coverText.sk coverText.eps