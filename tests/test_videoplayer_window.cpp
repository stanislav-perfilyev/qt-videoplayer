/**
 * @file test_videoplayer_window.cpp
 * @brief Unit tests for VideoPlayerWindow::loadFile().
 *
 * VideoPlayerWindow is a QMainWindow subclass, so constructing it requires a
 * live QApplication even when nothing is shown. Tests run headless via
 * QT_QPA_PLATFORM=offscreen (set in CI) — no real display is needed.
 */
#include <gtest/gtest.h>
#include <QApplication>
#include "videoplayer_window.h"

namespace {

/// Owns the single QApplication instance required by any QWidget subclass.
class QtEnvironment : public ::testing::Environment {
public:
    void SetUp() override
    {
        static int argc = 1;
        static char argv0[] = "test_videoplayer_window";
        static char* argv[] = {argv0};
        app = std::make_unique<QApplication>(argc, argv);
    }
    void TearDown() override { app.reset(); }

private:
    std::unique_ptr<QApplication> app;
};

}  // namespace

TEST(VideoPlayerWindow, LoadFileRejectsEmptyPath)
{
    VideoPlayerWindow window;
    EXPECT_FALSE(window.loadFile(QString()));
}

TEST(VideoPlayerWindow, LoadFileAcceptsNonEmptyPath)
{
    // loadFile() only validates the path is non-empty and hands it to the
    // async media backend — actual playback errors surface later via
    // errorOccurred(), not the return value. A non-existent-but-non-empty
    // path is therefore still expected to return true per the documented
    // contract in videoplayer_window.h.
    VideoPlayerWindow window;
    EXPECT_TRUE(window.loadFile(QStringLiteral("/tmp/does_not_exist.mp4")));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new QtEnvironment());
    return RUN_ALL_TESTS();
}
