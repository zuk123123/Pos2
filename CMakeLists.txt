cmake_minimum_required(VERSION 3.16)
project(Pos2 LANGUAGES CXX)

# --- Qt / C++ ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC_OPTIONS --compress-algo=zlib --compress 9)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network Sql QuickControls2)

# --- инклюды ---
include_directories(
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/net
    ${CMAKE_SOURCE_DIR}/src/app
)

# --- исходники ---
file(GLOB SRC_CPP
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/net/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/app/*.cpp"
)
file(GLOB SRC_H
    "${CMAKE_SOURCE_DIR}/src/core/*.h"
    "${CMAKE_SOURCE_DIR}/src/net/*.h"
    "${CMAKE_SOURCE_DIR}/src/app/*.h"
)
file(GLOB QML_FILES "${CMAKE_SOURCE_DIR}/src/qml/*.qml")

qt_add_executable(Pos2
    MANUAL_FINALIZATION
    ${SRC_CPP}
    ${SRC_H}
    ${QML_FILES}
    resources.qrc
)

target_link_libraries(Pos2 PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Network Qt6::Sql
)

# === Настройки базового URL ===
# Desktop -> localhost, Android -> IP в вашей сети (меняется флагом -DPOS2_ANDROID_LAN_IP=192.168.x.y)
set(POS2_DESKTOP_HOST "127.0.0.1" CACHE STRING "Desktop host for API base")
set(POS2_ANDROID_LAN_IP "192.168.100.184" CACHE STRING "LAN IP of server machine for Android")
set(POS2_PORT "8080" CACHE STRING "API port")

if(ANDROID)
    set(POS2_EXT_URL "http://${POS2_ANDROID_LAN_IP}:${POS2_PORT}")
else()
    set(POS2_EXT_URL "http://${POS2_DESKTOP_HOST}:${POS2_PORT}")
endif()

message(STATUS "POS2_EXT_URL = ${POS2_EXT_URL}")

if(ANDROID)
    target_compile_definitions(Pos2 PRIVATE
        POS2_API_BASE_DEFAULT="${POS2_EXT_URL}"
        POS2_PLATFORM_ANDROID=1
    )
else()
    target_compile_definitions(Pos2 PRIVATE
        POS2_API_BASE_DEFAULT="${POS2_EXT_URL}"
        POS2_PLATFORM_DESKTOP=1
    )
endif()

# --- удобно для desktop (бинарь в build/) ---
set_target_properties(Pos2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# --- локальная БД рядом с бинарём на desktop (если есть исходник) ---
set(LOCAL_DB_SRC "${CMAKE_SOURCE_DIR}/resources/db/localdb.sqlite")
if(EXISTS "${LOCAL_DB_SRC}")
    add_custom_command(TARGET Pos2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LOCAL_DB_SRC}"
                "$<TARGET_FILE_DIR:Pos2>/localdb.sqlite"
    )
endif()

qt_finalize_executable(Pos2)
