# $Id: Makefile,v 1.10 2009-12-13 12:04:18 cnvogelg Exp $

RELATIVEPATH=../
include ${RELATIVEPATH}LINUX/config.make

.PHONY: all clean mrproper install uninstall install-files doc install-files-doc install--doc

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
	${LINUXDOCTXT} ${LINUXDOCTXTPARAM} $<

%.pdf: %.ps
	ps2pdf $<

%.ps: %.dvi
	dvips -o $*.ps $<

%.dvi: %.sgml
	${LINUXDOCLATEX} ${LINUXDOCLATEXPARAM} -o dvi $<

%.info: %.sgml
	${LINUXDOCINFO} ${LINUXDOCINFOPARAM} $<
	perl -p -i -e 'if(m/^   \\input texinfo/){print "INFO-DIR-SECTION Miscellaneous\nSTART-INFO-DIR-ENTRY\n* opencbm: (opencbm).	Commodore serial IEC bus support.\nEND-INFO-DIR-ENTRY\n";}' $@

%.html: opencbm.sgml
	([ ! -d html ] && mkdir html/) || true
	cd html; ${LINUXDOCHTML} ${LINUXDOCHTMLPARAM} -s 2 ../$<; cd ..

all: $(TXT) $(DVI) $(INFO) $(HTML) ${PDF}

doc: all

install-doc: install

install-files-doc: install-files

clean:
	rm -f $(TEX)

mrproper: clean
	rm -f $(DVI) $(INFO) $(TXT) ${PDF} ${WINTXT} html/* || true
	([ -d html/ ] && rmdir html/) || true

install-files:
	gzip -c9 $(INFO) > $(DESTDIR)$(INFODIR)/$(INFO).gz

install: install-files
	test -z `type -p mkinfodir` || mkinfodir || /bin/true

uninstall:
	rm -f $(DESTDIR)$(INFODIR)/$(INFO).gz
