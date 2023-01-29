cmake_minimum_required(VERSION 3.5)
idf_component_register(SRCS EspDDNS.cpp Esp32Util.cpp
                    INCLUDE_DIRS "./"
                    REQUIRES "arduino" # Library requires Arduino
                    )
project(EspDDNS)
