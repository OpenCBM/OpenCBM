README.TXT for the mnib utilities 0.36 (08 November 2005)

mnib, n2d, n2g and g2d are Copyright (C) 2000-05 Markus Brenner
<markus@brenner.de> and Pete Rittwage <peter@rittwage.com>

homepage: http://rittwage.com/c64pp/dp.php?pg=mnib
homepage: http://markus.brenner.de/mnib/


========================================
= Introduction                         =
========================================

   mnib is a disk transfer program designed for copying even copy protected
   original disks into the G64 and D64 disk image formats. These disk images
   may be used on C64 emulators like VICE or CCS64 [2,3]

   *NEW* Also, mnib lets you write back disk images in NIB, G64, or D64+errorblock
   format to real C64 disks.

   REQUIREMENTS:

   - Commodore Disk Drive model 1541, 1541-II or 1571, modified to support
     the parallel XP1541 or XP1571 interface [1]

   - XE1541, XA1541, or XM1541 cable [1]

   - Microsoft DOS and cwsdpmi.exe software, Linux with CBM4LINUX, or
     Windows NT/2000/XP with CBM4WIN.


========================================
= Usage                                =
========================================

Reading out disks into disk images:

   1) connect 1541/71 drive to your PC's parallel port(s), using
      the XE1541/XA1541 and the XP1541/71 cables.

   2) insert disk into drive and start mnib:
      mnib filename.nib

   3) use n2g to convert the nibble file into a G64 image,
      or use n2d to convert it into a D64 image:
      n2g filename.nib
      n2d filename.nib

Writing back disk images to a real disk:

   1) connect 1541/71 drive to your PC's parallel port(s), using
      the XE1541/XA1541 and the XP1541/71 cables.

   2) insert destination disk into drive and start mnib:
      mnib -w filename.nib
      - or -
      mnib -w filename.g64
      - or -
      mnib -w filename.d64


========================================
= Tips and Tricks                      =
========================================


   Please support me!
   ------------------

   For further development of mnib it is *vital* that I get feedback
   from you, the users! Please send me reports about your usage of
   mnib. I want to know about problems, as well as success and failures
   to convert Original disks to G64/D64 images.

   If you own a stack of original disks and plan to convert them
   using mnib, PLEASE DROP ME A MAIL - I would love to get and
   analyze your NIB images, working as well as non-working, to improve
   mnib's success rate for future versions.

   If you send me your NIB images I will gladly add your name to the
   Thank You! list at the end of this document :-)
   

   Success Rate on Originals
   -------------------------

   Currently, I estimate mnib's 'success rate' on successfully
   copying copy protected games into working G64 images at about
   95%.

   The following table gives an overview over protection schemes
   and mnib's chances on copying them:

   Copy Protection          D64     G64     Used by

   Read Errors              X       X       years ca. 1983-1985
   Tracks 35-40             X       X       Firebird, Para Protect
   Half Tracks                      X
   Wide Tracks                      X       early EA, Activision
   Long/Custom Tracks               X       Datasoft
   Slowed down motor                X       v-max!
   Sync counting                    X       Epyx (early Vorpal)
   Nonstandard bitrates             X       Vorpal, Rapidlok
   Bitrate changes in track
   Sector synchronization           X       Rapidlok (Accolade)
   00 (weak) bytes                          Datasoft, EA, Rainbow Arts

   Not all of these may run on the current emulators. Disk emulation
   still isn't perfect, especially some of the more tricky protections
   (sector synchronization, 00 Bytes, Bitrate changes) are not yet
   fully implemented by VICE and CCS64.  Later versions of CCS now do
   support 00 bytes, as well as Pete's patched versions of VICE.

   ---

   usage: mnib [options] filename

   -w : Write disk image. When specified, MNIB will wite to disk instead of
        the default, which is to read the current disk. This will destroy all
        data on the disk and replace it with the selected image. It works with
        D64, G64, and raw NIB image formats. Writing D64's does reproduce the
        errors if an errorblock is present.

   -v : Verify written data. When specified, MNIB will read each track back
        in after writing and try to verify it to what was written. Not all
        track types can be verified, but it should give you a good idea of
        whether the disk is bad or drive is failing if you get a lot of
        [NO VERIFY].

   -u : Unformat disk (removes *ALL* data). This option writes all $00 bytes
        (weak bits) to the entire disk surface, simulating the state of a
        brand new never-formatted disk.

   -l : Limit functions to 40 tracks (R/W). Some disk drives will not function
        past track 41 and will click and jam the heads too far forward. The
        drive cover must then be removed and the head pushed back manually.
        If this happens to you, use this option with every operation. There
        are only a few disks which utilize track 41 for protection.

   -h : Use halftracks (R/W). This option will step the drive heads 1/2
        track at a time during disk operations instead of a full track. This
        is just for future use and is not needed with any C64 disk I have run
        across (and I have over 1,500). I have never seen halftracks used.

   -k : Disable reading 'killer' tracks (R). Some drives will timeout when
        trying to read tracks that consist of all sync. If you cannot read
        a disk because of timeouts or a hang, use this option.

   -r : Disable 'reduce syncs' option (R). By default, MNIB will "compress"
        a track when writing back out to a disk if the track is longer than
        what your drive can write at any given density (due to drive motor
        speed). Some (very rare) protections count sync lengths so the
        protection might fail with this option. For 99.9% of disks, the
        default (reduce syncs) is fine.

   -g : Enable 'reduce gaps' option (R). This option is another form of
        "compression" used when writing out a disk. "gaps" are inert data
        placed right before a sync mark that can usually be safely removed.
        It is not on by default, but if MNIB is truncating tracks and they
        still won't load, you can try this option to squeeze a bit more onto
        the track.

   -0 : Enable 'reduce weak' option (R). This option is another form of
        "compression" used when writing out a disk. "weak bits" (when not
        used for copy protection) are unformatted or corrupted data that can
        usually be safely removed. It is not on by default, but if MNIB is
        truncating tracks and they still won't load, you can try this option
        to squeeze a bit more onto the track.

   -f : Disable weak GCR bit simulation (W). "weak bits" are either corrupted
        (or illegal) GCR that are either intentionally placed on a disk for
        protection, or are simply unformatted data on the disk.  MNIB will by
        default zero out this data and write it to disk as if it were
        unformatted. This option can be disabled if the disk image is using
        illegal GCR on purpose, such as what V-MAX! commonly does on the
        loader track 20.

   -aX: Alternative track alignments (W). There are several different ways
           to align tracks when writing them back out. By default, MNIB will
        do its best to figure out how the original disk was aligned by
        analyzing the data. To force other methods, use this option.

        -aw: Align all tracks to the longest run of unformatted data.
        -ag: Align all tracks to the longest gap between sectors.
        -a0: Align all tracks to sector 0.
        -as: Align all tracks to the longest sync mark.
        -aa: Align all tracks to the longest run of any one byte (autogap).

   -eX: Extended read retries (R). This is used on deteriorated disks to
        increase the number of read attempts to get a track with no errors.
        Use any numerical value, but if it's too high it could take a while
        to read the disk. Default is 10.

   -pX: Custom protection handlers (W). This is used to set some flags to
        handle copy protections which don't remaster with default settings.

        -px: Used for V-MAX! disks to remaster track 20 properly.
        -pg: Used for GMA/Securispeed disks to remaster track 38 properly.
        -pr: Used for Rapidlok disks to remaster them properly. Most do not
             work still due to track sync checks throughout the games.
        -pv: Used for newer Vorpal disks, which must be custom aligned to
             load when remastered.

   Why Does it Bump?
   -----------------

   At the beginning of each disk transfer mnib issues a 'bump' command.
   This is necessary to guarantee an optimal track adjustment of the
   read head. As mnib can't rely on sector checksums, there's no other
   way on adjusting the head-to-track alignment but bumping. Sorry!


========================================
= History                              =
========================================

   0.17 added automatic 1541/157x drive type detection 
   0.18 tracks 36-41 added. No more crashing with unformatted tracks. 
   0.19 added Density and Halftrack command line switches (-d, -h) 
   0.20 added Bump and Reset options (-b, -r) 
   0.21 added timeout routine for nibble transfer 
   0.22 added flush command during reading 
   0.23 disable interrupts during serial protocol 
   0.24 improved serial protocol 
   0.25 hopefully fixed nibbler hangups 
   0.26 added 'S' track reading (read without waiting for Sync) 
   0.27 added hidden 'g' switch for GEOS 1.2 disk image 
   0.28 improved killer track detection, fixed some n2d and n2g bugs 
   0.29 added direct D64 nibble functionality 
   0.30 added D64 error correction by multiple read 
   0.31 added 40 track support for D64 images 
   0.32 bin-include bn_flop.prg 
   0.33 improved D64 mode, added g2d utility to archive 
   0.34 improved track cycle detection
   0.35 added XA1541 support, paranoid mode, first public release
   0.36 Program mostly rewritten and made cross-platform

========================================
= References                           =
========================================

  The latest version of this program is available on
  http://rittwage.com/c64pp/dp.php?pg=mnib

  [1] Circuit-diagrams and order form for the adaptor and cables
      http://sta.c64.org/cables.html  (diagrams and shop for X-cables)
      http://sta.c64.org/xe1541.html  (XE1541 cable)
      http://sta.c64.org/xa1541.html  (XA1541 cable)
      http://sta.c64.org/xp1541.html  (XP1541/71 cables)

  [2] CCS64 homepage
      http://www.computerbrains.com/ccs64/

  [3] VICE homepage
      http://http://viceteam.bei.t-online.de/


   "Thank you!" to all people who helped me out with information and
   testing

   - Andreas Boose         <boose@linux.rz.fh-hannover.de>
   - Joe Forster           <sta@c64.org>
   - Michael Klein         <michael.klein@puffin.lb.shuttle.de>
   - Matt Larsen           <mlarsen2000@yahoo.com>
   - Chris Link            <Chris.Link@StudServ.Stud.Uni-Hannover.DE>
   - Wolfgang Moser        <womo@d81.de>
   - H†kan Sundell         <Hakan.Sundell@xpress.se>
   - Nicolas Welte         <nwelte@web.de>
   - Tim Schurman
   - Spiro Trikaliotis
   - Nate Lawson
   - Jerry Kurtz
   - Matt Larsen
   - Quader
   - Jani
   - Curt Coder
