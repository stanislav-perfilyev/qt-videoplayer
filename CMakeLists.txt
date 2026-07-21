cmake_minimum_required(VERSION 3.16)
project(qt_videoplayer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# player.ui lives at the project root, not next to the .cpp files that
# include the generated ui_player.h (src/). AutoUic only searches the
# including file's own directory by default, so without this it fails on
# clean checkouts with "could not be found in the following directories".
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

# Qt installation path (Windows/MinGW only) — override via
# -DQT_PATH=<your Qt dir> or the QT_PATH environment variable.
# On Linux/CI, Qt6 is found via the system package manager instead.
if(NOT DEFINED QT_PATH)
    if(DEFINED ENV{QT_PATH})
        set(QT_PATH $ENV{QT_PATH})
    elseif(WIN32)
        set(QT_PATH "C:/Qt/6.7.0/mingw_64")
    endif()
endif()
if(DEFINED QT_PATH AND NOT QT_PATH STREQUAL "")
    list(APPEND CMAKE_PREFIX_PATH "${QT_PATH}/lib/cmake")
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Multimedia MultimediaWidgets)

set(WARNING_FLAGS
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

# ── Library (VideoPlayerWindow — testable without a running app) ─────────
# OBJECT (not STATIC): a Q_OBJECT class's vtable/staticMetaObject live in the
# AUTOMOC-generated mocs_compilation.cpp translation unit. When player_lib is
# a conventional static archive, the final executable link can fail with
# "undefined reference to vtable for VideoPlayerWindow" / "staticMetaObject"
# because the linker's single-pass archive-member extraction doesn't always
# pull that object file back in once player_lib's own object introduces the
# need for it. An OBJECT library sidesteps archive-member-order entirely —
# every consumer gets all of its compiled objects directly.
add_library(player_lib OBJECT
    src/videoplayer_window.cpp
    player.ui
)
target_include_directories(player_lib PUBLIC include)
target_link_libraries(player_lib PUBLIC
    Qt6::Core Qt6::Gui Qt6::Widgets
    Qt6::Multimedia Qt6::MultimediaWidgets)
target_compile_options(player_lib PRIVATE ${WARNING_FLAGS})

# ── Executable ────────────────────────────────────────────────────────────
add_executable(qt_videoplayer src/main.cpp)
target_link_libraries(qt_videoplayer PRIVATE player_lib)
target_compile_options(qt_videoplayer PRIVATE ${WARNING_FLAGS})
if(WIN32)
    set_target_properties(qt_videoplayer PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# ── Windows: copy Qt DLLs next to the .exe ───────────────────────────────
if(WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(QT_INSTALL_PATH "${QT_PATH}")
    foreach(QT_LIB Core Gui Widgets Multimedia MultimediaWidgets)
        add_custom_command(TARGET qt_videoplayer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}$<$<CONFIG:Debug>:d>.dll"
                "$<TARGET_FILE_DIR:qt_videoplayer>")
    endforeach()
endif()

# ── Unit tests (GTest) ────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_videoplayer_window tests/test_videoplayer_window.cpp)
target_link_libraries(test_videoplayer_window PRIVATE player_lib GTest::gtest)
target_compile_options(test_videoplayer_window PRIVATE ${WARNING_FLAGS})
include(GoogleTest)
gtest_discover_tests(test_videoplayer_window
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
