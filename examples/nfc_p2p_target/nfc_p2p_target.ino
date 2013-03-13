/**
  @file    nfc_p2p_target.ino
  @author  www.elechouse.com
  @brief   example of Peer to Peer communication for NFC_MODULE.

    By this demo, target send "Hi, this message comes from NFC TARGET."
    
  NOTE: this library only support MAX 50 bytes data packet, that is the tx_len must
  less than 50..
  
  @section  HISTORY
  
  V1.0 initial version
  
    Copyright (c) 2012 www.elechouse.com  All right reserved.
*/

/** include library */
#include "nfc.h"

/** define a nfc class */
NFC_Module nfc;

/** define RX and TX buffers, and length variable */
u8 tx_buf[50]="Hi, This message comes from NFC TARGET.";
u8 tx_len;
u8 rx_buf[50];
u8 rx_len;

void setup(void)
{
  Serial.begin(115200);
  nfc.begin();
  Serial.println("P2P Target Demo From Elechouse!");
  
  uint32_t versiondata = nfc.get_version();
  if (! versiondata) {
    Serial.println("Didn't find PN53x board");
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
  /** device is configured as Target */
  if(nfc.P2PTargetInit()){
    /**
      send data with a length parameter and receive some data,
      tx_buf --- data send buffer
      tx_len --- data send legth
      rx_buf --- data recieve buffer, return by P2PTargetTxRx
      rx_len --- data receive length, return by P2PTargetTxRx
    */
    Serial.println("Initiator is sensed.");
    tx_len = strlen((const char *)tx_buf);
    if(nfc.P2PTargetTxRx(tx_buf, tx_len, rx_buf, &rx_len)){
      Serial.print("Data Received: ");
      Serial.write(rx_buf, rx_len);
      Serial.println();
    }
    Serial.println();
  }
}
