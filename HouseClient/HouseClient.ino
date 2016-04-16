/*
  Copyright (c) 2014 MediaTek Inc.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
   See the GNU Lesser General Public License for more details.
*/
#include <LBT.h>
#include <LBTClient.h>
#include <LGSM.h>
#include <LCheckSIM.h>
#include <LDisplay.h>

static LBTDeviceInfo info = {0};
boolean find = 0;
#define SPP_SVR "pi3" // it should be the prefer server's name,  customize it yourself.
#define ard_log Serial.printf
int read_size = 0;
char buf_contex[200];
char *spaces = "                    ";

void setup()  
{
  Serial.begin(9600);
  ard_log("LBT start\n");
  
  Lcd.init();
  Lcd.font_init();
  Lcd.back_light_level(50);
  Lcd.screen_set(0xffff00);
  
  while(!LSMS.ready())
  {
    delay(1000);
  }
  Serial.println("SMS ready is ok");
  
  // begin BT
  bool success = LBTClient.begin();
  if( !success )
  {
      ard_log("Cannot begin Bluetooth Client successfully\n");
      delay(0xffffffff);
  }
  else
  {
      ard_log("Bluetooth Client begin successfully\n");
      // scan the devices around
      int num = LBTClient.scan(30);
      ard_log("scanned device number [%d]", num);
      for (int i = 0; i < num; i++)
      {
        memset(&info, 0, sizeof(info));
        // to check the prefer master(server)'s name
        if (!LBTClient.getDeviceInfo(i, &info))
        {
            continue;
        }
        ard_log("getDeviceInfo [%02x:%02x:%02x:%02x:%02x:%02x][%s]\n", 
            info.address.nap[1], info.address.nap[0], info.address.uap, info.address.lap[2], info.address.lap[1], info.address.lap[0],
            info.name);
        if (0 == strcmp(info.name, SPP_SVR))
        {
            find = 1;
            ard_log("found\n");
            break;
        }
      }
      
  }
}
 
void loop()
{

    // to check if the connection is ready
    if(find && !LBTClient.connected())
    {
        ard_log("Diconnected, try to connect\n");
        // do connect
        bool conn_result = LBTClient.connect(info.address);
        ard_log("connect result [%d]", conn_result);
 
        if( !conn_result )
        {
            ard_log("Cannot connect to SPP Server successfully\n");
            Lcd.draw_font(10, 0, "Connect failed", 0xffff00, 0);
            delay(0xffffffff);
        }
        else
        {
            ard_log("Connect to SPP Server successfully\n");
            Lcd.draw_font(10, 0, "Connected", 0xffff00, 0);
        }
        
        while (true) 
        {
          if(LSMS.available()) {
            char buffer[32] = {0};
            
            LSMS.remoteContent(buf_contex, 50);
            Serial.print("Content:");  // display Content part
            Serial.println(buf_contex);
            Serial.print("Content size:");
            Serial.println(strlen(buf_contex));
            Lcd.draw_font(10, 20, spaces, 0xffff00, 0);
            Lcd.draw_font(10, 20, buf_contex, 0xffff00, 0);
            
            int write_size = LBTClient.write(buf_contex, strlen(buf_contex));
            LBTClient.write((char *) "\n", 1);
            ard_log("client first spec write_size [%d]\n", write_size);

            // to read the data from master(server)
            while(1)
            {
              if(LBTClient.available())
              {
                read_size = LBTClient.readBytes(buffer,32);
                break;
              }
              delay(100);
            }
            ard_log("read size [%d][%s]\n", read_size, buffer);
            Lcd.draw_font(10, 40, spaces, 0xffff00, 0);
            Lcd.draw_font(10, 40, buffer, 0xffff00, 0);
          }
        }
        
        find = 0;
    }
    ard_log("loop client\n");
    delay(2000);

}
