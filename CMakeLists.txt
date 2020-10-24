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

project("n2kconvert")

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

set(NMEA2000_DIR "${CMAKE_SOURCE_DIR}/../NMEA2000/src")
set(NMEA2000_SOCKETCAN_DIR "${CMAKE_SOURCE_DIR}/../NMEA2000_socketCAN")
set(NMEA0183_DIR "${CMAKE_SOURCE_DIR}/../NMEA0183")

set(MAIN_SRC
    "src/BoardSerialNumber.cpp"
    "src/N2kDataToNMEA0183.cpp"
    "src/Options.cpp"
    "src/N2kConvert.cpp"
)

set(BIN_FILES
	"bin/n2kconvert-fifos.sh"
)

set(CONF_FILES
	"config/n2kconvert.conf"
)
set(SERVICE_FILES
	"config/n2kconvert.service"
	"config/n2kconvert-fifos.service"
)

find_package( Boost 1.65.1 COMPONENTS program_options REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )
add_subdirectory("${NMEA2000_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/buildNMEA2000")
add_subdirectory("${NMEA2000_SOCKETCAN_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/buildNMEA2000socketCAN")
add_subdirectory("${NMEA0183_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/buildNMEA0183")

add_executable(${PROJECT_NAME} ${MAIN_SRC} ${CONF_FILES})
target_link_libraries(${PROJECT_NAME}
	nmea0183
	nmea2000socketcan
	nmea2000
	${Boost_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION /usr/bin COMPONENT binaries)
install(FILES ${BIN_FILES} DESTINATION /usr/bin COMPONENT binaries PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
install(FILES ${CONF_FILES} DESTINATION /etc COMPONENT config PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(FILES ${SERVICE_FILES} DESTINATION /etc/systemd/system COMPONENT config PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# CPack configuration
set(CPACK_GENERATOR "DEB")
if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm")
	message("Bulding armhf deb file.")
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
else()
	message("Building x86 deb file.")
endif()
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "6")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
set(CPACK_PACKAGE_DESCRIPTION "Daemon to conver NMEA2000 to NMEA0183")
set(CPACK_PACKAGE_CONTACT "Jonathan Lovegren jalovegren@gmail.com")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/config/conffiles;${CMAKE_CURRENT_SOURCE_DIR}/config/postinst")
include(CPack)
