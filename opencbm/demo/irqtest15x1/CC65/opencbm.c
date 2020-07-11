#include "opencbm.h"

#include <cbm.h>

int cbm_driver_open_ex(CBM_FILE *, char * adapter)
{
  *adapter;
  return 0;
}

void cbm_driver_close(CBM_FILE)
{
}

int cbm_listen(CBM_FILE, unsigned char dev, unsigned char secondary)
{
  cbm_k_listen(dev | 0x20);
  cbm_k_second(secondary | 0x60);
  return 0;
}

void cbm_unlisten(CBM_FILE)
{
  cbm_k_unlsn();
}

int cbm_talk(CBM_FILE, unsigned char dev, unsigned char secondary)
{
  cbm_k_talk(dev | 0x40);
  cbm_k_tksa(secondary | 0x60);
  return 0;
}

void cbm_untalk(CBM_FILE, )
{
  cbm_k_untlk();
}

int cbm_raw_write(CBM_FILE, unsigned char *buffer, size_t len)
{
  unsigned int i;

  for (i = 0; i < len; i++) {
    cbm_k_ciout(*buffer++);
  }

  return i;
}

int cbm_raw_read(CBM_FILE, unsigned char *buffer, size_t len)
{
  unsigned int i;

  for (i = 0; i < len; i++) {
    *buffer++ = cbm_k_acptr();
    if (cbm_k_readst() & 0x40) {
#if 0
      if (i < len) {
        *buffer++ = cbm_k_acptr();
        i++;
      }
#endif
      break;
    }
  }

  return i;
}


int  cbm_exec_command(CBM_FILE, unsigned char dev, unsigned char *cmd, size_t len)
{
  cbm_listen(0, dev, 15);

  if (len == 0) len = strlen(cmd);

  cbm_raw_write(0, cmd, len);

  cbm_unlisten(0);
  return 0;
}

#define MAX_BYTE_UPLOAD 32

/*-------------------------------------------------------------------*/
/*--------- HELPER FUNCTIONS ----------------------------------------*/

/*! \internal \brief Write a 8 bit value into the buffer

 This function writes a 8 bit value into the given buffer.

 \param Buffer
   A pointer to the memory address where the address and the byte
   count are to be written to.

 \param Value
   The 8 bit value to be written.
*/

static void StoreInt8IntoBuffer(unsigned char * Buffer, unsigned int Value)
{
    *Buffer = (unsigned char) Value;
}

/*! \internal \brief Write a 16 bit value into the buffer

 This function writes a 16 bit value into the given buffer.
 The value is written in low endian, that is, the low byte first.

 \param Buffer
   A pointer to the memory address where the address and the byte
   count are to be written to.

 \param Value
   The 16 bit value to be written.
*/

static void StoreInt16IntoBuffer(unsigned char * Buffer, unsigned int Value)
{
    StoreInt8IntoBuffer(Buffer++, Value % 256);
    StoreInt8IntoBuffer(Buffer,   Value / 256);
}

/*! \internal \brief Write an address and a count number into memory

 This function is used to write the drive address and the byte count
 for the "M-W" and "M-R" command.

 \param Buffer
   A pointer to the memory address where the address and the byte
   count are to be written to.

 \param DriveMemAddress
   The address in the drive's memory where the program is to be
   stored.

 \param ByteCount
   The number of bytes to be transferred.
*/

static void StoreAddressAndCount(unsigned char * Buffer, unsigned int DriveMemAddress, unsigned int ByteCount)
{
    StoreInt16IntoBuffer(Buffer, DriveMemAddress);
    StoreInt8IntoBuffer(Buffer + 2, ByteCount);
}




int cbm_upload(CBM_FILE c, unsigned char DeviceAddress,
           int DriveMemAddress, const void *Program, size_t Size)
{
    unsigned char *bufferToProgram = Program;

    unsigned char command[] = { 'm', '-', 'w', ' ', ' ', ' ' };
    size_t i;
    int rv = 0;

    for(i = 0; i < Size; i += c)
    {
        cbm_listen(0, DeviceAddress, 15);

        // Calculate how many bytes are left

        c = Size - i;

        // Do we have more than the maximum number? Then, restrict to maximum

        if (c > MAX_BYTE_UPLOAD)
        {
            c = MAX_BYTE_UPLOAD;
        }

        // The command M-W consists of:
        // M-W <lowaddress> <highaddress> <count>
        // build that command:

        StoreAddressAndCount(&command[3], DriveMemAddress, c);

        // Write the M-W command to the drive...

        if ( cbm_raw_write(0, command, sizeof(command)) != sizeof command) {
            rv = -1;
            break;
        }


        // ... as well as the (up to MAX_BYTE_UPLOAD) data bytes

        if ( cbm_raw_write(0, bufferToProgram, c) != c ) {
            rv = -1;
            break;
        }

        // The UNLISTEN is the signal for the drive
        // to start execution of the command

        cbm_unlisten(0);

        // Now, advance the pointer into drive memory
        // as well to the program in PC's memory in case we
        // might need to use it again for another M-W command

        DriveMemAddress += c;
        bufferToProgram += c;

        // Advance the return value of send bytes, too.

        rv += c;
    }

    return rv;
}

/*! \brief Download data from a floppy's drive memory.

 This function reads data from the drive's memory via
 use of "M-R" commands.

 \param HandleDevice
   A CBM_FILE which contains the file handle of the driver.

 \param DeviceAddress
   The address of the device on the IEC serial bus. This
   is known as primary address, too.

 \param DriveMemAddress
   The address in the drive's memory where the program is to be
   stored.

 \param Buffer
   Pointer to a byte buffer where the data from the drive's
   memory is stored.

 \param Size
   The size of the data block to be stored, in bytes.

 \return
   Returns the number of bytes written into the storage buffer.
   If it does not equal Size, than an error occurred.
   Specifically, -1 is returned on transfer errors.

 If cbm_driver_open() did not succeed, it is illegal to
 call this function.
*/

enum { TRANSFER_SIZE_DOWNLOAD = 0x100u };

int cbm_download(CBM_FILE HandleDevice, unsigned char DeviceAddress, int DriveMemAddress, void * Buffer, size_t Size)
{
    unsigned char command[] = { 'm', '-', 'r', ' ', ' ', '\0', '\r' };
    unsigned char *StoreBuffer = Buffer;

    size_t i;
    int rv = 0;
    int readbytes = 0;
    int c;

    for(i = 0; i < Size; i += c)
    {
        // Calculate how much bytes are left
        c = Size - i;

        // Do we have more than 256? Then, restrict to 256
        if (c > TRANSFER_SIZE_DOWNLOAD)
        {
            c = TRANSFER_SIZE_DOWNLOAD;
        }

        /*
         * Workaround: The 154x/157x/1581 drives (and possibly others, too)
         * cannot cross a page boundary on M-R. Thus, make sure we do not
         * cross the boundary.
         */
        if (c + (DriveMemAddress & 0xFF) > 0x100) {
            c = 0x100 - (DriveMemAddress & 0xFF);
        }

        // The command M-R consists of:
        // M-R <lowaddress> <highaddress> <count> '\r'
        // build that command:

        StoreAddressAndCount(&command[3], DriveMemAddress, c);

        // Write the M-R command to the drive...
        if ( cbm_exec_command(HandleDevice, DeviceAddress, command, sizeof(command)) ) {
            rv = -1;
            break;
        }

        cbm_talk(0, DeviceAddress, 15);

        // now read the (up to 256) data bytes
        // and advance the return value of send bytes, too.
        readbytes = cbm_raw_read(0, StoreBuffer, c);

        if (readbytes != c) {
            rv = -1;
            break;
        }
        // Now, advance the pointer into drive memory
        // as well to the program in PC's memory in case we
        // might need to use it again for another M-W command
        DriveMemAddress += readbytes;
        StoreBuffer     += readbytes;

        rv              += readbytes;

#if 0
        {
        char dummy;

        // skip the trailing CR
        if ( cbm_raw_read(0, &dummy, 1) != 1 ) {
            rv = -1;
            break;
        }
        }
#endif

        // The UNTALK is the signal for end of transmission
        cbm_untalk(0);
    }

    return rv;
}

int cbm_identify(CBM_FILE, unsigned char DeviceAddress, enum cbm_device_type_e *CbmDeviceType, const char **CbmDeviceString)
{
    enum cbm_device_type_e deviceType = cbm_dt_unknown;
    unsigned short magic;
    unsigned char buf[3];
    char command[] = { 'm', '-', 'r', (char) 0x40, (char) 0xff, (char) 0x02 };
    static char unknownDevice[] = "*unknown*";
    char *deviceString = unknownDevice;
    int rv = -1;

    /* get footprint from 0xFF40 */
    if (cbm_exec_command(0, DeviceAddress, command, sizeof(command)) == 0
        && cbm_talk(0, DeviceAddress, 15) == 0)
    {
        if (cbm_raw_read(0, buf, 3) == 2)
        {
            magic = buf[0] | (buf[1] << 8);

            if(magic == 0xaaaa)
            {
                cbm_untalk(0);
                command[3] = (char) 0xFE; /* get footprint from 0xFFFE, IRQ vector */
                if (cbm_exec_command(0, DeviceAddress, command, sizeof(command)) == 0
                    && cbm_talk(0, DeviceAddress, 15) == 0)
                {
                    if (cbm_raw_read(0, buf, 3) == 2
                        && ( buf[0] != 0x67 || buf[1] != 0xFE ) )
                    {
                        magic = buf[0] | (buf[1] << 8);
                    }
                }
            }

            switch(magic)
            {
                default:
                    break;

#if 0
                case 0xfeb6:
                    deviceType = cbm_dt_cbm2031;
                    deviceString = "2031";
                    break;

                case 0xaaaa:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "1540 or 1541";
                    break;

                case 0xf00f:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "1541-II";
                    break;

                case 0xcd18:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "1541C";
                    break;

                case 0x10ca:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "DolphinDOS 1541";
                    break;

                case 0x6f10:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "SpeedDOS 1541";
                    break;

                case 0x2710:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "ProfessionalDOS 1541";
                    break;

                case 0x8085:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "JiffyDOS 1541";
                    break;

                case 0xaeea:
                    deviceType = cbm_dt_cbm1541;
                    deviceString = "64'er DOS 1541";
                    break;
#endif

                case 0xfed7:
                    deviceType = cbm_dt_cbm1570;
                    deviceString = "1570";
                    break;

                case 0x02ac:
                    deviceType = cbm_dt_cbm1571;
                    deviceString = "1571";
                    break;

                case 0x01ba:
                    deviceType = cbm_dt_cbm1581;
                    deviceString = "1581";
                    break;
            }
            rv = 0;
        }
        cbm_untalk(0);
    }

    if(CbmDeviceType)
    {
        *CbmDeviceType = deviceType;
    }

    if(CbmDeviceString)
    {
        *CbmDeviceString = deviceString;
    }

    return rv;
}
