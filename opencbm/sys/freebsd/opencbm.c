/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  Copyright 1999-2002 Michael Klein <michael(dot)klein(at)puffin(dot)lb(dot)shuttle(dot)de>
 *  Copyright 1997-2005 Joe Forster <sta(at)c64(dot)org> (Device Detection Code)
 *  Copyright 1997-2005 Wolfgang Moser (http://d81.de)   (Device Detection Code)
 *  Copyright 2000-2005 Markus Brenner                   (Parallel Burst Routines)
 *  Copyright 2000-2005 Pete Rittwage                    (Parallel Burst Routines)
 *  Copyright 2005      Tim Schürmann                    (Parallel Burst Routines)
 *  Copyright 2005-2006,2009 Spiro Trikaliotis           (Parallel Burst Routines)
 *  Copyright 2007-2009 Frédéric Brière                  (Adjustments on newer Linux kernels, abstraction from real hardware)
 *  Copyright 2009      Arnd Menge <arnd(at)jonnz(dot)de> (Parallel Burst Routines)
 *  Copyright 2018      Felix Palmen <felix(at)palmen-it(dot)de> (FreeBSD port)
 *
 */

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/syslog.h>

#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/bus.h>
#include <sys/malloc.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/condvar.h>

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/ppbus/ppbconf.h>
#include "ppbus_if.h"
#include <dev/ppbus/ppbio.h>

#define BUILDING_FREEBSD_MODULE
#include "../../include/LINUX/cbm_module.h"

#define CBM_NAME "cbm"

#define IEC_DATA   1
#define IEC_CLOCK  2
#define IEC_ATN    4
#define IEC_RESET  8

/* lpt output lines */
#define ATN_OUT   0x01
#define CLK_OUT   0x02
#define DATA_OUT  0x04
#define RESET     0x08
#define LP_IRQ    0x10
#define LP_BIDIR  0x20

/* lpt input lines */
#define ATN_IN     0x10
#define CLK_IN     0x20
#define DATA_IN    0x40

#define GET(line)        ((POLL() & line) == 0 ? 1 : 0)
#define SET(line)        (CTRL_WRITE(sc->sc_out_eor ^ \
                                (sc->sc_out_bits |= line)))
#define RELEASE(line)    (CTRL_WRITE(sc->sc_out_eor ^ \
                                (sc->sc_out_bits &= ~(line))))
#define SET_RELEASE(s,r) (CTRL_WRITE(sc->sc_out_eor ^ \
                                (sc->sc_out_bits = \
                                 (sc->sc_out_bits | (s)) & ~(r))))

#define POLL()           (ppb_rstr(ppbus))
#define XP_READ()        (ppb_rdtr(ppbus))
#define XP_WRITE(c)      (ppb_wdtr(ppbus,c))
#define CTRL_READ()      (ppb_rctr(ppbus))
#define CTRL_WRITE(c)    (ppb_wctr(ppbus,c))

#define set_data_forward() do { RELEASE(LP_BIDIR); \
                                 sc->sc_data_reverse = 0; } while (0)
#define set_data_reverse() do { SET(LP_BIDIR); \
                                 sc->sc_data_reverse = 1; } while (0)
#define timeout_us(us) pause(CBM_NAME, hz/1000000 * us)

#ifdef DEBUG
#define DBGLOG(fmt, ...) log(LOG_DEBUG, fmt, ##__VA_ARGS__)
#else
#define DBGLOG(fmt, ...)
#endif

static d_open_t cbm_open;
static d_close_t cbm_close;
static d_read_t cbm_read;
static d_write_t cbm_write;
static d_ioctl_t cbm_ioctl;

static int lp = 0;

static MALLOC_DEFINE(M_CBM, "cbm_buffer", "buffer for CBM bus I/O");

#define BUFFER_SIZE 0x2000

struct cbm_data {
    int sc_irq_rid;
    struct resource *sc_irq_resource;
    void *sc_irq_cookie;
    device_t sc_device;
    struct cdev *sc_cdev;
    int sc_cable;
    int sc_reset;
    int sc_hold_clk;
    unsigned char sc_out_bits;
    unsigned char sc_out_eor;
    int sc_busy;
    int sc_data_reverse;
    volatile int sc_cbm_irq_count;
    volatile int sc_eoi;
    struct mtx sc_lock;
    struct cv sc_cvp;
    char *sc_buf;
};

static struct cdevsw cbm_cdevsw = {
    .d_version = D_VERSION,
    .d_open = cbm_open,
    .d_close = cbm_close,
    .d_read = cbm_read,
    .d_write = cbm_write,
    .d_ioctl = cbm_ioctl,
    .d_name = CBM_NAME
};

/* forward references for parallel burst routines */
int cbm_parallel_burst_read_track(struct cbm_data *sc, device_t ppbus,
        unsigned char *buffer);
int cbm_parallel_burst_read_track_var(struct cbm_data *sc, device_t ppbus,
        unsigned char *buffer);
int cbm_parallel_burst_write_track(struct cbm_data *sc, device_t ppbus,
        unsigned char *buffer, int length);
unsigned char cbm_parallel_burst_read(struct cbm_data *sc, device_t ppbus);
int cbm_parallel_burst_write(struct cbm_data *sc, device_t ppbus,
        unsigned char c);
int cbm_handshaked_read(struct cbm_data *sc, device_t ppbus, int toggle);
int cbm_handshaked_write(struct cbm_data *sc, device_t ppbus,
        char data, int toggle);

static int
check_if_bus_free(struct cbm_data *sc, device_t ppbus)
{
    int ret = 0;
    do
    {
        RELEASE(ATN_OUT | CLK_OUT | DATA_OUT | RESET);
        timeout_us(100);
        SET(ATN_OUT);
        timeout_us(100);
        if (!GET(DATA_IN)) break;
        RELEASE(ATN_OUT);
        timeout_us(100);
        if (!GET(DATA_IN)) ret = 1;
    } while (0);

    RELEASE(ATN_OUT | CLK_OUT | DATA_OUT | RESET);
    return ret;
}

static void
wait_for_free_bus(struct cbm_data *sc, device_t ppbus)
{
    int i = 1;
    while (1)
    {
        if (check_if_bus_free(sc, ppbus))
        {
            device_printf(sc->sc_device, "bus is free!\n");
            break;
        }
        if (++i == 1000)
        {
            device_printf(sc->sc_device, "timeout waiting for free bus\n");
            break;
        }
        timeout_us(1000);
    }
}

static int
send_byte(struct cbm_data *sc, device_t ppbus, int b)
{
        int i, ack = 0;
        register_t saveintr;

        DBGLOG("send_byte %02x\n", b);

        saveintr = intr_disable();
        for (i = 0; i < 8; i++) {
                DELAY(70);
                if (!((b >> i) & 1))
                        SET(DATA_OUT);
                RELEASE(CLK_OUT);
                DELAY(20);
                SET_RELEASE(CLK_OUT, DATA_OUT);
        }
        intr_restore(saveintr);

        for (i = 0; (i < 20) && !(ack = GET(DATA_IN)); i++)
                DELAY(100);

        DBGLOG("ack=%d\n", ack);

        return ack;
}

static void
do_reset(struct cbm_data *sc, device_t ppbus)
{
    device_printf(sc->sc_device, "resetting devices\n");
    RELEASE(DATA_OUT | ATN_OUT | CLK_OUT | LP_IRQ | LP_BIDIR);
    sc->sc_data_reverse = 0;
    SET(RESET);
    pause(CBM_NAME, hz/10);
    RELEASE(RESET);
    device_printf(sc->sc_device, "waiting for free bus...\n");
    wait_for_free_bus(sc, ppbus);
}

static int
cbm_open(struct cdev *dev, int oflags __unused, int devtype __unused,
        struct thread *td __unused)
{
    struct cbm_data *sc = dev->si_drv1;
    device_t ppbus = device_get_parent(sc->sc_device);

    if (sc->sc_busy) return EBUSY;

    sc->sc_busy = 1;
    if (sc->sc_hold_clk) SET(CLK_OUT);

    return 0;
}

static int
cbm_close(struct cdev *dev, int fflag __unused, int devtype __unused,
        struct thread *td __unused)
{
    struct cbm_data *sc = dev->si_drv1;
    device_t ppbus = device_get_parent(sc->sc_device);

    if (!sc->sc_hold_clk) RELEASE(CLK_OUT);
    sc->sc_busy = 0;
    return 0;
}

static int
wait_for_listener(struct cbm_data *sc, device_t ppbus)
{
    mtx_lock(&sc->sc_lock);
    SET(LP_IRQ);
    int rv = 0;
    RELEASE(CLK_OUT);
    ppb_unlock(ppbus);
    while (sc->sc_cbm_irq_count &&
            !(rv = cv_wait_sig(&sc->sc_cvp, &sc->sc_lock)));
    ppb_lock(ppbus);
    RELEASE(LP_IRQ);
    mtx_unlock(&sc->sc_lock);
    return rv ? -1 : 0;
}

static int
cbm_raw_write(struct cbm_data *sc, device_t ppbus,
        const char *buf, size_t cnt, int atn, int talk)
{
        unsigned char c;
        int i;
        int rv = 0;
        size_t sent = 0;
        register_t saveintr;

        sc->sc_eoi = sc->sc_cbm_irq_count = 0;

        DBGLOG("cbm_write: %zu bytes, atn=%d\n", cnt, atn);

        RELEASE(DATA_OUT);
        SET(CLK_OUT | (atn ? ATN_OUT : 0));

        for (i = 0; (i < 100) && !GET(DATA_IN); i++)
                DELAY(10);

        if (!GET(DATA_IN)) {
                device_printf(sc->sc_device, "cbm_write: no devices found\n");
                RELEASE(CLK_OUT | ATN_OUT);
                return -ENODEV;
        }

        tsleep(sc, PCATCH, CBM_NAME, hz/50); /* 20ms */

        while (cnt > sent && rv == 0) {
                c = *buf++;
                DELAY(50);
                if (GET(DATA_IN)) {
                        sc->sc_cbm_irq_count = ((sent == (cnt - 1))
                                         && (atn == 0)) ? 2 : 1;

                        if (wait_for_listener(sc, ppbus) < 0) {
                                rv = -EINTR;
                        } else {
                                if (send_byte(sc, ppbus, c)) {
                                        sent++;
                                        DELAY(100);
                                } else {
                                        device_printf(sc->sc_device,
                                                "cbm_write: I/O error\n");
                                        rv = -EIO;
                                }
                        }
                } else {
                        device_printf(sc->sc_device,
                                "cbm_write: device not present\n");
                        rv = -ENODEV;
                }
        }
        DBGLOG("%zu bytes sent, rv=%d\n", sent, rv);

        if (talk && (rv == 0)) {
                saveintr = intr_disable();
                SET(DATA_OUT);
                RELEASE(ATN_OUT);

                RELEASE(CLK_OUT);
                for (i = 0; (i < 100) && !GET(CLK_IN); i++)
                        DELAY(10);
                if (!GET(CLK_IN)) {
                        device_printf(sc->sc_device,
                                "cbm_write: device not present\n");
                        rv = -ENODEV;
                }

                intr_restore(saveintr);
        } else {
                RELEASE(ATN_OUT);
        }
        DELAY(100);

        return (rv < 0) ? rv : (int)sent;
}

static int
cbm_write(struct cdev *dev, struct uio *uio, int ioflag __unused)
{
    struct cbm_data *sc = dev->si_drv1;
    device_t ppbus = device_get_parent(sc->sc_device);
    int rv = 0;

    ppb_lock(ppbus);
    while (uio->uio_resid > 0)
    {
        int amount = MIN(uio->uio_resid, BUFFER_SIZE);
        rv = uiomove(sc->sc_buf, amount, uio);
        if (rv != 0) break;
        rv = cbm_raw_write(sc, ppbus, sc->sc_buf, amount, 0, 0);
        if (rv < 0)
        {
            rv = -rv;
            break;
        }
        rv = 0;
    }
    ppb_unlock(ppbus);
    return rv;
}

static int
cbm_read(struct cdev *dev, struct uio *uio, int ioflag __unused)
{
    size_t received = 0;
    int bufpos = 0;
    int i, b, bit;
    int ok = 0;
    register_t saveintr;

    struct cbm_data *sc = dev->si_drv1;
    device_t ppbus = device_get_parent(sc->sc_device);

    ssize_t count = uio->uio_resid;
    DBGLOG("cbm_read: %zd bytes\n", count);

    if (sc->sc_eoi) return 0;

    ppb_lock(ppbus);
    do {
        i = 0;
        while (GET(CLK_IN)) {
            if (i >= 50) {
                if (tsleep(dev, PCATCH, CBM_NAME, hz/50) != EWOULDBLOCK)
                {
                    ppb_unlock(ppbus);
                    return EINTR;
                }
            } else {
                i++;
                DELAY(20);
            }
        }
        saveintr = intr_disable();
        RELEASE(DATA_OUT);
        for (i = 0; (i < 40) && !(ok = GET(CLK_IN)); i++)
            DELAY(10);
        if (!ok) {
            sc->sc_eoi = 1;
            SET(DATA_OUT);
            DELAY(70);
            RELEASE(DATA_OUT);
        }
        for (i = 0; i < 100 && !(ok = GET(CLK_IN)); i++)
            DELAY(20);
        for (bit = b = 0; (bit < 8) && ok; bit++) {
            for (i = 0; (i < 200) && !(ok = (GET(CLK_IN) == 0)); i++)
                DELAY(10);
            if (ok) {
                b >>= 1;
                if (GET(DATA_IN) == 0)
                    b |= 0x80;
                for (i = 0; i < 100 && !(ok = GET(CLK_IN)); i++)
                    DELAY(20);
            }
        }
        if (ok)
            SET(DATA_OUT);
        intr_restore(saveintr);
        if (ok) {
            ++received;
            sc->sc_buf[bufpos++] = b;
            if (bufpos == BUFFER_SIZE)
            {
                uiomove(sc->sc_buf, BUFFER_SIZE, uio);
                bufpos = 0;
            }
            if (received % 256)
                DELAY(50);
            else
                tsleep(dev, PCATCH, CBM_NAME, hz/50);
        }
    } while (received < count && ok && !sc->sc_eoi);
    ppb_unlock(ppbus);

    if (!ok)
    {
        device_printf(sc->sc_device, "cbm_read: I/O error\n");
        return EIO;
    }

    if (bufpos) uiomove(sc->sc_buf, bufpos, uio);

    DBGLOG("received=%zu, count=%zd, ok=%d, eoi=%d\n",
            received, count, ok, sc->sc_eoi);

    return 0;
}

static int
cbm_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int fflag,
        struct thread *td)
{
        struct cbm_data *sc = dev->si_drv1;
        device_t ppbus = device_get_parent(sc->sc_device);

        unsigned char buf[2], c, talk, mask, state, i;
        int rv = 0;
        int arg = 0;
        PARBURST_RW_VALUE *val = 0;

        if (cmd == CBMCTRL_TALK
                || cmd == CBMCTRL_LISTEN
                || cmd == CBMCTRL_OPEN
                || cmd == CBMCTRL_CLOSE
                || cmd == CBMCTRL_IEC_SET
                || cmd == CBMCTRL_IEC_RELEASE
                || cmd == CBMCTRL_IEC_WAIT
                || cmd == CBMCTRL_IEC_SETRELEASE)
            arg = *(int *)data;

        buf[0] = (arg >> 8) & 0x1f;     /* device */
        buf[1] = arg & 0x0f;    /* secondary address */

        ppb_lock(ppbus);
        switch (cmd) {
        case CBMCTRL_RESET:
                do_reset(sc, ppbus);
                rv = 0;
                break;

        case CBMCTRL_TALK:
        case CBMCTRL_LISTEN:
                talk = (cmd == CBMCTRL_TALK);
                buf[0] |= talk ? 0x40 : 0x20;
                buf[1] |= 0x60;
                rv = cbm_raw_write(sc, ppbus, buf, 2, 1, talk);
                rv = rv > 0 ? 0 : -rv;
                break;

        case CBMCTRL_UNTALK:
        case CBMCTRL_UNLISTEN:
                buf[0] = (cmd == CBMCTRL_UNTALK) ? 0x5f : 0x3f;
                rv = cbm_raw_write(sc, ppbus, buf, 1, 1, 0);
                rv = rv > 0 ? 0 : -rv;
                break;

        case CBMCTRL_OPEN:
        case CBMCTRL_CLOSE:
                buf[0] |= 0x20;
                buf[1] |= (cmd == CBMCTRL_OPEN) ? 0xf0 : 0xe0;
                rv = cbm_raw_write(sc, ppbus, buf, 2, 1, 0);
                if ((cmd == CBMCTRL_CLOSE) && (rv == 0)) {
                        /* issue an unlisten */
                        buf[0] = 0x3f;
                        cbm_raw_write(sc, ppbus, buf, 1, 1, 0);
                }
                rv = rv > 0 ? 0 : -rv;
                break;

        case CBMCTRL_GET_EOI:
                *(int *)data = sc->sc_eoi ? 1 : 0;
                rv = 0;
                break;

        case CBMCTRL_CLEAR_EOI:
                sc->sc_eoi = 0;
                rv = 0;
                break;

        case CBMCTRL_IEC_WAIT:
                rv = 0;
                switch (arg >> 8) {
                case IEC_DATA:
                        mask = DATA_IN;
                        break;
                case IEC_CLOCK:
                        mask = CLK_IN;
                        break;
                case IEC_ATN:
                        mask = ATN_IN;
                        break;
                default:
                        rv = EINVAL;
                }
                if (rv) break;
                state = (arg & 0xff) ? mask : 0;
                i = 0;
                while ((POLL() & mask) == state) {
                        if (i >= 20) {
                                if (tsleep(dev, PCATCH, CBM_NAME, hz/50)
                                        != EWOULDBLOCK)
                                {
                                    rv = EINTR;
                                    break;
                                }
                        } else {
                                i++;
                                DELAY(10);
                        }
                }
                if (rv) break;
                /* fall through */

        case CBMCTRL_IEC_POLL:
                c = POLL();
                if ((c & DATA_IN) == 0)
                        rv |= IEC_DATA;
                if ((c & CLK_IN) == 0)
                        rv |= IEC_CLOCK;
                if ((c & ATN_IN) == 0)
                        rv |= IEC_ATN;
                *(int *)data = rv;
                rv = 0;
                break;

        case CBMCTRL_IEC_SET:
                if (arg & ~(IEC_DATA | IEC_CLOCK | IEC_ATN | IEC_RESET)) {
                        /*
                         * there was some bit set that is not recognized, return
                         * with an error
                         */
                        rv = EINVAL;
                } else {
                        if (arg & IEC_DATA)
                                SET(DATA_OUT);
                        if (arg & IEC_CLOCK)
                                SET(CLK_OUT);
                        if (arg & IEC_ATN)
                                SET(ATN_OUT);
                        if (arg & IEC_RESET)
                                SET(RESET);
                        rv = 0;
                }
                break;

        case CBMCTRL_IEC_RELEASE:
                if (arg & ~(IEC_DATA | IEC_CLOCK | IEC_ATN | IEC_RESET)) {
                        /*
                         * there was some bit set that is not recognized, return
                         * with an error
                         */
                        rv = EINVAL;
                } else {
                        if (arg & IEC_DATA)
                                RELEASE(DATA_OUT);
                        if (arg & IEC_CLOCK)
                                RELEASE(CLK_OUT);
                        if (arg & IEC_ATN)
                                RELEASE(ATN_OUT);
                        if (arg & IEC_RESET)
                                RELEASE(RESET);
                        rv = 0;
                }
                break;

        case CBMCTRL_IEC_SETRELEASE:
                {
                        unsigned set = arg >> 8;
                        unsigned release = arg & 0xFF;
                        unsigned set_mask = 0;
                        unsigned release_mask = 0;

                        if ((set &
                             ~(IEC_DATA | IEC_CLOCK | IEC_ATN | IEC_RESET))
                            || (release &
                                ~(IEC_DATA | IEC_CLOCK | IEC_ATN | IEC_RESET)))
                        {
                                /*
                                 * there was some bit set that is not recognized, return
                                 * with an error
                                 */
                                rv = EINVAL;
                        } else {
                                if (set & IEC_DATA)
                                        set_mask |= DATA_OUT;
                                if (set & IEC_CLOCK)
                                        set_mask |= CLK_OUT;
                                if (set & IEC_ATN)
                                        set_mask |= ATN_OUT;
                                if (set & IEC_RESET)
                                        set_mask |= RESET;

                                if (release & IEC_DATA)
                                        release_mask |= DATA_OUT;
                                if (release & IEC_CLOCK)
                                        release_mask |= CLK_OUT;
                                if (release & IEC_ATN)
                                        release_mask |= ATN_OUT;
                                if (release & IEC_RESET)
                                        release_mask |= RESET;

                                SET_RELEASE(set_mask, release_mask);
                                rv = 0;
                        }
                }
                break;

        case CBMCTRL_PP_READ:
                if (!sc->sc_data_reverse) {
                        XP_WRITE(0xff);
                        set_data_reverse();
                }
                *(unsigned char *)data = XP_READ();
                rv = 0;
                break;

        case CBMCTRL_PP_WRITE:
                arg = *(unsigned char *)data;
                if (sc->sc_data_reverse)
                        set_data_forward();
                XP_WRITE(arg);
                rv = 0;
                break;

/* and now the parallel burst-routines */

        case CBMCTRL_PARBURST_READ:
                *(unsigned char *)data = cbm_parallel_burst_read(sc, ppbus);
                rv = 0;
                break;

        case CBMCTRL_PARBURST_WRITE:
                arg = *(unsigned char *)data;
                rv = cbm_parallel_burst_write(sc, ppbus, arg);
                break;

        case CBMCTRL_PARBURST_READ_TRACK:
                val = (PARBURST_RW_VALUE *)data;
                if (val->length < BUFFER_SIZE)
                {
                    rv = EINVAL;
                    break;
                }
                rv = !cbm_parallel_burst_read_track(sc, ppbus, sc->sc_buf);
                if (!rv) rv = copyout(sc->sc_buf, val->buffer, BUFFER_SIZE);
                break;

        case CBMCTRL_PARBURST_READ_TRACK_VAR:
                val = (PARBURST_RW_VALUE *)data;
                if (val->length < BUFFER_SIZE)
                {
                    rv = EINVAL;
                    break;
                }
                rv = !cbm_parallel_burst_read_track_var(sc, ppbus, sc->sc_buf);
                if (!rv) rv = copyout(sc->sc_buf, val->buffer, BUFFER_SIZE);
                break;

        case CBMCTRL_PARBURST_WRITE_TRACK:
                val = (PARBURST_RW_VALUE *)data;
                if (val->length > BUFFER_SIZE)
                {
                    rv = EINVAL;
                    break;
                }
                rv = copyin(val->buffer, sc->sc_buf, val->length);
                if (rv)
                    break;
                rv = !cbm_parallel_burst_write_track(sc, ppbus,
                        val->buffer, val->length);
                break;

        default:
                rv = ENOTTY;
        }
        ppb_unlock(ppbus);
        return rv;
}

static void
cbm_intr(void *arg)
{
    struct cbm_data *sc = arg;
    device_t ppbus = device_get_parent(sc->sc_device);

    POLL();
    if (sc->sc_cbm_irq_count == 0)
    {
        device_printf(sc->sc_device, "spurious interrupt\n");
        return;
    }
    else if (--sc->sc_cbm_irq_count == 0)
    {
        ppb_lock(ppbus);
        SET(CLK_OUT);
        DBGLOG("cbm_intr(): continue to send\n");
        ppb_unlock(ppbus);
        cv_signal(&sc->sc_cvp);
    }
    else
    {
        DBGLOG("cbm_intr(): must still wait\n");
    }
}

static void
cbm_identify(driver_t *driver, device_t parent)
{
    TUNABLE_INT("cbm.lp", &lp);
    int unit = device_get_unit(parent);
    if (unit != lp) return;
    device_t dev = device_find_child(parent, CBM_NAME, -1);
    if (!dev) BUS_ADD_CHILD(parent, 0, CBM_NAME, -1);
}

static int
cbm_probe(device_t dev)
{
    device_set_desc(dev, "Serial CBM bus driver");
    return BUS_PROBE_SPECIFIC;
}

static int
cbm_attach(device_t dev)
{
    struct cbm_data *sc = device_get_softc(dev);
    int error = 0;

    sc->sc_irq_rid = 0;
    sc->sc_irq_resource = bus_alloc_resource_any(dev, SYS_RES_IRQ,
            &sc->sc_irq_rid, RF_ACTIVE | RF_SHAREABLE);

    if (!sc->sc_irq_resource)
    {
        device_printf(dev, "unable to allocate interrupt resource\n");
        return ENXIO;
    }

    error = bus_setup_intr(dev, sc->sc_irq_resource,
            INTR_TYPE_TTY | INTR_MPSAFE, 0, cbm_intr,
            sc, &sc->sc_irq_cookie);
    if (error)
    {
        bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irq_rid,
                sc->sc_irq_resource);
        device_printf(dev, "unable to register interrupt handler\n");
        return error;
    }

    struct make_dev_args args;
    make_dev_args_init(&args);
    args.mda_flags = MAKEDEV_WAITOK | MAKEDEV_CHECKNAME;
    args.mda_devsw = &cbm_cdevsw;
    args.mda_uid = UID_ROOT;
    args.mda_gid = GID_OPERATOR;
    args.mda_mode = 0660;
    error = make_dev_s(&args, &sc->sc_cdev, CBM_NAME);
    if (error)
    {
        bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irq_rid,
                sc->sc_irq_resource);
        device_printf(dev, "unable to create character device\n");
        return error;
    }
    sc->sc_cdev->si_drv1 = sc;

    sc->sc_device = dev;

    sc->sc_cable = -1;
    sc->sc_reset = 1;
    sc->sc_hold_clk = 1;
    TUNABLE_INT_FETCH("cbm.cable", &sc->sc_cable);
    TUNABLE_INT_FETCH("cbm.reset", &sc->sc_reset);
    TUNABLE_INT_FETCH("cbm.hold_clk", &sc->sc_hold_clk);

    device_t ppbus = device_get_parent(dev);
    ppb_lock(ppbus);

    if (!PPB_IN_PS2_MODE(ppbus) && ppb_set_mode(ppbus, PPB_SPP) < 0)
    {
        ppb_unlock(ppbus);
        destroy_dev(sc->sc_cdev);
        bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irq_rid,
                sc->sc_irq_resource);
        device_printf(dev, "unable to set SPP port mode\n");
        return EINVAL;
    }

    error = ppb_request_bus(ppbus, dev, PPB_WAIT | PPB_INTR);
    if (error)
    {
        ppb_unlock(ppbus);
        destroy_dev(sc->sc_cdev);
        bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irq_rid,
                sc->sc_irq_resource);
        device_printf(dev, "unable to own parallel port\n");
        return error;
    }

    device_printf(dev, "parallel port is mine now\n");

    const char *msg = "";
    if (sc->sc_cable < 0)
    {
        unsigned char in = GET(ATN_IN);
        unsigned char out = (CTRL_READ() & ATN_OUT) ? 1 : 0;
        sc->sc_cable = (in != out);
        msg = " (auto)";
    }

    sc->sc_out_eor = sc->sc_cable ? 0xcb : 0xc4;

    device_printf(dev, "using %s cable%s\n",
            sc->sc_cable ? "active (XA1541)" : "passive (XM1541)", msg);

    sc->sc_cbm_irq_count = 0;

    sc->sc_out_bits = (CTRL_READ() ^ sc->sc_out_eor) &
        (DATA_OUT | CLK_OUT | ATN_OUT | RESET);

    if ((sc->sc_reset < 0 && (sc->sc_out_bits & RESET)) || sc->sc_reset > 0)
        do_reset(sc, ppbus);

    sc->sc_busy = 0;

    RELEASE(DATA_OUT | ATN_OUT | CLK_OUT | LP_IRQ | LP_BIDIR);
    sc->sc_data_reverse = 0;
    ppb_unlock(ppbus);

    mtx_init(&sc->sc_lock, CBM_NAME, 0, MTX_DEF);
    cv_init(&sc->sc_cvp, CBM_NAME);

    sc->sc_buf = malloc(BUFFER_SIZE, M_CBM, M_WAITOK);

    tsleep(sc, PCATCH, CBM_NAME, hz/20);

    return 0;
}

static int
cbm_detach(device_t dev)
{
    int error = 0;
    struct cbm_data *sc = device_get_softc(dev);

    free(sc->sc_buf, M_CBM);

    cv_destroy(&sc->sc_cvp);
    mtx_destroy(&sc->sc_lock);
    destroy_dev(sc->sc_cdev);

    device_t ppbus = device_get_parent(dev);
    ppb_lock(ppbus);
    error = ppb_release_bus(ppbus, dev);
    ppb_unlock(ppbus);

    bus_teardown_intr(dev, sc->sc_irq_resource, sc->sc_irq_cookie);
    bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irq_rid,
            sc->sc_irq_resource);

    return error;
}

static device_method_t cbm_methods[] = {
    DEVMETHOD(device_identify, cbm_identify),
    DEVMETHOD(device_probe, cbm_probe),
    DEVMETHOD(device_attach, cbm_attach),
    DEVMETHOD(device_detach, cbm_detach),
    { 0, 0 }
};

static driver_t cbm_driver = {
    CBM_NAME,
    cbm_methods,
    sizeof(struct cbm_data)
};

#if __FreeBSD_version >= 1400058
DRIVER_MODULE(cbm, ppbus, cbm_driver, 0, 0);
#else
static devclass_t cbm_devclass;

DRIVER_MODULE(cbm, ppbus, cbm_driver, cbm_devclass, 0, 0);
#endif
MODULE_DEPEND(cbm, ppbus, 1, 1, 1);

/*
        And here are the functions, used by parallel burst
        (they are all called by the ioctl-function)
*/

int cbm_parallel_burst_read_track(struct cbm_data *sc, device_t ppbus,
        unsigned char *buffer)
{
        int i, byte;
        register_t saveintr;

        saveintr = intr_disable();

        for (i = 0; i < BUFFER_SIZE; i += 1) {
                byte = cbm_handshaked_read(sc, ppbus, i & 1);
                if (byte == -1) {
                        intr_restore(saveintr);
                        return 0;
                }
                buffer[i] = byte;
        }

        cbm_parallel_burst_read(sc, ppbus);
        intr_restore(saveintr);
        return 1;
}

int cbm_parallel_burst_read_track_var(struct cbm_data *sc, device_t ppbus,
        unsigned char *buffer)
{
        int i, byte;
        register_t saveintr;

        saveintr = intr_disable();

        for (i = 0; i < BUFFER_SIZE; i += 1) {
                byte = cbm_handshaked_read(sc, ppbus, i & 1);
                if (byte == -1) {
                        intr_restore(saveintr);
                        return 0;
                }
                buffer[i] = byte;
                if (byte == 0x55)
                        break;
        }

        cbm_parallel_burst_read(sc, ppbus);
        intr_restore(saveintr);
        return 1;
}

int cbm_parallel_burst_write_track(struct cbm_data *sc, device_t ppbus,
        unsigned char *buffer, int length)
{
        int i;
        register_t saveintr;

        saveintr = intr_disable();

        for (i = 0; i < length; i++) {
                if (cbm_handshaked_write(sc, ppbus, buffer[i], i & 1)) {
                        /* timeout */
                        intr_restore(saveintr);
                        return 0;
                }
        }
        cbm_handshaked_write(sc, ppbus, 0, i & 1);
        cbm_parallel_burst_read(sc, ppbus);
        intr_restore(saveintr);
        return 1;
}

unsigned char cbm_parallel_burst_read(struct cbm_data *sc, device_t ppbus)
{
        int rv = 0;

        RELEASE(DATA_OUT | CLK_OUT);
        SET(ATN_OUT);
        DELAY(20);              /* 200? */
        while (GET(DATA_IN)) ;
        /* linux rv = inportb(parport); */
        if (!sc->sc_data_reverse) {
                XP_WRITE(0xff);
                set_data_reverse();
        }
        rv = XP_READ();
        DELAY(5);
        RELEASE(ATN_OUT);
        DELAY(10);
        while (!GET(DATA_IN)) ;
        return rv;
}

int cbm_parallel_burst_write(struct cbm_data *sc, device_t ppbus,
        unsigned char c)
{
        RELEASE(DATA_OUT | CLK_OUT);
        SET(ATN_OUT);
        DELAY(20);
        while (GET(DATA_IN)) ;
        /* linux PARWRITE(); */
        if (sc->sc_data_reverse)
                set_data_forward();
        XP_WRITE(c);
        /* linux outportb(parport, arg); */
        DELAY(5);
        RELEASE(ATN_OUT);
        DELAY(20);
        while (!GET(DATA_IN)) ;
        /* linux PARREAD(); */
        if (!sc->sc_data_reverse) {
                XP_WRITE(0xff);
                set_data_reverse();
        }
        XP_READ();
        return 0;
}

#define TO_HANDSHAKED_READ  3300000
#define TO_HANDSHAKED_WRITE 3300000

int cbm_handshaked_read(struct cbm_data *sc, device_t ppbus, int toggle)
{
        static int oldvalue = -1;
        int returnvalue = 0;
        int returnvalue2, returnvalue3, timeoutcount;
        int to = 0;

        if (!toggle) {
                while (GET(DATA_IN))
                        if (to++ > TO_HANDSHAKED_READ)
                                return -1;
        } else {
                while (!GET(DATA_IN))
                        if (to++ > TO_HANDSHAKED_READ)
                                return -1;
        }

        timeoutcount = 0;

        returnvalue3 = XP_READ();
        returnvalue2 = ~returnvalue3;   /* ensure to read once more */

        do {
                if (++timeoutcount >= 8) {
                        device_printf(sc->sc_device,
                             "Triple-Debounce TIMEOUT: 0x%02x, 0x%02x, 0x%02x (%d, 0x%02x)\n",
                             returnvalue, returnvalue2, returnvalue3,
                             timeoutcount, oldvalue);
                        break;
                }
                returnvalue = returnvalue2;
                returnvalue2 = returnvalue3;
                returnvalue3 = XP_READ();
        } while ((returnvalue != returnvalue2)
                 || (returnvalue != returnvalue3));

        oldvalue = returnvalue;

        return returnvalue;
}

int cbm_handshaked_write(struct cbm_data *sc, device_t ppbus,
        char data, int toggle)
{
        int to = 0;

        if (!toggle) {
                while (GET(DATA_IN))
                        if (to++ > TO_HANDSHAKED_WRITE)
                                return 1;
        } else {
                while (!GET(DATA_IN))
                        if (to++ > TO_HANDSHAKED_WRITE)
                                return 1;
        }
        /* linux outportb(parport, data); */
        if (sc->sc_data_reverse)
                set_data_forward();
        XP_WRITE(data);
        return 1;
}
