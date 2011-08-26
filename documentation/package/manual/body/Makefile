# using T1 Times font, which doesn't work well with -P pdf
# fix it with the G0 switch

# dvips -E -P pdf -G0 -o ad2.eps ad2.dvi



# 
# Generic:
#
# Map all .cpp C++ and C files into .o object files
#
# $@   represents the name.o file
# $<   represents the name.cpp file
#
%.eps: %.sk
	sk2ps $< $@

# run latex twice to regen dependencies, like .toc file
%.dvi: %.tex
	latex $<
	latex $<

%.ps: %.dvi
	dvips -P pdf -G0 -o $@ $<
%.pdf: %.ps
	ps2pdf $<




all: manualBody.pdf

clean: 
	rm *.pdf *.ps *.eps *.dvi *.log *.aux *.toc printPageOrder


manualBody.dvi: pageGeometry.tex gamePhaseDiagram.tex

design.dvi: pageGeometry.tex

manualBodyFull.pdf: manualBody.pdf design.pdf
	gs -dBATCH -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=manualBodyFull.pdf manualBody.pdf design.pdf


printPageOrder: printPageOrder.cpp
	g++ -o printPageOrder printPageOrder.cpp


manualPrintBooklet.pdf:  BOOKLET_PAGE_ORDER = $(shell ./printPageOrder 47 24 )


manualPrintBooklet.pdf:  manualBodyFull.pdf printPageOrder
	pdfnup manualBodyFull.pdf --frame true --paper letterpaper --noautoscale true --outfile manualPrintBooklet.pdf --pages $(BOOKLET_PAGE_ORDER)