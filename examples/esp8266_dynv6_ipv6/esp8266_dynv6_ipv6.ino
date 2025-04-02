#include <EspDDNS.h>

static constexpr const auto ssid { "******" };
static constexpr const auto passwd { "*****" };

void set_dnsserver_google()
{
    //set dns server
    ip_addr_t ip6_dns;
    // GOOGLE EXTERNAL - 2001:4860:4860::8888
    IP_ADDR6(&ip6_dns, PP_HTONL(0x20014860UL), PP_HTONL(0x48600000UL),
             PP_HTONL(0x00000000UL), PP_HTONL(0x00008888UL));
    dns_setserver(0, &ip6_dns);
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, passwd);
    
    // In case of ipv6, do enable lwp2 option in arduino ide.
    // IPv6 is by default
    ddns::client ddns_client(ddns::type::DYNV6, "yourdomain.dynv6.org",
                             "yourdynv6token", ddns::ip_type::IPv6);
    ddns_client.onUpdate([](String old_ip, String new_ip) {
        Serial.print("Ip address is changed to: ");
        Serial.print(new_ip);
    });

    // wait for the esp8266 to get the ipv6 
    ddns_client.wait();

    Serial.print("Connected.., local ip: ");
    Serial.println(WiFi.localIP());

    set_dnsserver_google();
    delay(100);

    //just check once on boot only.
    ddns_client.update();
}

void loop()
{
}
