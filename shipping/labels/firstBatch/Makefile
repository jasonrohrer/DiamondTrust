

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
#%.dvi: %.tex
#	latex $<
%.dvi: %.csv merge.tex template.tex templateUSA.tex merge.pl
	./merge.pl $< template.tex templateUSA.tex out.tex
	latex merge.tex
	mv merge.dvi $@
%.ps: %.dvi
	dvips -Ppdf -t letter -o $@ $<
%.pdf: %.ps
	ps2pdf $<


all: 35.pdf 40.pdf 50.pdf 55.pdf 65.pdf 80.pdf 85.pdf 100.pdf 105.pdf 400.pdf


allPages.pdf: 35.pdf 40.pdf 50.pdf 55.pdf 65.pdf 80.pdf 85.pdf 100.pdf 105.pdf 400.pdf
	gs \
  -o allPages.pdf \
  -sDEVICE=pdfwrite \
  -dPDFSETTINGS=/prepress \
   35.pdf 40.pdf 50.pdf 55.pdf 65.pdf 80.pdf 85.pdf 100.pdf 105.pdf 400.pdf


allPagesBut80.pdf: 35.pdf 40.pdf 50.pdf 55.pdf 65.pdf 85.pdf 100.pdf 105.pdf 400.pdf
	gs \
  -o allPagesBut80.pdf \
  -sDEVICE=pdfwrite \
  -dPDFSETTINGS=/prepress \
   35.pdf 40.pdf 50.pdf 55.pdf 65.pdf 85.pdf 100.pdf 105.pdf 400.pdf


clean: 
	rm *.pdf *.ps *.dvi *.log *.aux
