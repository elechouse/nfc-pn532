/**
  @file    nfc_mifare_mf1s50_reader.ino
  @author  www.elechouse.com
  @brief   example of reading mf1s50 card for NFC_MODULE
  
    For this demo, waiting for a MF1S50 card or tag, after reading a card/tag UID,
    then try to read the block 4/5/6/7 ..
  
  @section  HISTORY
  
  V1.0 initial version
  
    Copyright (c) 2012 www.elechouse.com  All right reserved.
*/

/** include library */
#include "Wire.h"
#include "nfc.h"

/** define a nfc class */
NFC_Module nfc;

void setup(void)
{
  Serial.begin(9600);
  nfc.begin();
  Serial.println("MF1S50 Reader Demo From Elechouse!");
  
  uint32_t versiondata = nfc.get_version();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  /** Set normal mode, and disable SAM */
  nfc.SAMConfiguration();
}

void loop(void)
{
  u8 buf[32],sta;
  
  
  /** Polling the mifar card, buf[0] is the length of the UID */
  sta = nfc.InListPassiveTarget(buf);
  
  /** check state and UID length */
  if(sta && buf[0] == 4){
    /** the card may be Mifare Classic card, try to read the block */  
    Serial.print("UUID length:");
    Serial.print(buf[0], DEC);
    Serial.println();
    Serial.print("UUID:");
    nfc.puthex(buf+1, buf[0]);
    Serial.println();
    /** factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF */
    u8 key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 blocknum = 4;
    /** Authentication blok 4 */
    sta = nfc.MifareAuthentication(0, blocknum, buf+1, buf[0], key);
    if(sta){
      /** save read block data */
      u8 block[16];
      Serial.println("Authentication success.");
      
      // uncomment following lines for writing data to blok 4
/*      
      strcpy((char*)block, "Elechoues - NFC");
      sta = nfc.MifareWriteBlock(blocknum, block);
      if(sta){
        Serial.println("Write block successfully:");
      }
*/  

      /** read block 4 */
      sta = nfc.MifareReadBlock(blocknum, block);
      if(sta){
        Serial.println("Read block successfully:");
        
        nfc.puthex(block, 16);
        Serial.println();
      }
      
      /** read block 5 */
      sta = nfc.MifareReadBlock(blocknum+1, block);
      if(sta){
        Serial.println("Read block successfully:");
        
        nfc.puthex(block, 16);
        Serial.println();
      }
      
      /** read block 6 */
      sta = nfc.MifareReadBlock(blocknum+2, block);
      if(sta){
        Serial.println("Read block successfully:");
        
        nfc.puthex(block, 16);
        Serial.println();
      }
      
      /** read block 7 */
      sta = nfc.MifareReadBlock(blocknum+3, block);
      if(sta){
        Serial.println("Read block successfully:");
        
        nfc.puthex(block, 16);
        Serial.println();
      }
    }  
  }
}
