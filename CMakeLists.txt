cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0091 NEW)
project(node_datachannel VERSION 1.0.5)
set(BUILD_SHARED_LIBS OFF)

add_compile_definitions(OPENSSL_API_COMPAT=0x10100001L)
add_compile_definitions(OPENSSL_CONFIGURED_API=0x30000000L)

include_directories(${CMAKE_JS_INC})

set(CMAKE_BUILD_TYPE Release)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_subdirectory(./external/libdatachannel)
set(NO_WEBSOCKET ON)
set(NO_EXAMPLES ON)
set(NO_TESTS ON)
set(USE_NICE OFF)

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
