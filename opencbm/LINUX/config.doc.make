# Find out if we should use linuxdoc or sgml2txt/sgml2latex/sgml2info/sgml2html
#
LINUXDOCTXT = ${shell for c in linuxdoc sgml2txt; do test ! -z `which $$c` && test -f `which $$c` && echo $$c; done | head -n 1}
ifeq "${LINUXDOCTXT}" ""
  $(error You must have linuxdoc or sgmltools installed. Check config.make)
else
 ifeq "${LINUXDOCTXT}" "linuxdoc"
  LINUXDOCLATEX=${LINUXDOCTXT}
  LINUXDOCINFO=${LINUXDOCTXT}
  LINUXDOCHTML=${LINUXDOCTXT}

  LINUXDOCTXTPARAM=-B txt
  LINUXDOCLATEXPARAM=-B latex
  LINUXDOCINFOPARAM=-B info
  LINUXDOCHTMLPARAM=-B html
 else
  LINUXDOCLATEX=sgml2latex
  LINUXDOCINFO=sgml2info
  LINUXDOCHTML=sgml2html

  LINUXDOCTXTPARAM=
  LINUXDOCLATEXPARAM=
  LINUXDOCINFOPARAM=
  LINUXDOCHTMLPARAM=
 endif
endif
