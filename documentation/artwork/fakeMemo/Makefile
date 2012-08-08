

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


all: memoA.pdf  memoB.pdf  memoC.pdf memoD.pdf

sig.eps: sigUpRes.png
	convert sigUpRes.png sig.eps

memoA.dvi: sig.eps memo.tex reA.txt textA.txt dateA.txt
	cp textA.txt textBody.tex
	cp reA.txt reText.tex
	cp dateA.txt dateText.tex
	latex -jobname=memoA memo.tex
	rm textBody.tex reText.tex dateText.tex

memoB.dvi: sig.eps memo.tex reB.txt textB.txt dateB.txt
	cp textB.txt textBody.tex
	cp reB.txt reText.tex
	cp dateB.txt dateText.tex
	latex -jobname=memoB memo.tex
	rm textBody.tex reText.tex dateText.tex

memoC.dvi: sig.eps memo.tex reC.txt textC.txt dateC.txt
	cp textC.txt textBody.tex
	cp reC.txt reText.tex
	cp dateC.txt dateText.tex
	latex -jobname=memoC memo.tex
	rm textBody.tex reText.tex dateText.tex

memoD.dvi: sig.eps memo.tex reD.txt textD.txt dateD.txt
	cp textD.txt textBody.tex
	cp reD.txt reText.tex
	cp dateD.txt dateText.tex
	latex -jobname=memoD memo.tex
	rm textBody.tex reText.tex dateText.tex



clean: 
	rm *.pdf *.ps *.dvi *.log *.aux
