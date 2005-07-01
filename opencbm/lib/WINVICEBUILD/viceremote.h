#ifndef _VICEREMOTE_H
#define _VICEREMOTE_H

typedef
struct REMOTECONTROL_REGS {
    UINT AC;
    UINT XR;
    UINT YR;
    UINT SP;
    UINT FLAGS;
    UINT PC;
} REMOTECONTROL_REGS;

typedef
struct REMOTECONTROL_ONEREGBUFFER {
    REMOTECONTROL_REGS InRegs;
    REMOTECONTROL_REGS InRegsValid;
    REMOTECONTROL_REGS OutRegs;
} REMOTECONTROL_ONEREGBUFFER;

typedef
struct REMOTECONTROL_MEMBUFFER {
    UINT perform;
    UINT address;
    UINT size;
} REMOTECONTROL_MEMBUFFER;

typedef
struct REMOTECONTROL_ONEMEMBUFFER {
    REMOTECONTROL_MEMBUFFER read;
    REMOTECONTROL_MEMBUFFER write;
} REMOTECONTROL_ONEMEMBUFFER;

typedef
struct REMOTECONTROL {
    UINT version;
    UINT viceAvailable;
    UINT controllerAvailable;

    UINT vicebuffer;
    UINT viceackbuffer;
    UINT remotecontrollerbuffer;

    INT trapaddress;
    UINT reset;

    INT regupdateaddress;

    REMOTECONTROL_ONEMEMBUFFER memorybuffer[2];
    REMOTECONTROL_ONEREGBUFFER regbuffer[2];

    unsigned char data[2048];

} REMOTECONTROL;

#endif /* #ifndef _VICEREMOTE */
