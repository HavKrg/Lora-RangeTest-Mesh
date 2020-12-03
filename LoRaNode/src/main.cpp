// Sketch for a LoRa mesh-node.
// The nodes ID is read from EEPROM (See https://github.com/nootropicdesign/lora-mesh )
// Sketch simply receives a message and sends a reply
// Relies on Radiohead library by Mike McCauley (http://www.airspayce.com/mikem/arduino/RadioHead/index.html)
// Tested with TTGO-T3 as "client-node" and Moteino as "server-nodes"

#include <RH_RF95.h>
#include <RHMesh.h>
#include <EEPROM.h>



// Singleton instance of the radio driver
RH_RF95 rf95(10,2);
// LoRa Mesh Manager
RHMesh manager(rf95, EEPROM.read(0));


void setup()
{
// Open serial port
Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
// Initialize LoRa Radio
// if (!rf95.init())
//     Serial.println("RF95 init failed");

rf95.init();
// Initialize LoRa Mesh Manger
manager.init();
// if(!manager.init())
//     Serial.println("LoRa manager initialization failed");
manager.setTimeout(10000) ;

// Configure rf95 radio
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf2048);
  rf95.setFrequency(868100000);
  rf95.setTxPower(20, false);

// Print the Node ID
  Serial.print("Node ID : ");
  Serial.println(manager.thisAddress());

}

// Reply message replace 'X' with the node number
uint8_t data[] = "2";
// Buffer for incoming message
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];


void loop()
{
  uint8_t len = sizeof(buf);
  uint8_t from;
  if (manager.recvfromAck(buf, &len, &from))
  {
    // Print incomming message when it arrives
    Serial.print("0x");
    Serial.print(from, HEX);
    Serial.print(": ");
     Serial.println((char*)buf);

    // Send a reply back to the originator client
    if (manager.sendtoWait(data, sizeof(data), from) != RH_ROUTER_ERROR_NONE)
      Serial.println("sendtoWait failed");
  }
}