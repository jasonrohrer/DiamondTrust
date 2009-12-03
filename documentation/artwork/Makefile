

all: logo_withFonts.eps

logo_withFonts.eps:  logo.eps
	eps2eps -dEmbedAllFonts=true logo.eps logo_withFonts.eps

logo.eps: logo.sk
	sk2ps logo.sk logo.eps