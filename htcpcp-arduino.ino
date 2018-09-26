/*
Copyright (C) 2018 Jorge Matricali <jorgematricali@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "pot.h"
#include "time.h"

#define LISTEN_PORT 80
#define LISTEN_PORT_UDP 8888

/* Prototipos */
void send_short_response(EthernetClient client, int status, String message);

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
const char timeServer[] = "time.nist.gov";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

EthernetServer server(LISTEN_PORT);
EthernetUDP Udp;

pot_t *POT = NULL;

void setup()
{
  pinMode(PIN_LED_LINK, OUTPUT);
  pinMode(PIN_LED_STATUS_READY, OUTPUT);
  digitalWrite(PIN_LED_LINK, LOW);
  digitalWrite(PIN_LED_STATUS_READY, LOW);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.println("HTCPCP/1.0 Server");
  POT = (pot_t *) malloc(sizeof *POT);
  if (POT == NULL) {
    Serial.println("Cannot allocate POT");
    exit(1);
  }
  pot_init(POT);
  
  Serial.println("Obtaining an IP address...");
  Ethernet.begin(mac);
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    for (;;) {
      delay(1);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  Serial.println("Clock sync...");
  Udp.begin(LISTEN_PORT_UDP);
  sendNTPpacket(timeServer); // send an NTP packet to a time server

  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }

  server.begin();
  Serial.print("Server started at: ");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(LISTEN_PORT);

  digitalWrite(PIN_LED_LINK, HIGH);
  pot_refresh(POT);
}

void send_short_response(EthernetClient client, int status, String message)
{
  time_t timer = now();
  struct tm *tv;
  tv = gmtime(&timer);
  char datetime[32] = "";
  sprintf(datetime, "Date: %02d-%02d-%04d %02d:%02d:%02d GMT", tv->tm_mday, tv->tm_mon, tv->tm_year + 1900, tv->tm_hour, tv->tm_min, tv->tm_sec);
  client.print("HTCPCP/1.0 ");
  client.print(status);
  client.print(" ");
  client.println(message);
  client.println(datetime);
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.print("<html><h1>");
  client.print(status);
  client.print(" ");
  client.print(message);
  client.println("</h1></html>");
}

void sendNTPpacket(const char * address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
void loop()
{
  EthernetClient client = server.available();
  if (client) {
    Serial.print("New incoming connection from ");
    Serial.println(client.remoteIP());

    String method = "";
    String path = "";
    String protocol = "";
    int state = 0;
    boolean currentLineIsBlank = true;

    while (client.connected() && client.available()) {
      char c = client.read();
      Serial.write(c);
      if (c == '\r' || c == '\n') {
        goto bad_request;
      }
      if (c == ' ') {
        /* Continuamos con el path */
        break;
      }
      method.concat(c);
    }

    while (client.connected() && client.available()) {
      char c = client.read();
      Serial.write(c);
      if (c == '\r' || c == '\n') {
        goto bad_request;
      }
      if (c == ' ') {
        /* Continuamos con el protocolo */
        break;
      }
      path.concat(c);
    }

    while (client.connected() && client.available()) {
      char c = client.read();
      Serial.write(c);
      if (c == '\r') {
        continue;
      }
      if (c == '\n') {
        /* Continuamos con los headers */
        break;
      }
      protocol.concat(c);
    }
    
    /* An http request ends with a blank line */
    while (client.connected() && client.available()) {
      char c = client.read();
      Serial.write(c);
      if (c == '\n' && currentLineIsBlank) {
        break;
      }
      if (c == '\n') {
        currentLineIsBlank = true;
      } else if (c != '\r') {
        currentLineIsBlank = false;
      }
    }

    /* Handle method */
    if (!method.equals("BREW") && !method.equals("POST")
        && !method.equals("GET") && !method.equals("PROPFIND")) {
      send_short_response(client, 501, "Not Implemented");
      goto cleanup;
    }

    if (!path.equals("/pot-1")) {
      /* Only 1 pot :D */
      send_short_response(client, 404, "Pot Not Found");
      goto cleanup;
    }

    if (method.equals("BREW") && !method.equals("POST")) {
      pot_refresh(POT);

      if (POT->status == POT_STATUS_BREWING) {
        Serial.println("POT BUSY!");
        send_short_response(client, 510, "Pot Busy");
        goto cleanup;
      }
      
      Serial.println("Brewing...");
      pot_brew(POT);
      send_short_response(client, 200, "OK");
      goto cleanup;
    }

    send_short_response(client, 501, "Not Implemented");
    goto cleanup;

bad_request:
    send_short_response(client, 400, "Bad Request");

cleanup:
    delay(1);
    client.stop();
    Serial.print("Client disconnected.");
  }
  pot_refresh(POT);
}
