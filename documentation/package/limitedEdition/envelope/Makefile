all: envelopeFront_withFonts.eps

envelopeFront_withFonts.eps:  envelopeFront.eps
	eps2eps -dEmbedAllFonts=true envelopeFront.eps envelopeFront_withFonts.eps

envelopeFront.eps: envelopeFront.sk
	sk2ps envelopeFront.sk envelopeFront.eps
