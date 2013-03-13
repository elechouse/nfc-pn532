/*****************************************************************************/
/*!
    @file     nfc.h
    @author   www.elechouse.com
	@brief      NFC Module I2C library source file.
	This is a library for the Elechoues NFC_Module
	----> LINKS HERE!!!

    NOTE:
        IRQ pin is unused.

	@section  HISTORY
    V1.1    Add fuction about Peer to Peer communication
                u8 P2PInitiatorInit();
                u8 P2PTargetInit();
                u8 P2PInitiatorTxRx(u8 *t_buf, u8 t_len, u8 *r_buf, u8 *r_len);
                u8 P2PTargetTxRx(u8 *t_buf, u8 t_len, u8 *r_buf, u8 *r_len);
            Change wait_ready(void) to wait_ready(u8 ms=NFC_WAIT_TIME);
            Attach Wire library with NFC_MODULE, modify I2C buffer length to 64
                and change i2c speed to 400KHz

    V1.0    Initial version.

    Copyright (c) 2012 www.elechouse.com  All right reserved.
*/
/*****************************************************************************/

#include "nfc.h"

u8 hextab[17]="0123456789ABCDEF";
u8 ack[6]={
    0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00
};
u8 nfc_version[6]={
    0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03
};

/** data buffer */
u8 nfc_buf[NFC_CMD_BUF_LEN];

/*****************************************************************************/
/*!
	@brief
	@param
*/
/*****************************************************************************/
NFC_Module::NFC_Module(void)
{

}

/*****************************************************************************/
/*!
	@brief  initial function.
	@param  NONE.
	@return NONE.
*/
/*****************************************************************************/
void NFC_Module::begin(void)
{
#ifdef PN532DEBUG
    u8 i;
#endif
    Wire.begin();
#ifdef PN532DEBUG
    for(i=0; i<16; i++){
        Serial.write((u8)hextab[i]);
    }
    for(i=0; i<6; i++){
        Serial.write((u8)ack[i]);
    }
    for(i=0; i<6; i++){
        Serial.write((u8)nfc_version[i]);
    }
#endif
}

/*****************************************************************************/
/*!
	@brief  Get version of PN532
	@param  NONE
	@return version number.
*/
/*****************************************************************************/
u32 NFC_Module::get_version(void)
{
    u32 version;

    nfc_buf[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    if(!write_cmd_check_ack(nfc_buf, 1)){
        return 0;
    }
    wait_ready();
    read_dt(nfc_buf, 12);
    if(nfc_buf[5] != 0xD5){
        return 0;
    }
    // check some basic stuff
	if (0 != strncmp((char *)nfc_buf, (char *)nfc_version, 6)) {
#ifdef PN532DEBUG
		Serial.println("Firmware doesn't match!");
#endif
		return 0;
	}

	version = nfc_buf[7];
	version <<= 8;
	version |= nfc_buf[8];
	version <<= 8;
	version |= nfc_buf[9];
	version <<= 8;
	version |= nfc_buf[10];

	return version;
}

/*****************************************************************************/
/*!
	@brief  Configures the SAM (Secure Access Module)
	@param  mode - set mode, normal mode default
	@param  timeout - Details in NXP's PN532UM.pdf
	@param  irq - 0 unused (default), 1 used
	@return 0 - failed
            1 - successfully
*/
/*****************************************************************************/
u8 NFC_Module::SAMConfiguration(u8 mode, u8 timeout, u8 irq)
{
#ifdef PN532DEBUG
    Serial.print("SAMConfiguration\n");
#endif
	nfc_buf[0] = PN532_COMMAND_SAMCONFIGURATION;
	nfc_buf[1] = mode; // normal mode;
	nfc_buf[2] = timeout; // timeout 50ms * 20 = 1 second
	nfc_buf[3] = irq; // use IRQ pin!

    if(!write_cmd_check_ack(nfc_buf, 4)){
        return 0;
    }

	// read data packet
	read_dt(nfc_buf, 8);

	return  (nfc_buf[6] == PN532_COMMAND_SAMCONFIGURATION);
}

/*****************************************************************************/
/*!
	@brief card inventory.
	@param  buf - buf[0] UUID length; buf[1], buf[2], buf[3] buf[4] UUID
	@param  brty - optional parameter, braud rate:  PN532_BRTY_ISO14443A,
                                                    PN532_BRTY_ISO14443B,
                                                    PN532_BRTY_212KBPS,
                                                    PN532_BRTY_424KBPS,
                                                    PN532_BRTY_JEWEL,
	@param  maxtg - optional parameter, maximum tag numbers to read once,
        maximum 2, recommend 1.
	@param  idata - assistant parameter, unused.
	@return 0 - failed
            1 - successfully
*/
/*****************************************************************************/
u8 NFC_Module::InListPassiveTarget(u8 *buf, u8 brty,
                                   u8 len, u8 *idata, u8 maxtg)
{
    nfc_buf[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    nfc_buf[1] = maxtg;
    nfc_buf[2] = brty;
    if(len){
        memcpy(nfc_buf+3, idata, len);
    }
    if(!write_cmd_check_ack(nfc_buf, 3+len)){
        return 0;
    }
#ifdef PN532DEBUG
    puthex(nfc_buf, 3+len);
    Serial.println();
#endif
//	puthex(nfc_buf, 3+len);
//    Serial.println();
	
    /** "Waiting for IRQ (indicates card presence)" */
    wait_ready();
	wait_ready();
	wait_ready();
#ifdef PN532DEBUG
	Serial.print(" Found Card.\n");
#endif
    read_dt(nfc_buf,40);
//	puthex(nfc_buf, nfc_buf[3]+6);
//    Serial.println();
    if(nfc_buf[NFC_FRAME_ID_INDEX-1] != 0xD5){
        return 0;
    }
	
    puthex(nfc_buf, nfc_buf[3]+6);
    Serial.println();

    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INLISTPASSIVETARGET+1)){
        return 0;
    }
//    if(nfc_buf[NFC_FRAME_ID_INDEX+1]!=1){
//#ifdef PN532DEBUG
//        Serial.println(nfc_buf[NFC_FRAME_ID_INDEX+1],DEC);
//#endif
//        return 0;
//    }
    if(brty == PN532_BRTY_ISO14443A){
        /** UUID length */
        buf[0] = nfc_buf[12];

        for(u8 i=1; i<5; i++){
            buf[i] = nfc_buf[12+i];
        }
    }else{
        buf[0] = nfc_buf[3];
        memcpy(buf, nfc_buf+5, nfc_buf[3]);
    }

    return 1;
}

u8 NFC_Module::FelicaPoll(u8 *buf, u8 len, u8 *idata)
{
	static u8 sta=0;
	if(!sta){
		nfc_buf[0] = PN532_COMMAND_INLISTPASSIVETARGET;
		nfc_buf[1] = 0x02;
		nfc_buf[2] = 0x02;
		if(len){
			memcpy(nfc_buf+3, idata, len);
		}
		if(!write_cmd_check_ack(nfc_buf, 3+len)){
			return 0;
		}
#ifdef PN532DEBUG
		puthex(nfc_buf, 3+len);
		Serial.println();
#endif
	//	puthex(nfc_buf, 3+len);
	    Serial.println("Send command");
	}
    sta = 1;
    /** "Waiting for IRQ (indicates card presence)" */
    wait_ready();
	wait_ready();
	wait_ready();
#ifdef PN532DEBUG
	Serial.print(" Found Card.\n");
#endif
    read_dt(nfc_buf,40);
	puthex(nfc_buf, nfc_buf[3]+6);
    Serial.println();
    if(nfc_buf[NFC_FRAME_ID_INDEX-1] != 0xD5){
        return 0;
    }
	sta = 0;
    puthex(nfc_buf, nfc_buf[3]+6);
    Serial.println();

    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INLISTPASSIVETARGET+1)){
        return 0;
    }
//    if(nfc_buf[NFC_FRAME_ID_INDEX+1]!=1){
//#ifdef PN532DEBUG
//        Serial.println(nfc_buf[NFC_FRAME_ID_INDEX+1],DEC);
//#endif
//        return 0;
//    }
    buf[0] = nfc_buf[3];
    memcpy(buf, nfc_buf+5, nfc_buf[3]);


    return 1;
}

/*****************************************************************************/
/*!
	@brief  Mifare funciton. Authentication a block for more operation.
	@param  type - key type. 0-KEYA, 1-KEYB
	@param  block - block to Authentication
	@param  uuid - pointer to selected card's UUID
	@param  uuid_len - UUID length
	@param  key - pointer to key buffer.
	@return 0 - failed
            1 - successfully
*/
/*****************************************************************************/
u8 NFC_Module::MifareAuthentication(u8 type, u8 block,
                                    u8 *uuid, u8 uuid_len, u8 *key)
{
    u8 i;
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 1; // logical number of the relevant target
    nfc_buf[2] = MIFARE_CMD_AUTH_A+type;
    nfc_buf[3] = block;

    for(i=0; i<6; i++){
        nfc_buf[4+i] = key[i];
    }
    for(i=0; i<uuid_len; i++){
        nfc_buf[10+i] = uuid[i];
    }

    if(!write_cmd_check_ack( nfc_buf, (10+uuid_len) )){
        return 0;
    }

    wait_ready();
    read_dt(nfc_buf, 8);
#if 0
    if(nfc_buf[5] == 0xD5){
        Serial.print("Authentication receive:");
        puthex(nfc_buf, nfc_buf[3]+6);
        Serial.println();
    }
#endif
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INDATAEXCHANGE+1)){
#ifdef PN532DEBUG
        puthex(nfc_buf, 20);
        Serial.println("Authentication fail.");
#endif
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
        return 0;
    }
    return 1;
}

/*****************************************************************************/
/*!
	@brief  Mifare funciton. Read a block.
	@param  block - block to read
	@param  buf - pointer to data buffer.
	@return 0 - failed
            1 - successfully
*/
/*****************************************************************************/
u8 NFC_Module::MifareReadBlock(u8 block, u8 *buf)
{
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 1; // logical number of the relevant target
    nfc_buf[2] = MIFARE_CMD_READ;
    nfc_buf[3] = block;

    if(!write_cmd_check_ack(nfc_buf, 4)){
        return 0;
    }
    wait_ready();
    read_dt(nfc_buf, 26);
/**
    if(nfc_buf[5] == 0xD5){
        Serial.print("Block receive:");
        puthex(nfc_buf, nfc_buf[3]+6);
        Serial.println();
    }
*/
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INDATAEXCHANGE+1)){
#ifdef PN532DEBUG
        puthex(nfc_buf, 20);
        Serial.println("Authentication fail.");
#endif
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
        return 0;
    }

    memcpy(buf, nfc_buf+8, 16);

    return 1;
}

/*****************************************************************************/
/*!
	@brief  Mifare funciton. Write to a block.
	@param  block - block to write
	@param  buf - pointer to data buffer.
	@return 0 - failed
            1 - successfully
*/
/*****************************************************************************/
u8 NFC_Module::MifareWriteBlock(u8 block, u8 *buf)
{
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 1; // logical number of the relevant target
    nfc_buf[2] = MIFARE_CMD_WRITE;
    nfc_buf[3] = block;

    memcpy(nfc_buf+4, buf, 16);

    if(!write_cmd_check_ack(nfc_buf, 20)){
        return 0;
    }
    wait_ready();
    read_dt(nfc_buf, 26);
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INDATAEXCHANGE+1)){
#ifdef PN532DEBUG
        puthex(nfc_buf, 20);
        Serial.println("Authentication fail.");
#endif
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
        return 0;
    }
    return 1;
}

/*****************************************************************************/
/*!
	@brief  Configure PN532 as Initiator
	@param  NONE
	@return 0 - failed
            1 - successfully
*/
/*****************************************************************************/
u8 NFC_Module::P2PInitiatorInit()
{
    /** avoid resend command */
    static u8 send_flag=1;
    nfc_buf[0] = PN532_COMMAND_INJUMPFORDEP;
    nfc_buf[1] = 0x01; // avtive mode
    nfc_buf[2] = 0x02; // 201Kbps
    nfc_buf[3] = 0x01;

    nfc_buf[4] = 0x00;
    nfc_buf[5] = 0xFF;
    nfc_buf[6] = 0xFF;
    nfc_buf[7] = 0x00;
    nfc_buf[8] = 0x00;

    if(send_flag){
        send_flag = 0;
        if(!write_cmd_check_ack(nfc_buf, 9)){
#ifdef PN532_P2P_DEBUG
            Serial.println("InJumpForDEP sent fialed\n");
#endif
            return 0;
        }
#ifdef PN532_P2P_DEBUG
        Serial.println("InJumpForDEP sent ******\n");
#endif
    }
    wait_ready(10);
    read_dt(nfc_buf, 25);

    if(nfc_buf[5] != 0xD5){
//        Serial.println("InJumpForDEP sent read failed");
        return 0;
    }

    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INJUMPFORDEP+1)){
#ifdef PN532_P2P_DEBUG
        puthex(nfc_buf, nfc_buf[3]+7);
        Serial.println("Initiator init failed");
#endif
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
        return 0;
    }

#ifdef PN532_P2P_DEBUG
    Serial.println("InJumpForDEP read success");
#endif
    send_flag = 1;
    return 1;
}

/*****************************************************************************/
/*!
	@brief  Configure PN532 as Target.
	@param  NONE.
	@return 0 - failed
            1 - successfully
*/
/*****************************************************************************/
u8 NFC_Module::P2PTargetInit()
{
    /** avoid resend command */
    static u8 send_flag=1;
    nfc_buf[0] = PN532_COMMAND_TGINITASTARGET;
    /** 14443-4A Card only */
    nfc_buf[1] = 0x00;

    /** SENS_RES */
    nfc_buf[2] = 0x04;
    nfc_buf[3] = 0x00;

    /** NFCID1 */
    nfc_buf[4] = 0x12;
    nfc_buf[5] = 0x34;
    nfc_buf[6] = 0x56;

    /** SEL_RES */
    nfc_buf[7] = 0x40;      // DEP only mode

    /**Parameters to build POL_RES (18 bytes including system code) */
    nfc_buf[8] = 0x01;
    nfc_buf[9] = 0xFE;
    nfc_buf[10] = 0xA2;
    nfc_buf[11] = 0xA3;
    nfc_buf[12] = 0xA4;
    nfc_buf[13] = 0xA5;
    nfc_buf[14] = 0xA6;
    nfc_buf[15] = 0xA7;
    nfc_buf[16] = 0xC0;
    nfc_buf[17] = 0xC1;
    nfc_buf[18] = 0xC2;
    nfc_buf[19] = 0xC3;
    nfc_buf[20] = 0xC4;
    nfc_buf[21] = 0xC5;
    nfc_buf[22] = 0xC6;
    nfc_buf[23] = 0xC7;
    nfc_buf[24] = 0xFF;
    nfc_buf[25] = 0xFF;
    /** NFCID3t */
    nfc_buf[26] = 0xAA;
    nfc_buf[27] = 0x99;
    nfc_buf[28] = 0x88;
    nfc_buf[29] = 0x77;
    nfc_buf[30] = 0x66;
    nfc_buf[31] = 0x55;
    nfc_buf[32] = 0x44;
    nfc_buf[33] = 0x33;
    nfc_buf[34] = 0x22;
    nfc_buf[35] = 0x11;
    /** Length of general bytes  */
    nfc_buf[36] = 0x00;
    /** Length of historical bytes  */
    nfc_buf[37] = 0x00;

    if(send_flag){
        send_flag = 0;
        if(!write_cmd_check_ack(nfc_buf, 38)){
            send_flag = 1;
            return 0;
        }
#ifdef PN532_P2P_DEBUG
        Serial.println("Target init sent.");
#endif
    }

    wait_ready(10);
    read_dt(nfc_buf, 24);

    if(nfc_buf[5] != 0xD5){
        return 0;
    }

    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_TGINITASTARGET+1)){
#ifdef PN532_P2P_DEBUG
        puthex(nfc_buf, nfc_buf[3]+7);
        Serial.println("Target init fail.");
#endif
        return 0;
    }

    send_flag = 1;
#ifdef PN532_P2P_DEBUG
    Serial.println("TgInitAsTarget read success");
#endif
    return 1;
}

/*****************************************************************************/
/*!
	@brief  Initiator send and reciev data.
    @param  tx_buf --- data send buffer, user sets
            tx_len --- data send legth, user sets.
            rx_buf --- data recieve buffer, returned by P2PInitiatorTxRx
            rx_len --- data receive length, returned by P2PInitiatorTxRx
	@return 0 - send failed
            1 - send successfully
*/
/*****************************************************************************/
u8 NFC_Module::P2PInitiatorTxRx(u8 *t_buf, u8 t_len, u8 *r_buf, u8 *r_len)
{
//    wait_ready();
//    wait_ready();
    wait_ready(15);
    nfc_buf[0] = PN532_COMMAND_INDATAEXCHANGE;
    nfc_buf[1] = 0x01; // logical number of the relevant target

    memcpy(nfc_buf+2, t_buf, t_len);

    if(!write_cmd_check_ack(nfc_buf, t_len+2)){
        return 0;
    }
#ifdef PN532_P2P_DEBUG
    Serial.println("Initiator DataExchange sent.");
#endif

    wait_ready(200);

    read_dt(nfc_buf, 60);
    if(nfc_buf[5] != 0xD5){
        return 0;
    }

#ifdef PN532_P2P_DEBUG
    Serial.println("Initiator DataExchange Get.");
#endif

    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_INDATAEXCHANGE+1)){
#ifdef PN532_P2P_DEBUG
        puthex(nfc_buf, nfc_buf[3]+7);
        Serial.println("Send data failed");
#endif
        return 0;
    }

    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
#ifdef PN532_P2P_DEBUG
        Serial.print("InExchangeData Error:");
        puthex(nfc_buf, nfc_buf[3]+7);
        Serial.println();
#endif
        return 0;
    }

#ifdef PN532_P2P_DEBUG
    puthex(nfc_buf, nfc_buf[3]+7);
    Serial.println();
#endif
    /** return read data */
    *r_len = nfc_buf[3]-3;
    memcpy(r_buf, nfc_buf+8, *r_len);
    return 1;
}

/*****************************************************************************/
/*!
	@brief  Target sends and recievs data.
    @param  tx_buf --- data send buffer, user sets
            tx_len --- data send legth, user sets.
            rx_buf --- data recieve buffer, returned by P2PInitiatorTxRx
            rx_len --- data receive length, returned by P2PInitiatorTxRx
	@return 0 - send failed
            1 - send successfully
*/
/*****************************************************************************/
u8 NFC_Module::P2PTargetTxRx(u8 *t_buf, u8 t_len, u8 *r_buf, u8 *r_len)
{
    nfc_buf[0] = PN532_COMMAND_TGGETDATA;
    if(!write_cmd_check_ack(nfc_buf, 1)){
        return 0;
    }
    wait_ready(100);
    read_dt(nfc_buf, 60);
    if(nfc_buf[5] != 0xD5){
        return 0;
    }

    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_TGGETDATA+1)){
#ifdef PN532_P2P_DEBUG
        puthex(nfc_buf, 20);
        Serial.println("Target GetData failed");
#endif
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
 #ifdef PN532_P2P_DEBUG
        Serial.print("TgGetData Error:");
        puthex(nfc_buf, nfc_buf[3]+7);
        Serial.println();
#endif
        return 0;
    }

#ifdef PN532_P2P_DEBUG
    Serial.println("TgGetData:");
    puthex(nfc_buf, nfc_buf[3]+7);
    Serial.println();
#endif

    /** return read data */
    *r_len = nfc_buf[3]-3;
    memcpy(r_buf, nfc_buf+8, *r_len);

    nfc_buf[0] = PN532_COMMAND_TGSETDATA;
    memcpy(nfc_buf+1, t_buf, t_len);

    if(!write_cmd_check_ack(nfc_buf, 1+t_len)){
        return 0;
    }
    wait_ready(100);
    read_dt(nfc_buf, 26);

    if(nfc_buf[5] != 0xD5){
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_TGSETDATA+1)){
#ifdef PN532_P2P_DEBUG
        puthex(nfc_buf, 20);
        Serial.println("Send data failed");
#endif
        return 0;
    }
    if(nfc_buf[NFC_FRAME_ID_INDEX+1]){
        return 0;
    }

    return 1;
}



/*****************************************************************************/
/*!
	@brief  Unfinished
	@param
	@return 0 - send failed
            1 - send successfully
*/
/*****************************************************************************/
u8 NFC_Module::TgInitAsTarget()
{
    nfc_buf[0] = PN532_COMMAND_TGINITASTARGET;
    /** 14443-4A Card only */
    nfc_buf[1] = 0x04;

    /** SENS_RES */
    nfc_buf[2] = 0x04;
    nfc_buf[3] = 0x00;

    /** NFCID1 */
    nfc_buf[4] = 0x12;
    nfc_buf[5] = 0x34;
    nfc_buf[6] = 0x56;

    /** SEL_RES */
    nfc_buf[7] = 0x60;

    /**Parameters to build POL_RES (18 bytes including system code) */
    nfc_buf[8] = 0x01;
    nfc_buf[9] = 0xFE;
    nfc_buf[10] = 0xA2;
    nfc_buf[11] = 0xA3;
    nfc_buf[12] = 0xA4;
    nfc_buf[13] = 0xA5;
    nfc_buf[14] = 0xA6;
    nfc_buf[15] = 0xA7;
    nfc_buf[16] = 0xC0;
    nfc_buf[17] = 0xC1;
    nfc_buf[18] = 0xC2;
    nfc_buf[19] = 0xC3;
    nfc_buf[20] = 0xC4;
    nfc_buf[21] = 0xC5;
    nfc_buf[22] = 0xC6;
    nfc_buf[23] = 0xC7;
    nfc_buf[24] = 0xFF;
    nfc_buf[25] = 0xFF;
    /** NFCID3t */
    nfc_buf[26] = 0xAA;
    nfc_buf[27] = 0x99;
    nfc_buf[28] = 0x88;
    nfc_buf[29] = 0x77;
    nfc_buf[30] = 0x66;
    nfc_buf[31] = 0x55;
    nfc_buf[32] = 0x44;
    nfc_buf[33] = 0x33;
    nfc_buf[34] = 0x22;
    nfc_buf[35] = 0x11;
    /** Length of general bytes  */
    nfc_buf[36] = 0x00;
    /** Length of historical bytes  */
    nfc_buf[37] = 0x00;

    if(!write_cmd_check_ack(nfc_buf, 38)){
        return 0;
    }

    return 1;
}

/*****************************************************************************/
/*!
	@brief  Unfinished
	@param
	@return
*/
/*****************************************************************************/
u8 NFC_Module::TargetPolling()
{
    static poll_sta_type sta;
    static u8 count=0;
    wait_ready();
    read_dt(nfc_buf, 30);
#ifdef PN532DEBUG
    delay(300);
    puthex(nfc_buf, 9);
    Serial.println();
#endif
    switch(sta){
        case NFC_STA_TAG:
            break;
        case NFC_STA_GETDATA:
            count++;
            if(count > 10){
                Serial.println("*************Reinit Target************");
                /** Target release */
                TgInitAsTarget();
                Serial.println("*************Reinit Target************");
                sta = NFC_STA_TAG;
            }
            break;
        case NFC_STA_SETDATA:
            break;
    }
    if(nfc_buf[5] == 0xD5){
        puthex(nfc_buf, nfc_buf[3]+6);
        switch(nfc_buf[NFC_FRAME_ID_INDEX]){
            case PN532_COMMAND_TGINITASTARGET+1:
                Serial.println("TgInitTatget");
                nfc_buf[0] = PN532_COMMAND_TGGETDATA;
                sta = NFC_STA_GETDATA;
                count=0;
                if(!write_cmd_check_ack(nfc_buf, 1)){
                    return 0;
                }
                break;
            case PN532_COMMAND_TGRESPONSETOINITIATOR+1:
                Serial.println("TgResponseToInitiator");
                break;
            case PN532_COMMAND_TGGETINITIATORCOMMAND+1:
                Serial.println("TgGetInitiatorCommand");
                break;
            case PN532_COMMAND_TGGETDATA+1:
                Serial.println("TgGetData");
                if(!nfc_buf[NFC_FRAME_ID_INDEX+1]){
                    Serial.println("TgGetData Success.");
                    switch(nfc_buf[NFC_FRAME_ID_INDEX+2]){
                        case 0x60:
                            nfc_buf[0] = PN532_COMMAND_TGSETDATA;
                            nfc_buf[1] = 0x00;
                            nfc_buf[2] = 0xFF;
                            if(!write_cmd_check_ack(nfc_buf, 3)){
                                return 0;
                            }
                            break;
                        case 0x30:
                            nfc_buf[0] = PN532_COMMAND_TGSETDATA;
                            for(u8 i=1; i<17; i++){
                                nfc_buf[i] = i;
                            }
                            if(!write_cmd_check_ack(nfc_buf, 17)){
                                return 0;
                            }
                            break;
                        default:
                            Serial.println("TgGetData Error.");
                            wait_ready();
                            /** Target release */
                            TgInitAsTarget();
                            break;
                    }

                }else{
                    /** Target release */
                    TgInitAsTarget();
                }
                break;
            case PN532_COMMAND_TGSETDATA+1:
                Serial.println("TgSetData");
                if(!nfc_buf[NFC_FRAME_ID_INDEX+1]){
                    Serial.println("TgSetData Success.");
                    nfc_buf[0] = PN532_COMMAND_TGGETDATA;
                    sta = NFC_STA_GETDATA;
                    count=0;
                    if(!write_cmd_check_ack(nfc_buf, 1)){
                        /**  */
                        Serial.println("SetData check error.");
                        return 0;
                    }
                }
                break;
            default:
                Serial.println("Undifined command");
                break;
        }
    }else{
        Serial.println(".");
    }
    return 1;
}

/*****************************************************************************/
/*!
	@brief  PN532 SetParameters command. Details in NXP's PN532UM.pdf
	@param  para - parameter to set
	@return 0 - send failed
            1 - send successfully
*/
/*****************************************************************************/
u8 NFC_Module::SetParameters(u8 para)
{
    nfc_buf[0] = PN532_COMMAND_SETPARAMETERS;
    nfc_buf[1] = para;

    if(!write_cmd_check_ack(nfc_buf, 2)){
        return 0;
    }
    read_dt(nfc_buf, 8);
    if(nfc_buf[NFC_FRAME_ID_INDEX] != (PN532_COMMAND_SETPARAMETERS+1)){
        return 0;
    }
    return 1;
}

/*****************************************************************************/
/*!
	@brief  send frame to PN532 and wait for ack
	@param  cmd - pointer to frame buffer
	@param  len - frame length
	@return 0 - send failed
            1 - send successfully
*/
/*****************************************************************************/
u8 NFC_Module::write_cmd_check_ack(u8 *cmd, u8 len)
{
    write_cmd(cmd, len);
    wait_ready();
#ifdef PN532DEBUG
	Serial.println("IRQ received");
#endif

	// read acknowledgement
	if (!read_ack()) {
#ifdef PN532DEBUG
		Serial.println("No ACK frame received!");
#endif
		return false;
	}

	return true; // ack'd command
}

/*****************************************************************************/
/*!
	@brief  send a byte data via I2C interface
	@param  data - The byte to send.
	@return 0 - send failed
            1 - successful
*/
/*****************************************************************************/
inline u8 NFC_Module::send(u8 data)
{
#if ARDUINO >= 100
    return Wire.write((u8)data);
#else
    return Wire.send((u8)data);
#endif
}

/*****************************************************************************/
/*!
	@brief  receive a byte from I2C interface
	@param  NONE
	@return received data
*/
/*****************************************************************************/
inline u8 NFC_Module::receive(void)
{
#if ARDUINO >= 100
    return Wire.read();
#else
    return Wire.receive();
#endif
}

/*****************************************************************************/
/*!
	@brief  Send a byte by hex format
	@param  data - the byte
	@return NONE
*/
/*****************************************************************************/
void NFC_Module::puthex(u8 data)
{
    Serial.write(hextab[(data>>4)&0x0F]);
    Serial.write(hextab[data&0x0F]);
    Serial.write(' ');
}

/*****************************************************************************/
/*!
	@brief  Send hexadecimal data through Serial with specified length.
	@param  buf - pointer of data buffer.
	@param  len - length need to send.
	@return NONE
*/
/*****************************************************************************/
void NFC_Module::puthex(u8 *buf, u32 len)
{
    u32 i;
    for(i=0; i<len; i++)
    {
        Serial.write(hextab[(buf[i]>>4)&0x0F]);
        Serial.write(hextab[buf[i]&0x0F]);
        Serial.write(' ');
    }
}

/*****************************************************************************/
/*!
	@brief  Write data frame to PN532.
	@param  cmd - Pointer of the data frame.
	@param  len - length need to write
	@return NONE
*/
/*****************************************************************************/
void NFC_Module::write_cmd(u8 *cmd, u8 len)
{
    uint8_t checksum;

    len++;

#ifdef PN532DEBUG
    Serial.print("Sending: ");
#endif

    delay(2);     // or whatever the delay is for waking up the board

    // I2C START
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
    send(PN532_PREAMBLE);
    send(PN532_PREAMBLE);
    send(PN532_STARTCODE2);

    send(len);
    send(~len + 1);

    send(PN532_HOSTTOPN532);
    checksum += PN532_HOSTTOPN532;

#ifdef PN532DEBUG
    puthex(PN532_PREAMBLE);
    puthex(PN532_PREAMBLE);
    puthex(PN532_STARTCODE2);
    puthex((unsigned char)len);
    puthex((unsigned char)(~len + 1));
    puthex(PN532_HOSTTOPN532);
#endif

    for (uint8_t i=0; i<len-1; i++)
    {
        if(send(cmd[i])){
            checksum += cmd[i];
#ifdef PN532DEBUG
            puthex(cmd[i]);
#endif
        }else{
            i--;
            delay(1);
        }
    }

    send(~checksum);
    send(PN532_POSTAMBLE);

    // I2C STOP
    Wire.endTransmission();

#ifdef PN532DEBUG
    puthex(~checksum);
    puthex(PN532_POSTAMBLE);
    Serial.write('\n');
#endif
}

/*****************************************************************************/
/*!
	@brief  Read data frame from PN532.
	@param  buf - pointer of data buffer
	@param  len - length need to read
	@return NONE.
*/
/*****************************************************************************/
void NFC_Module::read_dt(u8 *buf, u8 len)
{
    delay(2);

#ifdef PN532DEBUG
    Serial.print("Reading: ");
#endif
    // Start read (n+1 to take into account leading 0x01 with I2C)
    Wire.requestFrom((u8)PN532_I2C_ADDRESS, (u8)(len+2));
    // Discard the leading 0x01
    receive();
    for (u8 i=0; i<len; i++)
    {
        delay(1);
        buf[i] = receive();
#if 0
        if((len!=6)&&i==3&&(buf[0]==0)&&(buf[1]==0)&&(buf[2]==0xFF)){
            len = buf[i]+6;
        }
#endif

#ifdef PN532DEBUG
        puthex(buf[i]);
#endif
    }
    // Discard trailing 0x00 0x00
    // receive();

#ifdef PN532DEBUG
    Serial.println();
#endif
}

/*****************************************************************************/
/*!
	@brief  read ack frame from PN532
	@param  NONE
	@return 0 - ack failed
            1 - Ack OK
*/
/*****************************************************************************/
u8 NFC_Module::read_ack(void)
{
    u8 ack_buf[6];

	read_dt(ack_buf, 6);

//    puthex(ack_buf, 6);
//    Serial.println();
	return (0 == strncmp((char *)ack_buf, (char *)ack, 6));
}

/*****************************************************************************/
/*!
	@brief  DISCARD
	@param
	@return
*/
/*****************************************************************************/
u8 NFC_Module::read_sta(void)
{
    /**
    uint8_t x = digitalRead(_irq);

    if (x == 1)
    return PN532_I2C_BUSY;
      else
    return PN532_I2C_READY;
*/
    delay(NFC_WAIT_TIME);
    return PN532_I2C_READY;
}

/*****************************************************************************/
/*!
	@brief  Because of IRQ pin is unused, use this function to wait for PN532
        being ready.
	@param  NONE.
	@return Always return ready.
*/
/*****************************************************************************/
u8 NFC_Module::wait_ready(u8 ms)
{
    delay(ms);
    return PN532_I2C_READY;
}
