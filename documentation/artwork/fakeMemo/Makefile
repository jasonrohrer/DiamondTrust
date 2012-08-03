

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
%.dvi: %.tex
	latex $<
%.ps: %.dvi
	dvips -Ppdf -t a4 -o $@ $<
%.pdf: %.ps
	ps2pdf $<


all: memo.pdf

sig.eps: sigUpRes.png
	convert sigUpRes.png sig.eps

memo.dvi: sig.eps

clean: 
	rm *.pdf *.ps *.dvi *.log *.aux
