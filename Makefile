all: statement
statement: statement.pdf

statement.pdf: statement.tex
	pdflatex -enable-write18 statement.tex
	rm -f *.log *.aux *.out

clean:
	rm -f *.log *.aux *.out *.toc

delete: clean
	rm -f statement.pdf
