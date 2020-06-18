.PHONY: mrproper install uninstall install-files

mrproper: clean
ifneq "$(words $(INC))" "0"
	rm -f $(INC)
endif
ifneq "$(words $(EXTRA_A65_INC))" "0"
	rm -f $(EXTRA_A65_INC)
endif
	rm -f *~ LINUX/*~ WINDOWS/*~

install-files: $(PROG)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 -s $(PROG) $(DESTDIR)$(BINDIR)
ifneq "$(words $(MAN1))" "0"
	install -d $(DESTDIR)$(MANDIR)
	install -m 644    $(MAN1) $(DESTDIR)$(MANDIR)
endif
ifneq "$(words $(LINKS))" "0"
	install -d $(DESTDIR)$(BINDIR)
	for l in $(LINKS); do ln -sf $(PROG) $(DESTDIR)$(BINDIR)/$$l; done
	install -d $(DESTDIR)$(MANDIR)
	for l in $(LINKS); do ln -sf $(MAN1) $(DESTDIR)$(MANDIR)/$$l.1; done
endif

install: install-files

uninstall:
#	rm -f $(DESTDIR)$(BINDIR)/$(PROG)
	for P in $(PROG); do rm -f $(DESTDIR)$(BINDIR)/$$P; done
ifneq "$(words $(MAN1))" "0"
	rm -f $(DESTDIR)$(MANDIR)/$(MAN1) $(DESTDIR)$(MANDIR)/$(MAN1).gz
endif
ifneq "$(words $(LINKS))" "0"
	for l in $(LINKS); do rm -f $(DESTDIR)$(BINDIR)/$$l $(DESTDIR)$(MANDIR)/$$l.1{,.gz}; done
endif
	-rmdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(MANDIR)

