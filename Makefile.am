ACLOCAL_AMFLAGS = -I m4 --install
AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = bootstrap LICENSE README.md
SUBDIRS = src test
doc_DATA = README.md

main: all
	@echo "------------ build done ----------"
	./src/aincrad
