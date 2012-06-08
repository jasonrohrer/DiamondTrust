all: envelopeFront_withFonts.eps envelopeBack_withFonts.eps

envelopeFront_withFonts.eps:  envelopeFront.eps
	eps2eps -dEmbedAllFonts=true envelopeFront.eps envelopeFront_withFonts.eps

envelopeBack_withFonts.eps:  envelopeBack.eps
	eps2eps -dEmbedAllFonts=true envelopeBack.eps envelopeBack_withFonts.eps

#
# Map all skencil files to eps
#
%.eps: %.sk
	sk2ps $< $@