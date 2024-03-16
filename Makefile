GROUP_NUMBER=1

group_dir=g${GROUP_NUMBER}

all: statement
statement: statement.pdf

reports: report_template report_serial report_omp report_mpi

report_template: 
	cd src && \
		pdflatex -enable-write18 report_template.tex && \
		rm -f *.log *.aux *.out

report_%: 
	# Runs twice on purpose (to resolve back references)
	cd src/$*/report && \
		pdflatex -enable-write18 report.tex && \
		pdflatex -enable-write18 report.tex && \
		rm -f *.log *.aux *.out

zips: clean delete_zips check_src_structure zip_all zip_serial zip_omp zip_mpi
	mkdir -p delivery
	mv ${group_dir}*.zip delivery

zip_all:
	rm ${group_dir}.zip || true
	cp -r src ${group_dir}
	zip -r ${group_dir}.zip ${group_dir}	
	rm -r ${group_dir}

zip_%:
	rm ${group_dir}$*.zip || true
	cp -r src/$* ${group_dir}$*
	zip -r ${group_dir}$*.zip ${group_dir}$*
	rm -r ${group_dir}$*

check_src_structure:
	test -d ./src/serial || exit 1
	test -d ./src/omp || exit 1
	test -d ./src/mpi || exit 1

	test -e ./src/serial/Makefile || exit 1
	test -e ./src/omp/Makefile || exit 1
	test -e ./src/mpi/Makefile || exit 1

	test -e ./src/serial/report/report.pdf || exit 1
	test -e ./src/omp/report/report.pdf || exit 1
	test -e ./src/mpi/report/report.pdf || exit 1

statement.pdf: statement.tex
	pdflatex -enable-write18 statement.tex
	rm -f *.log *.aux *.out

clean:
	rm -f *.log *.aux *.out *.toc
	rm -f src/*.log src/*.aux src/*.out src/*.toc src/*.pdfsync
	rm -r ${group_dir} || true

delete: clean

delete_zips:
	rm -rf delivery
