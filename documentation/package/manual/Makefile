all: manualCoverText_withFonts.eps

manualCoverText_withFonts.eps:  manualCoverText.eps
	eps2eps -dEmbedAllFonts=true manualCoverText.eps manualCoverText_withFonts.eps

manualCoverText.eps: manualCoverText.sk
	sk2ps manualCoverText.sk manualCoverText.eps