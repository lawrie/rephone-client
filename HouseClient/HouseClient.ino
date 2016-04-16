#include <LBT.h>
#include <LBTClient.h>
#include <LGSM.h>
#include <LCheckSIM.h>
#include <LDisplay.h>

// Change to the name of your server
#define SPP_SVR "pi3"

LBTDeviceInfo info;
char *spaces = "                    ";

void setup()  
{
  Serial.begin(9600);
  Serial.println("House Client started");
  
  // Set up the LCD screen
  Lcd.init();
  Lcd.font_init();
  Lcd.back_light_level(50);
  Lcd.screen_set(0xffff00); // Yellow background
  
  // Set up SMS
  while(!LSMS.ready())
  {
    delay(1000);
  }
  Serial.println("SMS is ready");
  
  // Set up Bluetooth
  bool found = false;
  bool success = LBTClient.begin();
  if( !success )
  {
    Serial.println("Cannot start Bluetooth");
	Lcd.draw_font(10, 0, "Cannot start Bluetooth", 0xffff00, 0);
    delay(0xffffffff);
  }
  else
  {
    Serial.println("Bluetooth client started");
    // Look for the Bluetooth devices
    int num = LBTClient.scan(30);
    Serial.printf("Found [%d] devices\n", num);
    for (int i = 0; i < num; i++)
    {
      memset(&info, 0, sizeof(info));
      // See if it is the required server
      if (!LBTClient.getDeviceInfo(i, &info)) continue;
      Serial.printf("getDeviceInfo [%02x:%02x:%02x:%02x:%02x:%02x][%s]\n", 
          info.address.nap[1], info.address.nap[0], info.address.uap, info.address.lap[2], info.address.lap[1], info.address.lap[0],
          info.name);
      if (0 == strcmp(info.name, SPP_SVR))
      {
        found = true;
        Serial.println("Server found");
        break;
	  }
    }
  }
  
  if( !found )
  {
    Serial.println("Server not found");
	Lcd.draw_font(10, 0, "Server not found", 0xffff00, 0);
    delay(0xffffffff);
  }
  
  Serial.println("Trying to connect\n");
  // Try to connect
  bool conn_result = LBTClient.connect(info.address);
  Serial.printf("Connect result [%d]\n", conn_result);
 
  if( !conn_result )
  {
    Serial.println("Connect failed");
    Lcd.draw_font(10, 0, "Connect failed", 0xffff00, 0);
    delay(0xffffffff);
  }
  else
  {
    Serial.println("Connected to SPP Server");
    Lcd.draw_font(10, 0, "Connected", 0xffff00, 0);
  }
}
 
void loop()
{
  // Wait for SMS message
  if( LSMS.available() ) 
  {
    char cmd[50];
    char reply[32];
          
    // Get the command from the text message 
    LSMS.remoteContent(cmd, 50);
    Serial.print("Command:");
    Serial.println(cmd);
    Lcd.draw_font(10, 20, spaces, 0xffff00, 0);
    Lcd.draw_font(10, 20, cmd, 0xffff00, 0);
	Lcd.draw_font(10, 40, spaces, 0xffff00, 0);
	
	// Send the command to the SPP server, terminated by newline
    LBTClient.write(cmd, strlen(cmd));
    LBTClient.write((char *) "\n", 1);

    // Read reply from SPP server
    while(true)
    {
      if(LBTClient.available())
      {
        LBTClient.readBytes(reply,32);
        break;
      }
      delay(100);
    }
    Serial.printf("Reply: [%s]\n", reply);
    Lcd.draw_font(10, 40, reply, 0xffff00, 0);
  }
}
