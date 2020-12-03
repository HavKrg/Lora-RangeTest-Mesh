// Sketch for a LoRa mesh-client. 
// The Mesh nodes ID is read from EEPROM (See https://github.com/nootropicdesign/lora-mesh )
// Sketch sends out messages to other mesh nodes and displays a filled circle on the screen if it reaches them, and an empty circle if it does not
// Based on Radiohead rf22_mesh_client.pde example and https://github.com/nootropicdesign/lora-mesh
// Tested with TTGO client and Moteino nodes

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <RH_RF95.h>
#include <RHMesh.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 16

#define NUMBEROFLORANODES 10

TwoWire twi = TwoWire(1);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);



/*************************************
            LORA ADDRESSES
*************************************/

#define BAUD 115200;

/*************************************
            LORA PINS
*************************************/
#define SCK 5      // GPIO5  -- SX1276's SCK
#define MISO 19    // GPIO19 -- SX1276's MISO
#define MOSI 27    // GPIO27 -- SX1276's MOSI
#define SS 18      // GPIO18 -- SX1276's CS
#define RST 23     // GPIO14 -- SX1276's RESET
#define DI0 26     // GPIO26 -- SX1276's IRQ(Interrupt Request)
#define BAND 868E6 //

/*************************************
            LORA SETTINGS
*************************************/
RH_RF95 rf95(SS, DI0);
RHMesh manager(rf95, 1);

bool connectionStatus[NUMBEROFLORANODES];


bool sendMessage(int address);
void displayMessage(bool connection[]);

void setup()
{
  // put your setup code here, to run once:
  // initialize serial communication
  Serial.begin(9600);
  while (!Serial)
  {
    Serial.println("Serial connected");
  }

  // initialize LED-screen
  pinMode(LED_BUILTIN, OUTPUT);
  twi.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    while (1)
      ;
  }
  
  // set all connection statuses to 0 (false)
  memset(connectionStatus, 0, NUMBEROFLORANODES);

  // initialize RF95 radio
  if (!rf95.init())
    Serial.println("RF95 init failed");
  // initialize mesh manager
  if (!manager.init())
    Serial.println("Mesh manager Initialization failed");

  // configure rf95 radio (see http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html#ab9605810c11c025758ea91b2813666e3a1759bc33893056f011b81e62e5c8af5f)
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf2048);
  rf95.setFrequency(868100000);

  rf95.setTxPower(20, false);

  // set timeout for manager (needed for low bandwidth modem config)
  manager.setTimeout(3000);
  }

// Message sendt to client
uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];

void loop()
{
 for (size_t i = 2; i < 4; i++)
 {
   connectionStatus[i] = sendMessage(i);
 }
 
 displayMessage(connectionStatus);
 delay(3000);
}

bool sendMessage(int address){
  Serial.printf("Sending to %d\n", address);
    
  // Send a message to a rf95 client
  // It will be routed by the intermediate
  // nodes to the destination node, accorinding to the
  // routing tables in each node
  if (manager.sendtoWait(data, sizeof(data), address) == RH_ROUTER_ERROR_NONE)
  {
    // It has been reliably delivered to the next node.
    // Now wait for a reply from the ultimate server
    uint8_t len = sizeof(buf);
    uint8_t from;    
    if (manager.recvfromAckTimeout(buf, &len, 3000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
      return 1;
    }
    else
    {
      Serial.printf("No reply, is node %d running?\n", address);
      return 0;
    }
  }
  else
    Serial.println("sendtoWait failed. Are the intermediate router servers running?");
  return 0;
}

void displayMessage(bool connection[])
{
  display.clearDisplay();

  if (connection[2] == 1)
    display.fillCircle(70, 30, 15, SSD1306_WHITE);
  else
    display.drawCircle(70, 30, 15, SSD1306_WHITE);

  if (connection[3] == 1)
    display.fillCircle(70, 60, 15, SSD1306_WHITE);
  else
    display.drawCircle(70, 60, 15, SSD1306_WHITE);

  display.display();
}

  