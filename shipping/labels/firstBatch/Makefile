

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
%.dvi: %.csv merge.tex
	cp $< in.csv
	latex merge.tex
	mv merge.dvi $@
	rm in.csv
%.ps: %.dvi
	dvips -Ppdf -t a4 -o $@ $<
%.pdf: %.ps
	ps2pdf $<


all: 35.pdf 40.pdf 105.pdf


clean: 
	rm *.pdf *.ps *.dvi *.log *.aux
