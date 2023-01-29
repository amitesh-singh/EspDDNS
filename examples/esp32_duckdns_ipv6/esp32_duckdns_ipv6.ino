// esp32 example
#include "EspDDNS.h"
#include <lwip/dns.h>

static constexpr const auto ssid { "************" };
static constexpr const auto passwd { "*************" };

bool waitFlag = true;
void WiFiEvent(WiFiEvent_t event)
{
    switch(event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            //set sta hostname here
            WiFi.setHostname("espddns-ipv6test");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            //enable sta ipv6 here
            WiFi.enableIpV6();
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            Serial.print("STA IPv6: ");
            Serial.println(WiFi.localIPv6());
            waitFlag = false;
            break;
        default:
            break;
    }
}

void set_dnsserver_google()
{
    //set dns server
    ip_addr_t ip6_dns;
    // GOOGLE EXTERNAL - 2001:4860:4860::8888
    IP_ADDR6(&ip6_dns,PP_HTONL(0x20014860UL), PP_HTONL(0x48600000UL), PP_HTONL(0x00000000UL), PP_HTONL(0x00008888UL));
    dns_setserver(0, &ip6_dns);
}

void setup()
{
    Serial.begin(115200);

    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_MODE_STA);    
    WiFi.begin(ssid, passwd);

    set_dnsserver_google();
    delay(100);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(100);
      Serial.print(".");
    }

    ddns::client ddns_client(ddns::type::DUCKDNS, "XXXX.duckdns.org", 
                            "abcde-fdgh-ijh", ddns::ip_type::IPv6);

    Serial.println("Connected..");

    set_dnsserver_google();
    delay(100);

    ddns_client.onUpdate([](String old_ip, String new_ip) {
        Serial.printf("Ip address is changed to %s from %s", new_ip.c_str(), old_ip.c_str());
    });
   
    //just check once on boot only.
    while (waitFlag)
    {
        Serial.println("waiting...");
        delay(1000);
    }
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.localIPv6());

    // For esp32, this is required. The global ipv6 gets assigned after few seconds since link ipv6 is assigned
    delay(5000);

    Serial.printf("global ipv6 address: %s", ddns_client.get_ipv6().c_str());
    ddns_client.update();
}

void loop()
{
}
