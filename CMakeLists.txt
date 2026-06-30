cmake_minimum_required(VERSION 3.16)
project(qt_videoplayer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt installation path — override via -DCMAKE_PREFIX_PATH=<your Qt dir>
if(NOT DEFINED CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.7.0/mingw_64/lib/cmake" CACHE PATH "Qt prefix")
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Multimedia MultimediaWidgets)

# ── Library (VideoPlayerWindow) ───────────────────────────────────────────
add_library(player_lib
    src/videoplayer_window.cpp
    player.ui
)
target_include_directories(player_lib PUBLIC include)
target_link_libraries(player_lib PUBLIC
    Qt6::Core Qt6::Gui Qt6::Widgets
    Qt6::Multimedia Qt6::MultimediaWidgets)

# ── Executable ────────────────────────────────────────────────────────────
add_executable(qt_videoplayer src/main.cpp)
target_link_libraries(qt_videoplayer PRIVATE player_lib)
set_target_properties(qt_videoplayer PROPERTIES WIN32_EXECUTABLE TRUE)

# ── Windows: copy Qt DLLs next to the .exe ───────────────────────────────
if(WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    foreach(QT_LIB Core Gui Widgets Multimedia MultimediaWidgets)
        add_custom_command(TARGET qt_videoplayer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}$<$<CONFIG:Debug>:d>.dll"
                "$<TARGET_FILE_DIR:qt_videoplayer>")
    endforeach()
endif()

# ── .gitignore reminder: build/ should never be committed ─────────────────
