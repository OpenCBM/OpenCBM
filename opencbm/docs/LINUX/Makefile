# $Id: Makefile,v 1.5 2006-05-13 16:51:50 strik Exp $

include ../LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files

SGML = opencbm.sgml

PDF  = opencbm.pdf
TXT  = opencbm.txt
WINTXT  = opencbm.win.txt
TEX  = opencbm.tex
DVI  = opencbm.dvi
INFO = opencbm.info
HTML = html/opencbm.html

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
	perl -p -i -e 'if(m/^   \\input texinfo/){print "INFO-DIR-SECTION Miscellaneous\nSTART-INFO-DIR-ENTRY\n* opencbm: (opencbm).	Commodore serial IEC bus support.\nEND-INFO-DIR-ENTRY\n";}' $@

%.html: opencbm.sgml
	([ ! -d html ] && mkdir html/) || true
	cd html; sgml2html -s 2 ../$<; cd ..

all: $(TXT) $(DVI) $(INFO) $(HTML) ${PDF}

clean:
	rm -f $(TEX)

mrproper: clean
	rm -f $(DVI) $(INFO) $(TXT) ${PDF} ${WINTXT} html/* || true
	([ -d html/ ] && rmdir html/) || true

install-files:
	gzip -c9 $(INFO) > $(INFODIR)/$(INFO).gz

install: install-files
	test -z `type -p mkinfodir` || mkinfodir || /bin/true

uninstall:
	rm -f $(INFODIR)/$(INFO).gz
