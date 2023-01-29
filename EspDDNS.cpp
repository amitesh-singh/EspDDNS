/*
MIT License

Copyright (c) 2023 Amitesh Singh <singh[dot]amitesh[at]gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "EspDDNS.h"

#if defined(ESP32)
#include "IPv6Address.h"
#include "Esp32Util.h"
extern "C"{
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
}
#endif


namespace ddns
{
    IPAddress updater::getPublicIPv4()
    {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, public_ip_finder_url);
        int httpCode = http.GET();
        IPAddress public_ip;

        if (httpCode == HTTP_CODE_OK)
        {
            public_ip.fromString(http.getString());
        }

        http.end();

        return public_ip;
    }

    void updater::getEspIPv6()
    {
    #if defined(ESP8266)
    #if LWIP_IPV6
        for (auto addr : addrList)
        {
            if (!addr.isLocal() && addr.isV6()) 
            {
                esp_ipv6_.fromString(addr.toString());
                break;
            }
        }
    #endif
    #endif
    
    #if defined(ESP32)
        esp_ipv6_ = esp32experimental::globalIPv6();
    #endif
    }

    void updater::getEspIPv4(bool use_local_ip)
    {
        if (use_local_ip)
        {
            esp_ipv4_ = WiFi.localIP();
        }
        else
        {
            //get the public IP
            esp_ipv4_ = getPublicIPv4();
        }
    }

    void updater::getDDNSIPv4(const String &domain)
    {
        int ret = WiFi.hostByName(domain.c_str(), duckdns_ipv4_);
        if (ret != 1)
        {
            dns_success_ = false;
            return;
        }

        dns_success_ = true;
    }

    void updater::getDDNSIPv6(const String &domain)
    {
#if defined(ESP8266)
    #if LWIP_IPV6
        ip_addr_t ip6_dns;
        IP_ADDR6(&ip6_dns,PP_HTONL(0x20014860UL), PP_HTONL(0x48600000UL), PP_HTONL(0x00000000UL), PP_HTONL(0x00008888UL));

        dns_setserver(0, &ip6_dns);

        int  ret = WiFi.hostByName(domain.c_str(), duckdns_ipv6_);
        if (ret != 1)
        {
            dns_success_ = false;
            return;
        }
        dns_success_ = true;
    #endif
#endif
#if defined(ESP32)
    ip_addr_t ipv6_addr;
    bool  ret = esp32experimental::hostByName6(domain.c_str(), ipv6_addr);
    if (!ret)
    {
        dns_success_ = false;
        return;
    }

    dns_success_ = true;
    duckdns_ipv6_ = IPv6Address(ipv6_addr.u_addr.ip6.addr);
#endif
    }

    bool updater::connect(const String &url)
    {
        WiFiClient client;
        HTTPClient http;

        http.begin(client, url);
        int http_code = http.GET();
        bool ret = http_code != HTTP_CODE_OK ? false : true;

        http.end();

        return ret;
    }

    bool duckdns::update(const String &domain, const String &token, ip_type ipt, bool use_local_ip)
    {
        String url;
        
        if (ipt == ip_type::IPv6)
        {
#if defined(ESP8266)
        #if LWIP_IPV6
            getEspIPv6();
            getDDNSIPv6(domain);
            if (dns_success_ and esp_ipv6_ == duckdns_ipv6_) return false;

            url = "http://www.duckdns.org/update?domains=" + domain + "&token=" + token + "&ipv6=" + esp_ipv6_.toString();
        #else
            return false;
        #endif
#endif

#if defined(ESP32)
            getEspIPv6();
            getDDNSIPv6(domain);
            if (dns_success_ and esp_ipv6_ == duckdns_ipv6_) return false;

            url = "http://www.duckdns.org/update?domains=" + domain + "&token=" + token + "&ipv6=" + esp_ipv6_.toString();
#endif
        }
        else if (ipt == ip_type::IPv4)
        {
            getEspIPv4(use_local_ip);
            getDDNSIPv4(domain);
            if (dns_success_ and esp_ipv4_ == duckdns_ipv4_) return false;

            url = "http://www.duckdns.org/update?domains=" + domain + "&token=" + token + "&ip=" + esp_ipv4_.toString();
        }

        bool ret = connect(url);

        return ret;
    }

    bool client::update(bool use_local_ip)
    {
        bool ret { false };

        if (ip_type_ == ip_type::IPv4
            || ip_type_ == ip_type::IPv6)
        {
            ret = updater_->update(domain_, token_, ip_type_, use_local_ip);
            if (ret and cb_)
            {
               cb_(ip_type_ == ip_type::IPv6 ? updater_->ddns_ipv6().toString() : updater_->ddns_ipv4().toString(),
                    ip_type_== ip_type::IPv6 ? updater_->esp_ipv6().toString(): updater_->esp_ipv4().toString());
            }
        }
        else if (ip_type_ == ip_type::IPv4_and_v6)
        {
            ret = updater_->update(domain_, token_, ip_type::IPv4, use_local_ip);
            if (ret and cb_)
            {
                cb_(updater_->ddns_ipv4().toString(), updater_->esp_ipv4().toString());
            }

            ret = updater_->update(domain_, token_, ip_type::IPv6, use_local_ip);
            if (ret and cb_)
            {
                cb_(updater_->ddns_ipv6().toString(), updater_->esp_ipv6().toString());
            }
        }

        return ret;
    }

    String client::get_ipv6()
    {
    #if defined(ESP32)
        auto ip_addr = esp32experimental::globalIPv6();

        return ip_addr.toString();
    #elif defined(ESP8266)
      #if LWIP_IPV6
        for (auto addr : addrList)
        {
            if (!addr.isLocal() && addr.isV6()) 
            {
                return addr.toString();
            }
        }

        return "";
      #endif
    #endif
    }
}
