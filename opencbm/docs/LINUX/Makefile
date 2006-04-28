# $Id: Makefile,v 1.3 2006-04-28 12:51:09 strik Exp $

include ../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

SGML = cbm4linux.sgml

PDF  = cbm4linux.pdf
TXT  = cbm4linux.txt
WINTXT  = cbm4linux.win.txt
TEX  = cbm4linux.tex
DVI  = cbm4linux.dvi
INFO = cbm4linux.info
HTML = html/cbm4linux.html

%.win.txt: %.txt
	sed -e 's/.//g;' -e 's/\[[0-9]*m//g;' $< > $@
	unix2dos $@
	
%.txt: %.sgml
	sgml2txt $<

%.pdf: %.dvi
	dvipdf $<

%.dvi: %.sgml
	sgml2latex -o dvi $<

%.info: %.sgml
	sgml2info $<
	perl -p -i -e 'if(m/^   \\input texinfo/){print "INFO-DIR-SECTION Miscellaneous\nSTART-INFO-DIR-ENTRY\n* cbm4linux: (cbm4linux).	Commodore serial IEC bus support.\nEND-INFO-DIR-ENTRY\n";}' $@

%.html: cbm4linux.sgml
	-[ ! -d html ] && mkdir html/
	cd html; sgml2html -s 2 ../$<; cd ..

all: $(TXT) $(DVI) $(INFO) $(HTML) ${PDF}

clean:
	rm -f $(TEX)

mrproper: clean
	rm -f $(DVI) $(INFO) $(TXT) ${PDF} ${WINTXT} html/*
	rmdir html/

install-files:
	gzip -c9 $(INFO) > $(INFODIR)/$(INFO).gz

install: install-files
	test -z `type -p mkinfodir` || mkinfodir || /bin/true

uninstall:
	rm -f $(INFODIR)/$(INFO).gz
