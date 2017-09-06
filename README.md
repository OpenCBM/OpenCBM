# OpenCBM
Win 7/8/10, and Linux/i386/AMD64 kernel driver and development library to control serial CBM devices, such as the Commodore 1541 disk drive, connected to the PC's parallel port via a XM1541 or XA1541 cable. Fast disk copier included. Successor of cbm4linux. Also supports the XU1541 and the XUM1541 devices (a.k.a. "ZoomFloppy").

## What is OpenCBM?

The popular Commodore 8-bit home-computers like the C-64 and the VIC-20 are using a custom serial bus to talk to attached devices (disk drive, printer). This proprietary serial bus protocol is not natively supported by modern hard- or software.

OpenCBM provides an interface to this so-called IEC bus at the level of simple TALK and LISTEN commands, similar to the one provided by the Commodore kernel routines. Additionally, some higher and lower level bus control is available as well, allowing for full control of the bus.

The CBM serial devices are connected to the PC either to the parallel port via an XM1541 or XA1541 cable and, optionally, an XP1541 or XP1571 add-on cable. Alternatively, more modern USB cable solutions like XU1541 or XUM1541 (a.k.a. ZoomFloppy) are supported.

OpenCBM has a plugin concept which allows to additionally add custom build cables.

OpenCBM can be used on PCs on Linux and Windows (all cables). Additioanlly, USB based cables are supported on FreeBSD and on Mac OS X.

## Supported Operating Systems

OpenCBM supports the following operating systems:

* For USB based cables: Any Linux, FreeBSD or MacOS X variant that support libusb-0.1 should be supported. Linux, FreeBSD and Mac OS X have been explicitly tested.
* For parallel port based cables: Linux 3.x and 2.6 variants. 2.0, 2.2 and 2.4 might still work, but have not been tested for ages. For Linux, i386 and AMD64 architectures are supported.
* For parallel port based as well as USB based cables: Windows NT 4.0, 2000, XP and Server 2003, Vista, 7 and 8. For USB based cables, NT 4.0 is not supported, though. The i386 architecture a.k.a "x86" ("32 bit") is fully supported; additionally, 64 bit Windows ("x64", "x86_64") versions are supported. Itanium-based Windows ("iA64") are not supported, though.

##  Supported CBM hardware

Currently, opencbm supports the following CBM devices:

* VIC 1541, VIC1540 (all variants, including clones)
* VIC 1570, VIC 1571 (including the 1571CR and the 1571 inside of a C128DCR)
* VIC 1581 (not with d64copy ( d64copy), not with cbmformat ( cbmformat) or cbmforng ( cbmforng))
* other CBM IEC drives, printers, and compatibles (only with cbmctrl ( cbmctrl))
* VIC 8250, 8050, 4040, 2031, SFD 1001, and possibly other IEEE drives with an IEC to IEEE converter (for example, IEC2IEEE from Jochen Adler, cf. http://www.nlq.de/, or with a ZoomFloppy extension that lets you use IEEE devices directly.
* VIC 1530 (a.k.a. C2N) tape device, and VIC 1531 with an appropriate adapter; both are supported by ZoomTape, only.

## Further reading

The current manual, including installation instructions, can be read online at http://opencbm.trikaliotis.net/.

A Doxygen output of the sources (for developers) (still work-in-progress) can be found at http://opencbm.trikaliotis.net/doxygen/.

The mailing lists of OpenCBM can be found at https://sourceforge.net/p/opencbm/mailman/.

Bug tracker are available at https://sourceforge.net/p/opencbm/_list/tickets.

## Contributions

We explicitly welcome outside contributors. If you feel like you can add to the projects, feel free to ask.


## Maintainers

[@spiro-trikaliotis](https://github.com/spiro-trikaliotis),
[@go4retro](https://github.com/go4retro).

## License

OpenCBM is published under the GPLv2.
