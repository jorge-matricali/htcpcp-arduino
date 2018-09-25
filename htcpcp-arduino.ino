#include <SPI.h>
#include <Ethernet.h>

#define LISTEN_PORT 80

/* Prototipos */
void send_short_response(EthernetClient client, int status, String message);

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
EthernetServer server(LISTEN_PORT);

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.println("HTCPCP/1.0 Server");
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

  server.begin();
  Serial.print("Server started at: ");
  Serial.print(Ethernet.localIP());
  Serial.print(":");
  Serial.println(LISTEN_PORT);
}

void send_short_response(EthernetClient client, int status, String message)
{
  client.print("HTCPCP/1.0 ");
  client.print(status);
  client.print(" ");
  client.println(message);
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println();
  client.print("<html><h1>");
  client.print(status);
  client.print(" ");
  client.print(message);
  client.println("</h1></html>");
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

    if (!path.equals("/pot-1")) {
      /* Only 1 pot :D */
      send_short_response(client, 404, "Pot Not Found");
      goto cleanup;
    }

bad_request:
    send_short_response(client, 400, "Bad Request");

cleanup:
    delay(1);
    client.stop();
    Serial.print("Client disconnected.");
  }
}
