#
# Makefile for building the final report
#

%.pdf: %.dvi
	dvipdfm $*

%.dvi: %.tex
	latex $<
	latex $<
	latex $<

all: report

preview: report.pdf
	evince $<

report: report.pdf

clean:
	rm -rf *.dvi *.pdf *.aux *.log *.lof *.lot *.toc

