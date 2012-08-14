

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


all: 35.ps 40.ps 50.ps 55.ps 65.ps 80.ps 85.ps 100.ps 105.ps 400.ps


clean: 
	rm *.pdf *.ps *.dvi *.log *.aux
