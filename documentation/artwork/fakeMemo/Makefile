

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

memoA.dvi: sig.eps memo.tex reA.txt textA.txt
	cp textA.txt textBody.tex
	cp reA.txt reText.tex
	latex -jobname=memoA memo.tex
	rm textBody.tex reText.tex

memoB.dvi: sig.eps reB.txt textB.txt
	cp textB.txt textBody.tex
	cp reB.txt reText.tex
	latex -jobname=memoB memo.tex
	rm textBody.tex reText.tex

memoC.dvi: sig.eps reC.txt textC.txt
	cp textC.txt textBody.tex
	cp reC.txt reText.tex
	latex -jobname=memoC memo.tex
	rm textBody.tex reText.tex



clean: 
	rm *.pdf *.ps *.dvi *.log *.aux
