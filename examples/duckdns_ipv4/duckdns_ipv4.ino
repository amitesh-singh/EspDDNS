//This example works for both ESP32 and ESP8266
#include <EspDDNS.h>

static constexpr const auto ssid { "******" };
static constexpr const auto passwd { "*****" };

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, passwd);
    
    ddns::client ddns_client(ddns::type::DUCKDNS, "yourdomain.duckdns.org", "yourduckdnstoken", ddns::ip_type::IPv4);
    ddns_client.onUpdate([](String old_ip, String new_ip) {
        Serial.print("Ip address is changed to: ");
        Serial.println(new_ip);
    });
  
    ddns_client.wait();

    Serial.print("Connected.., local ip: ");
    Serial.println(WiFi.localIP());

    //just check once on boot only.
    ddns_client.update();
}

void loop()
{
}
