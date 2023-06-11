cmake_minimum_required(VERSION 3.16)
project(node_datachannel VERSION 1.0.7)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# add_compile_definitions(OPENSSL_API_COMPAT=0x10100001L)
# add_compile_definitions(OPENSSL_CONFIGURED_API=0x30000000L)

include_directories(${CMAKE_JS_INC})

set(CMAKE_BUILD_TYPE Release)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

set(NO_WEBSOCKET ON)
set(NO_EXAMPLES ON)
set(NO_TESTS ON)
set(USE_NICE OFF)
set(USE_SYSTEM_JUICE OFF)
set(ENABLE_LOCALHOST_ADDRESS ON)

add_subdirectory(./external/libdatachannel)

add_library(${PROJECT_NAME} SHARED
    src/rtc-wrapper.cpp
    src/media-direction.cpp
    src/media-rtcpreceivingsession-wrapper.cpp
    src/media-rtcpsrreporter-wrapper.cpp
    src/media-track-wrapper.cpp
    src/media-audio-wrapper.cpp
    src/media-video-wrapper.cpp
    src/data-channel-wrapper.cpp
    src/peer-connection-wrapper.cpp
    src/thread-safe-callback.cpp
    src/main.cpp
    ${CMAKE_JS_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_compile_definitions(juice PRIVATE JUICE_ENABLE_LOCALHOST_ADDRESS=1)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api
    ${CMAKE_BINARY_DIR}/_deps/libdatachannel-src/include
)

set(LINK_LIBRARIES
    ${CMAKE_JS_LIB}
    datachannel-static
)

list(APPEND LINK_LIBRARIES -static-libgcc -static-libstdc++)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})
