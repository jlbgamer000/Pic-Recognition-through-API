#include <WiFi.h>

// Updated credentials
const char* ssid     = "vivo X200";
const char* password = "vivox2001234";

WiFiClient client;
uint8_t buf[512];

// Track Wi-Fi state for disconnection detection
int lastWiFiStatus = WL_IDLE_STATUS;

void setup() {
  Serial.begin(115200);                    // USB serial for monitoring
  
  Serial1.setRxBufferSize(4096);
  Serial1.begin(115200, SERIAL_8N1, 7, 6);

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected, IP address: " + WiFi.localIP().toString());
  lastWiFiStatus = WL_CONNECTED;           // Initialise state
}

void loop() {
  // --- Wi-Fi connection monitor ---
  int currentStatus = WiFi.status();
  if (currentStatus != lastWiFiStatus) {
    if (currentStatus == WL_CONNECTED) {
      Serial.println("WiFi reconnected, IP: " + WiFi.localIP().toString());
    } else {
      Serial.print("WiFi disconnected! Status code: ");
      Serial.println(currentStatus);
    }
    lastWiFiStatus = currentStatus;
  }

  // --- Original serial proxy logic ---
  if (Serial1.available()) {
    String cmd = Serial1.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("CONNECT ")) {
      String target = cmd.substring(8);
      int colonIdx = target.indexOf(':');
      if (colonIdx > 0) {
        String host = target.substring(0, colonIdx);
        int port = target.substring(colonIdx + 1).toInt();

        client.stop();
        if (client.connect(host.c_str(), port)) {
          // Keep the critical fix: no setNoDelay(true)
          Serial1.print("PROXY_OK\n");

          while (client.connected() || client.available() || Serial1.available()) {
            // Forward from Google -> UNO Q
            int len = client.available();
            if (len > 0) {
              if (len > 64) len = 64;
              int r = client.read(buf, len);
              if (r > 0) {
                Serial1.write(buf, r);
                Serial1.flush();
                delay(2);
              }
            }

            // Forward from UNO Q -> Google
            int s_len = Serial1.available();
            if (s_len > 0) {
              if (s_len > sizeof(buf) - 1) s_len = sizeof(buf) - 1;
              int r = Serial1.readBytes(buf, s_len);
              if (r > 0) {
                char temp[512];
                memcpy(temp, buf, r);
                temp[r] = '\0';
                if (strstr(temp, "+++CLOSE_TCP") != NULL) {
                  break;
                }
                client.write(buf, r);
              }
            }
            yield();
          }
          client.stop();
        } else {
          Serial1.print("PROXY_FAIL\n");
        }
      }
    }
  }
}