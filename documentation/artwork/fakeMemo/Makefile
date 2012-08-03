

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


all: memoA.pdf  memoB.pdf  memoC.pdf

sig.eps: sigUpRes.png
	convert sigUpRes.png sig.eps

memoA.dvi: sig.eps memo.tex
	cp textA.txt textBody.tex
	latex -jobname=memoA memo.tex
	rm textBody.tex

memoB.dvi: sig.eps
	cp textB.txt textBody.tex
	latex -jobname=memoB memo.tex
	rm textBody.tex

memoC.dvi: sig.eps
	cp textC.txt textBody.tex
	latex -jobname=memoC memo.tex
	rm textBody.tex



clean: 
	rm *.pdf *.ps *.dvi *.log *.aux
