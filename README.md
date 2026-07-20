# Qt6 Video Player

A desktop video player built with Qt 6 Widgets + Qt Multimedia — play/pause/stop,
±10 s seek, and a live progress bar backed by `QMediaPlayer`.

## Architecture

`VideoPlayerWindow` (`include/videoplayer_window.h`, `src/videoplayer_window.cpp`)
wraps a `QMediaPlayer` + `QAudioOutput` and a `QVideoWidget` swapped into the
`.ui`-designed layout at construction time. All Qt-owned objects (`player_`,
`audioOutput_`, the video widget) are parented to the window and destroyed
automatically by Qt's object tree — no manual cleanup needed. `ui_` is the
generated `Ui::MainWindow`, held by `std::unique_ptr` since it has no Qt
parent of its own.

`loadFile()` is the only method with app-independent logic (validates the
path, hands it to the async media backend) and is exercised directly by the
test suite.

## Usage

1. Launch the app.
2. **File → Open…** (or the toolbar) to pick a video file.
3. Play / Pause / Stop, or seek ±10 s with the rewind/forward buttons.
4. The progress bar tracks playback position; duration sets its range.

Supported containers depend on the Qt Multimedia backend installed on the
host (GStreamer on Linux, Media Foundation on Windows).

## Build and run

### Linux

```bash
sudo apt-get install qt6-base-dev qt6-multimedia-dev
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/qt_videoplayer
```

### Windows (MinGW)

```powershell
cmake -B build -DQT_PATH="C:/Qt/6.7.0/mingw_64" -G "MinGW Makefiles"
cmake --build build --parallel
./build/qt_videoplayer.exe
```

`QT_PATH` can also be set as an environment variable instead of a CMake
argument.

## Tests

```bash
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

Tests construct `VideoPlayerWindow` off-screen (`QT_QPA_PLATFORM=offscreen`)
and cover `loadFile()`'s validation contract: empty paths are rejected,
non-empty paths are accepted and handed to the async backend (playback
errors, if any, surface later via `errorOccurred()`).

## Prerequisites

- CMake ≥ 3.16, C++20 compiler (GCC 14 / Clang 18 tested in CI)
- Qt 6 with the Widgets and Multimedia modules
- GoogleTest (fetched automatically via CMake `FetchContent`)
