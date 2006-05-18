The high precision 1541 RPM measurement tool for OpenCBM

CBMrpm41 is a rewrite from scratch of the former sample project
rpm1541. It bases on a unique and new developed technique to do
time measurements on a Commodore 1541 floppy disk drive.

  Code: (C) 2006 Wolfgang Moser

License: GNU General Public License
  On request I perhaps may want to publish the 6502 based code under
  the GNU Lesser General Public License to allow others to link that
  code to proprietary programs or other software products, which
  licensing terms are incompatible to the GPL, The Star Commander
  for example.

Some of the features implemented in the rewritten  6502  routine:

* VIA shift register usage (the core feature)
* VIA bugs regarding the shift register and more extracts from the
  former "viatimers.a65" testproject
* virtual 24 Bit timer construction
* Chinese Remainder Theorem, Extended Euklidean Algorithm and
  Coefficients calculation as well as determining the "best" modulus
  (latch register reload value)
* Tested shift register modes 101, 100 and finally 001 and their
  drawbacks
* Software extension of the virtual 23.598 bits timer to fully 32
  bits
* Ux command table programming with a user defined table at $0306
  (maybe as an idea for the on-demand o65 linker)
* Using ASCII-2-HEX (PETSCII ?) conversion routines in the 1541
  for parameter passing instead of transferring plain bytes


* Possible applications for the high precision timer system
    + measuring out track-to-track alignment
    + measuring out on-track sector distances
    + measuring out on-track SYNC lengths as well as distances
    + measuring mean bitrates
    + measuring sector distances (on-track) and sector distribution


Wolfgang Moser, 2006-05-18
