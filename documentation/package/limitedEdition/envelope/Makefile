all: envelopeFront_withFonts.eps envelopeBack_withFonts.eps logo_withFonts.eps envelopeBack.png envelopeFront.png

envelopeBack.eps: logo.eps

#envelopeFront_withFonts.eps:  envelopeFront.eps
#	eps2eps -dEmbedAllFonts=true envelopeFront.eps envelopeFront_withFonts.eps

#envelopeBack_withFonts.eps:  envelopeBack.eps
#	eps2eps -dEmbedAllFonts=true envelopeBack.eps envelopeBack_withFonts.eps


envelopeBack.png: envelopeBack.eps
	gs -dSAFER -dBATCH -dNOPAUSE -sDEVICE=pngmono -sOutputFile=envelopeBack.png -r1200 -dDEVICEWIDTHPOINTS=432 -dDEVICEHEIGHTPOINTS=648 envelopeBack.eps


envelopeFront.png: envelopeFront.eps
	gs -dSAFER -dBATCH -dNOPAUSE -sDEVICE=pngmono -sOutputFile=envelopeFront.png -r1200 -dDEVICEWIDTHPOINTS=432 -dDEVICEHEIGHTPOINTS=648 envelopeFront.eps


#
# Map all skencil files to eps
#
%.eps: %.sk
	sk2ps $< $@
%_withFonts.eps:  %.eps
	eps2eps -dEmbedAllFonts=true $< $@
