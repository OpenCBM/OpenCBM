# $Id: Makefile,v 1.1 2005-03-02 18:17:19 strik Exp $

include ../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

SGML = cbm4linux.sgml

TXT  = cbm4linux.txt
TEX  = cbm4linux.tex
DVI  = cbm4linux.dvi
INFO = cbm4linux.info
HTML = html/cbm4linux.html

%.txt: %.sgml
	sgml2txt $<

%.dvi: %.sgml
	sgml2latex -o dvi $<

%.info: %.sgml
	sgml2info $<
	perl -p -i -e 'if(m/^   \\input texinfo/){print "INFO-DIR-SECTION Miscellaneous\nSTART-INFO-DIR-ENTRY\n* cbm4linux: (cbm4linux).	Commodore serial IEC bus support.\nEND-INFO-DIR-ENTRY\n";}' $@

%.html: cbm4linux.sgml
	-[ ! -d html ] && mkdir html/
	cd html; sgml2html -s 2 ../$<; cd ..

all: $(TXT) $(DVI) $(INFO) $(HTML)

clean:
	rm -f $(TEX)

mrproper: clean
	rm -f $(DVI) $(INFO) $(TXT) html/*

install-files:
	gzip -c9 $(INFO) > $(INFODIR)/$(INFO).gz

install: install-files
	test -z `type -p mkinfodir` || mkinfodir || /bin/true

uninstall:
	rm -f $(INFODIR)/$(INFO).gz
