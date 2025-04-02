# EspDDNS

IPV6 enabled DDNS client for ESP8266 and ESP32.

## Supported DDNS providers:
- duckdns  -> `Fully supported` https:://duckdns.org
- dynv6    -> `Fully supported` https://dynv6.com/

This only works if IPv6 is supported by your ISP. [https://test-ipv6.com/](https://test-ipv6.com/)
## esp8266 example
- For IPv6 support in esp8266, enable lwpv2 IP6 low memory option in arduino ide. <br>
  `Tools->lwIP variant->v2 IP6 low memory`

```
#include <EspDDNS.h>

static constexpr const auto ssid { "******" };
static constexpr const auto passwd { "*****" };

void set_dnsserver_google()
{
    //set dns server
    ip_addr_t ip6_dns;
    // GOOGLE EXTERNAL - 2001:4860:4860::8888
    IP_ADDR6(&ip6_dns,PP_HTONL(0x20014860UL), PP_HTONL(0x48600000UL), 
             PP_HTONL(0x00000000UL), PP_HTONL(0x00008888UL));
    dns_setserver(0, &ip6_dns);
}

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, passwd);
    
    // In case of ipv6, do enable lwp2 option in arduino ide.
    // IPv6 is by default
    // ddns::client ddns_client(ddns::type::DUCKDNS, "yourdomain.duckdns.org", "yourduckdnstoken");
    ddns::client ddns_client(ddns::type::DUCKDNS, "yourdomain.duckdns.org",
                             "yourduckdnstoken", ddns::ip_type::IPv6);
    ddns_client.onUpdate([](String old_ip, String new_ip) {
        Serial.print("Ip address is changed to: ");
        Serial.print(new_ip);
    });

    // wait for the esp8266 to get the ipv6 address
    ddns_client.wait();

    Serial.print("Connected.., local ip: ");
    Serial.println(WiFi.localIP());

    set_dnsserver_google();

    //just check once on boot only.
    ddns_client.update();
}

void loop()
{
}
```
## ESP32
- IPv6 is not fully enabled in ESP32 arduino SDK. You have to compile arduino as a component under `esp-idf`.

IPV6 must be enabled at esp-idf level:
- `CONFIG_LWIP_IPV6`
- `CONFIG_LWIP_IPV6_AUTOCONFIG=y`
- `CONFIG_LWIP_IPV6_RDNSS_MAX_DNS_SERVERS=2`

~~Refer [examples/esp32_duckdns_ipv6](examples/esp32_duckdns_ipv6/esp32_duckdns_ipv6.ino)~~

Refer to [EspDDNS with esp-idf](https://github.com/amitesh-singh/EspDDNS_with_IDF)

# Support me

[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/singhamiteK)
