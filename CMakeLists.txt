cmake_minimum_required(VERSION 3.16)
project(Pos2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network Sql)

# include-пути
include_directories(
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/net
)

# Исходники: .cpp + .h (чтобы IDE их показывала)
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

# QML добавим в таргет, чтобы IDE их показывала (загрузка пойдет из .qrc)
file(GLOB QML_FILES "${CMAKE_SOURCE_DIR}/src/qml/*.qml")

add_executable(Pos2
    ${SRC_CPP}
    ${SRC_H}
    ${QML_FILES}
    resources.qrc
)

target_link_libraries(Pos2 PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Network Qt6::Sql
)

# В Debug включаем qml-логирование
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Pos2 PRIVATE QT_QML_DEBUG)
endif()

# Куда класть бинарь
set_target_properties(Pos2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# ✅ Копируем локальную БД из resources/db рядом с бинарем (опционально)
set(LOCAL_DB_SRC "${CMAKE_SOURCE_DIR}/resources/db/localdb.sqlite")
if(EXISTS "${LOCAL_DB_SRC}")
    add_custom_command(TARGET Pos2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LOCAL_DB_SRC}"
                "$<TARGET_FILE_DIR:Pos2>/localdb.sqlite"
        VERBATIM
    )
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
