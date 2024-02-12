TX    = $(wildcard *.tex)

all: statement.pdf
.PHONY: all

statement.pdf: $(TX)
	pdflatex -enable-write18 statement.tex
	rm -f *.log *.aux *.out

clean:
	rm -f *.log *.aux *.out *.toc

delete:
	rm -f statement.pdf
