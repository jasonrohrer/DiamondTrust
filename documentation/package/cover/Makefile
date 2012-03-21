all: coverText_withFonts.eps coverTextSecond_withFonts.eps

coverText_withFonts.eps:  coverText.eps
	eps2eps -dEmbedAllFonts=true coverText.eps coverText_withFonts.eps

coverText.eps: coverText.sk
	sk2ps coverText.sk coverText.eps


coverTextSecond_withFonts.eps:  coverTextSecond.eps
	eps2eps -dEmbedAllFonts=true coverTextSecond.eps coverTextSecond_withFonts.eps

coverTextSecond.eps: coverTextSecond.sk
	sk2ps coverTextSecond.sk coverTextSecond.eps