
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

#if defined(ESP32)
#include <Arduino.h>
#include "Esp32Util.h"
#include <esp_netif.h>

esp_netif_t* get_esp_interface_netif(esp_interface_t interface);

namespace ddns
{
    IPv6Address esp32experimental::globalIPv6()
    {
        esp_ip6_addr_t addr;
        
        if(esp_netif_get_ip6_global(get_esp_interface_netif(ESP_IF_WIFI_STA), &addr))
        {
            return IPv6Address();
        }
        
        return IPv6Address(addr.addr);
    }

    void esp32experimental::wifi_dns6_found_callback(const char *name, const ip_addr_t *ipaddr, void *data)
    {
        dns_api_msg *msg = static_cast<dns_api_msg *>(data);

        if(ipaddr && !msg->result)
        {
            msg->ip_addr = *ipaddr;
            msg->result = 1;
        } 
        else
        {
            msg->result = -1;
        }
    }

    bool  esp32experimental::hostByName6(const char* hostname, ip_addr_t& result)
    {
        ip_addr_t addr;
        dns_api_msg arg = {};
        bool ip_found = true;

        //err_t err = dns_gethostbyname(aHostname, &addr, wifi_dns6_found_callback, &arg);
        err_t err = dns_gethostbyname_addrtype(hostname, &addr, wifi_dns6_found_callback, &arg,
                                                LWIP_DNS_ADDRTYPE_IPV6);

        if(err == ERR_OK)
        {
            result = addr;
        } 
        else if(err == ERR_INPROGRESS)
        {
            uint8_t retries = 0; //for 5s
            while (arg.result == 0) 
            {
                //wait here.
                delay(100);
                if (retries++ > 50)
                {
                    ip_found = false;
                    break;
                }
            }

            if (ip_found)
            {
                result = arg.ip_addr;
            }
        }

        return err == ERR_OK or (err == ERR_INPROGRESS and arg.result == 1 and ip_found == true);
    }
}
#endif
