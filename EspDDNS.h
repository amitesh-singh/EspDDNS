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

#pragma once

#include <Arduino.h>
#include <IPAddress.h>
#include <functional>

#if defined(ESP8266)
  #include "ESP8266WiFi.h"
  #include "ESP8266HTTPClient.h"
#elif defined(ESP32)
  #include "WiFi.h"
  #include "HTTPClient.h"
  #include <esp_netif.h>
#include <IPv6Address.h>

#endif
#if defined(ESP8266)
    #if LWIP_IPV6
        #include "lwip/ip_addr.h"
        #include <lwip/dns.h>
        #include <AddrList.h>
        #include "lwip/err.h"
        #include "lwip/dhcp.h"
    #endif
#endif

namespace ddns
{
    enum class type
    {
        DUCKDNS, //duckdns.org
        DYNV6,  //dynv6.com
    };

    enum class ip_type
    {
        IPv6,
        IPv4,
        IPv4_and_v6
    };

    class updater
    {
        constexpr const static auto public_ip_finder_url { "http://ifconfig.me/ip" };

        protected:

        bool dns_success_ = false;
    #if defined(ESP8266)
        IPAddress esp_ipv6_;
        IPAddress duckdns_ipv6_;
    #elif defined(ESP32)
        IPv6Address esp_ipv6_;
        IPv6Address duckdns_ipv6_;
    #endif
        IPAddress esp_ipv4_;
        IPAddress duckdns_ipv4_;

        IPAddress getPublicIPv4();
        void getEspIPv4(bool use_local_ip);
        void getEspIPv6();
        void getDDNSIPv4(const String &domain);
        void getDDNSIPv6(const String &domain);

        bool connect(const String &url);

        public:
#if defined(ESP8266)
        IPAddress &esp_ipv6()
        {
            return esp_ipv6_;
        }

        IPAddress &ddns_ipv6()
        {
            return duckdns_ipv6_;
        }
#elif defined(ESP32)
  IPv6Address &esp_ipv6()
        {
            return esp_ipv6_;
        }

        IPv6Address &ddns_ipv6()
        {
            return duckdns_ipv6_;
        }
#endif
        IPAddress &esp_ipv4()
        {
            return esp_ipv4_;
        }

        IPAddress &ddns_ipv4()
        {
            return duckdns_ipv4_;
        }

        virtual bool update(const String &domain, const String& token, ip_type, bool) = 0;
        virtual ~updater() {}
    };

    class duckdns: public updater
    {
        public:
        bool update(const String &domain, const String &token, ip_type ipt, bool use_local_ip) override;
    };

    class dynv6: public updater
    {
        public:
        bool update(const String &domain, const String &token, ip_type ipt, bool use_local_ip) override;
    };

    class client
    {
        public:
        using cb_t = std::function<void (String oldIp, String newIp)>;
        
        private:
        type type_;
        String domain_;
        String token_;
        ip_type ip_type_;

        cb_t cb_;
        updater *updater_;

        public:
        client(type t, String &&domain, String &&token, ip_type ipt = ip_type::IPv6): 
            type_(t),
            domain_(std::move(domain)),
            token_(std::move(token)),
            ip_type_(ipt)
        {
            if (type_ == type::DUCKDNS)
            {
                updater_ = new duckdns();
            }
            else if (type_ == type::DYNV6)
            {
                updater_ = new dynv6();
            }
        #if defined(ESP32)
            if (ip_type_ == ip_type::IPv6)
            {
                //enable sta ipv6 here
               // WiFi.enableIpV6();
            }
        #endif
        }

        void onUpdate(cb_t cb)
        {
            cb_ = cb;
        }
  
        void wait()
        {
          for (bool configured = false; !configured;) {
            for (auto addr : addrList)
              if (ip_type_ == ip_type::IPv6 or ip_type_ == ip_type::IPv4_and_v6) {
                if (configured = !addr.isLocal() && addr.isV6()) {
                  break;
                }
              } else if (ip_type_ == ip_type::IPv4) {
                if (configured = !addr.isLocal()) {
                  break;
              }
            }
            delay(100);
          }
        }


        bool update(bool use_local_ip = false);
        String get_ipv6();
        ~client()
        {
            delete updater_;
        }
    };
}
