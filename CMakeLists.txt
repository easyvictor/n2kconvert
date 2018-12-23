#  The MIT License
#
#  Copyright (c) 2018 Jonathan Lovegren
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.

cmake_minimum_required(VERSION 3.10)
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()
set(PROJ_NAME "n2kconvert")
set(NMEA2000_DIR "${CMAKE_SOURCE_DIR}/../NMEA2000/src")
set(NMEA2000_SOCKETCAN_DIR "${CMAKE_SOURCE_DIR}/../NMEA2000_socketCAN")
set(NMEA0183_DIR "${CMAKE_SOURCE_DIR}/../NMEA0183")

project(${PROJ_NAME})

file(GLOB MAIN_SRC
    "src/BoardSerialNumber.cpp"
    "src/N2kDataToNMEA0183.cpp"
    "src/main.cpp"
)

add_subdirectory("${NMEA2000_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/buildNMEA2000")
add_subdirectory("${NMEA2000_SOCKETCAN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/buildNMEA2000socketCAN")
add_subdirectory("${NMEA0183_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/buildNMEA0183")

add_executable(${PROJECT_NAME} ${MAIN_SRC})

target_link_libraries(${PROJECT_NAME} nmea0183 nmea2000socketcan nmea2000)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

