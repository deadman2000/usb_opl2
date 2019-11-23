/*
* USBTest.c
*
* Created: 01.02.2019 22:20:07
*  Author: DEADMAN
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"

#include "OPL.h"

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

#define BUFF_SIZE 16

struct command_t
{
    uchar address;
    uchar data;
};

struct dataexchange_t
{
    uchar size;
    struct command_t commands[BUFF_SIZE];
};


struct dataexchange_t pdata;


PROGMEM const char usbHidReportDescriptor[] = { // USB report descriptor
    0x06, 0x00, 0xff,                       // USAGE_PAGE (Vendor Defined Page)
    0x09, 0x01,                             // USAGE (Vendor Usage 1)
    0xa1, 0x01,                             // COLLECTION (Application)
    0x15, 0x00,                             //    LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,                       //    LOGICAL_MAXIMUM (255)
    0x75, 0x08,                             //    REPORT_SIZE (8)
    0x95, sizeof(struct dataexchange_t),    //    REPORT_COUNT
    0x09, 0x00,                             //    USAGE (Undefined)
    0xb2, 0x02, 0x01,                       //    FEATURE (Data,Var,Abs,Buf)
    0xc0                                    // END_COLLECTION
};

static uchar    currentAddress;
static uchar    bytesRemaining;

uchar usbFunctionWrite(uchar *data, uchar len)
{
    if (bytesRemaining == 0)
        return 1;

    if (len > bytesRemaining)
        len = bytesRemaining;

    uchar *buffer = (uchar*)&pdata;
    
    memcpy(buffer + currentAddress, data, len);

    currentAddress += len;
    bytesRemaining -= len;
    
    if (bytesRemaining == 0)
    {
        for (int i = 0; i < pdata.size; ++i) {
            struct command_t cmd = pdata.commands[i];
            if (cmd.address == 0xff && cmd.data == 0xff)
                opl_reset();
            else
                opl_write(cmd.address, cmd.data);
        }            
    }

    return bytesRemaining == 0;
}

extern void hadUsbReset(void) {
    opl_reset();
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
    usbRequest_t *rq = (void*)data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        if (rq->bRequest == USBRQ_HID_GET_REPORT){
            bytesRemaining = sizeof(struct dataexchange_t);
            currentAddress = 0;
            return USB_NO_MSG;
        } else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
            bytesRemaining = sizeof(struct dataexchange_t);
            currentAddress = 0;
            return USB_NO_MSG;
        }
    } else {
        /* no vendor specific requests implemented */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void)
{
    opl_init();
    
    DDRD = 0xFF;
    
    usbInit();
    usbDeviceDisconnect();
    
    PORTD |= (1 << DDD0);
    _delay_ms(250);
    PORTD &= ~(1 << DDD0);

    usbDeviceConnect();
    sei();
    for (;;) {                /* main event loop */
        usbPoll();
    }
}