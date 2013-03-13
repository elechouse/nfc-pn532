/*****************************************************************************/
/*!
    @file     nfc.h
    @author   www.elechouse.com
	@brief      NFC Module I2C library header file.
	This is a library for the Elechoues NFC_Module
	----> LINKS HERE!!!

    NOTE:
        1. IRQ pin is unused.
        2. Referenced Adafruit_NFCShield_I2C library
	@section  HISTORY
    V1.1    Add fuction about Peer to Peer communication
            u8 P2PInitiatorInit();
            u8 P2PTargetInit();
            u8 P2PInitiatorTxRx(u8 *t_buf, u8 t_len, u8 *r_buf, u8 *r_len);
            u8 P2PTargetTxRx(u8 *t_buf, u8 t_len, u8 *r_buf, u8 *r_len);

            Change wait_ready(void) to wait_ready(u8 ms=NFC_WAIT_TIME);

    V1.0    Initial version.

    Copyright (c) 2012 www.elechouse.com  All right reserved.
*/
/*****************************************************************************/

#ifndef __NFC_H
#define __NFC_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include <avr/pgmspace.h>
#include <Wire.h>

#ifndef __TYPE_REDEFINE
#define __TYPE_REDEFINE
typedef uint8_t u8;
typedef int8_t  s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
#endif


#define PN532_PREAMBLE                      (0x00)
#define PN532_STARTCODE1                    (0x00)
#define PN532_STARTCODE2                    (0xFF)
#define PN532_POSTAMBLE                     (0x00)

#define PN532_HOSTTOPN532                   (0xD4)

// PN532 Commands
#define PN532_COMMAND_DIAGNOSE              (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_GETGENERALSTATUS      (0x04)
#define PN532_COMMAND_READREGISTER          (0x06)
#define PN532_COMMAND_WRITEREGISTER         (0x08)
#define PN532_COMMAND_READGPIO              (0x0C)
#define PN532_COMMAND_WRITEGPIO             (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE     (0x10)
#define PN532_COMMAND_SETPARAMETERS         (0x12)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_POWERDOWN             (0x16)
#define PN532_COMMAND_RFCONFIGURATION       (0x32)
#define PN532_COMMAND_RFREGULATIONTEST      (0x58)
#define PN532_COMMAND_INJUMPFORDEP          (0x56)
#define PN532_COMMAND_INJUMPFORPSL          (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_INATR                 (0x50)
#define PN532_COMMAND_INPSL                 (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU     (0x42)
#define PN532_COMMAND_INDESELECT            (0x44)
#define PN532_COMMAND_INRELEASE             (0x52)
#define PN532_COMMAND_INSELECT              (0x54)
#define PN532_COMMAND_INAUTOPOLL            (0x60)
#define PN532_COMMAND_TGINITASTARGET        (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES     (0x92)
#define PN532_COMMAND_TGGETDATA             (0x86)
#define PN532_COMMAND_TGSETDATA             (0x8E)
#define PN532_COMMAND_TGSETMETADATA         (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS     (0x8A)

#define PN532_WAKEUP                        (0x55)

#define PN532_SPI_STATREAD                  (0x02)
#define PN532_SPI_DATAWRITE                 (0x01)
#define PN532_SPI_DATAREAD                  (0x03)
#define PN532_SPI_READY                     (0x01)

#define PN532_I2C_ADDRESS                   (0x48 >> 1)
#define PN532_I2C_READBIT                   (0x01)
#define PN532_I2C_BUSY                      (0x00)
#define PN532_I2C_READY                     (0x01)
#define PN532_I2C_READYTIMEOUT              (20)

#define PN532_MIFARE_ISO14443A              (0x00)

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_RESTORE                  (0xC2)

// Prefixes for NDEF Records (to identify record type)
#define NDEF_URIPREFIX_NONE                 (0x00)
#define NDEF_URIPREFIX_HTTP_WWWDOT          (0x01)
#define NDEF_URIPREFIX_HTTPS_WWWDOT         (0x02)
#define NDEF_URIPREFIX_HTTP                 (0x03)
#define NDEF_URIPREFIX_HTTPS                (0x04)
#define NDEF_URIPREFIX_TEL                  (0x05)
#define NDEF_URIPREFIX_MAILTO               (0x06)
#define NDEF_URIPREFIX_FTP_ANONAT           (0x07)
#define NDEF_URIPREFIX_FTP_FTPDOT           (0x08)
#define NDEF_URIPREFIX_FTPS                 (0x09)
#define NDEF_URIPREFIX_SFTP                 (0x0A)
#define NDEF_URIPREFIX_SMB                  (0x0B)
#define NDEF_URIPREFIX_NFS                  (0x0C)
#define NDEF_URIPREFIX_FTP                  (0x0D)
#define NDEF_URIPREFIX_DAV                  (0x0E)
#define NDEF_URIPREFIX_NEWS                 (0x0F)
#define NDEF_URIPREFIX_TELNET               (0x10)
#define NDEF_URIPREFIX_IMAP                 (0x11)
#define NDEF_URIPREFIX_RTSP                 (0x12)
#define NDEF_URIPREFIX_URN                  (0x13)
#define NDEF_URIPREFIX_POP                  (0x14)
#define NDEF_URIPREFIX_SIP                  (0x15)
#define NDEF_URIPREFIX_SIPS                 (0x16)
#define NDEF_URIPREFIX_TFTP                 (0x17)
#define NDEF_URIPREFIX_BTSPP                (0x18)
#define NDEF_URIPREFIX_BTL2CAP              (0x19)
#define NDEF_URIPREFIX_BTGOEP               (0x1A)
#define NDEF_URIPREFIX_TCPOBEX              (0x1B)
#define NDEF_URIPREFIX_IRDAOBEX             (0x1C)
#define NDEF_URIPREFIX_FILE                 (0x1D)
#define NDEF_URIPREFIX_URN_EPC_ID           (0x1E)
#define NDEF_URIPREFIX_URN_EPC_TAG          (0x1F)
#define NDEF_URIPREFIX_URN_EPC_PAT          (0x20)
#define NDEF_URIPREFIX_URN_EPC_RAW          (0x21)
#define NDEF_URIPREFIX_URN_EPC              (0x22)
#define NDEF_URIPREFIX_URN_NFC              (0x23)

#define PN532_GPIO_VALIDATIONBIT            (0x80)
#define PN532_GPIO_P30                      (0)
#define PN532_GPIO_P31                      (1)
#define PN532_GPIO_P32                      (2)
#define PN532_GPIO_P33                      (3)
#define PN532_GPIO_P34                      (4)
#define PN532_GPIO_P35                      (5)

#define PN532_SAM_NORMAL_MODE               (0x01)
#define PN532_SAM_VIRTUAL_CARD              (0x02)
#define PN532_SAM_WIRED_CARD                (0x03)
#define PN532_SAM_DUAL_CARD                 (0x04)

#define PN532_BRTY_ISO14443A                0x00
#define PN532_BRTY_ISO14443B                0x03
#define PN532_BRTY_212KBPS                  0x01
#define PN532_BRTY_424KBPS                  0x02
#define PN532_BRTY_JEWEL                    0x04

//#define PN532DEBUG
//#define PN532_P2P_DEBUG
#define NFC_WAIT_TIME                       30
#define NFC_CMD_BUF_LEN                     64
#define NFC_FRAME_ID_INDEX                  6

typedef enum{
    NFC_STA_TAG,
    NFC_STA_GETDATA,
    NFC_STA_SETDATA,
}poll_sta_type;

class NFC_Module{
public:
    NFC_Module();
    void begin(void);
    u32 get_version(void);
    u8 SAMConfiguration(u8 mode=PN532_SAM_NORMAL_MODE, u8 timeout=20, u8 irq=0);

    u8 InListPassiveTarget(u8 *buf, u8 brty=PN532_BRTY_ISO14443A,
                            u8 len=0, u8 *idata=NULL, u8 maxtg=1);
    u8 InDataExchange(u8 mode, u8 tg, u8 *buf=NULL, u8 len=0);
    u8 MifareAuthentication(u8 type, u8 block, u8 *uuid, u8 uuid_len, u8 *key);
    u8 MifareReadBlock(u8 block, u8 *buf);
    u8 MifareWriteBlock(u8 block, u8 *buf);

    u8 P2PInitiatorInit();
    u8 P2PTargetInit();
    u8 P2PInitiatorTxRx(u8 *t_buf, u8 t_len, u8 *r_buf, u8 *r_len);
    u8 P2PTargetTxRx(u8 *t_buf, u8 t_len, u8 *r_buf, u8 *r_len);

    u8 TgInitAsTarget();
    u8 TargetPolling();
    u8 SetParameters(u8 para);

	u8 FelicaPoll(u8 *buf, u8 len, u8 *idata);
	
    void puthex(u8 *buf, u32 len);
    void puthex(u8 data);
private:

	inline u8 send(u8 data);
	inline u8 receive();

	void write_cmd(u8 *cmd, u8 len);
	u8 write_cmd_check_ack(u8 *cmd, u8 len);
	void read_dt(u8 *buf, u8 len);
	u8 read_sta(void);
	u8 wait_ready(u8 ms=NFC_WAIT_TIME);
	u8 read_ack(void);
};

#endif /** __NFC_H */
