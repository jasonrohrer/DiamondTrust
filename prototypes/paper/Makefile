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
	dvips -Ppdf -t letter -o $@ $<
%.pdf: %.ps
	ps2pdf $<


all: equipment.pdf

clean: 
	rm *.pdf *.ps *.dvi *.log *.aux


equipment.dvi: diamondSheet.eps moneySheet.eps map_clear.eps decisonSheet.eps