#include <WiFi.h>
#include <M2M_LM75A.h>

M2M_LM75A Temp_Sensor;

// Wi-Fi Credentials
const char *ssid = "ESP32_Temp_Router";
const char *password = "12345678";

WiFiServer server(80);

double temps[10];  // Stores last 10 temperature readings
int Bufindex = 0;  // Circular buffer index

double lastValidTemp = 0;

void setup() {
    Serial.begin(115200);
    Temp_Sensor.begin();

    // Initialize buffer with initial temperature
    double initialTemp = Temp_Sensor.getTemperature();
    if (initialTemp != -1000){
      lastValidTemp = initialTemp;
    }
    for (int i = 0; i < 10; i++) temps[i] = initialTemp;

    // Set ESP32 as an Access Point
    WiFi.softAP(ssid, password);
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    if (!client) return;

    String request = client.readStringUntil('\r');
    client.flush();

    // Read temperature and validate it
    double temp = Temp_Sensor.getTemperature();
    if (temp != -1000) {
        lastValidTemp = temp;
    } else {
        temp = lastValidTemp;  // Use last valid reading
    }

    // Update buffer with valid temp
    temps[Bufindex] = temp;
    Bufindex = (Bufindex + 1) % 10;

    // Calculate average temperature
    double avgTemp = 0;
    for (double t : temps) avgTemp += t;
    avgTemp /= 10;

    // Check for deviation alert
    bool alert = checkDeviation(temps, 10);

    // Debug Output
    Serial.printf("Temp: %.2f, Avg Temp: %.2f, Alert: %d\n", temp, avgTemp, alert);

    // Handle AJAX requests
    if (request.indexOf("/get_temp") != -1) {
        sendResponse(client, String(temp, 2) + " °C");
        return;
    }
    if (request.indexOf("/get_alert") != -1) {
        sendResponse(client, alert ? "1" : "0");
        return;
    }

    // Serve Webpage
    sendWebPage(client);
}

// Function to check if temperature deviation exceeds 5°C
bool checkDeviation(double *arr, int size) {
    double minT = arr[0], maxT = arr[0];
    for (int i = 1; i < size; i++) {
        if (arr[i] > maxT) maxT = arr[i];
        if (arr[i] < minT) minT = arr[i];
    }
    return (maxT - minT) > 5;
}

// Function to send HTTP response
void sendResponse(WiFiClient &client, const String &message) {
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n" + message);
    client.stop();
}

// Function to send webpage
void sendWebPage(WiFiClient &client) {
    client.print(
        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
        "<!DOCTYPE html><html lang='en'><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>ESP32 Temperature Monitor</title>"
        "<style>"
        "body{text-align:center;font-family:sans-serif;background:#222;color:white;}"
        ".temp{font-size:80px;font-weight:bold;}"
        ".alert{width:150px;height:15px;margin:auto;background:gray;transition:background 0.5s;}"
        "</style>"
        "<script>"
        "function updateTemp(){"
        "fetch('/get_temp').then(res=>res.text()).then(temp=>{"
        "document.getElementById('temp').innerHTML=temp;"
        "fetch('/get_alert').then(res=>res.text()).then(alert=>{"
        "document.getElementById('alert').style.background=alert=='1'?'red':'gray';"
        "if(alert=='1'){ playBeep(); }"
        "});"
        "setTimeout(updateTemp, 2000);"
        "});"
        "}"
        // Web Audio API beep function
        "function playBeep() {"
        "var ctx = new (window.AudioContext || window.webkitAudioContext)();"
        "var oscillator = ctx.createOscillator();"
        "var gainNode = ctx.createGain();"
        "oscillator.type = 'sine';"
        "oscillator.frequency.setValueAtTime(1000, ctx.currentTime);"
        "gainNode.gain.setValueAtTime(1, ctx.currentTime);"
        "oscillator.connect(gainNode);"
        "gainNode.connect(ctx.destination);"
        "oscillator.start();"
        "setTimeout(()=>{oscillator.stop();}, 500);"
        "}"
        "window.onload=updateTemp;"
        "</script>"
        "</head><body>"
        "<h1>ESP32 Temperature Monitor</h1>"
        "<div class='temp' id='temp'>Loading...</div>"
        "<h3>Deviation Alert</h3><div id='alert' class='alert'></div>"
        "</body></html>"
    );
    client.stop();
}
