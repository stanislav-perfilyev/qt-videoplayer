#include "videoplayer_window.h"
#include <QApplication>
#include <QLoggingCategory>

namespace {
constexpr int kDefaultWindowWidth  = 900;
constexpr int kDefaultWindowHeight = 650;
}  // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Qt6 Video Player");
    app.setOrganizationName("perfilyev-dev");

    // Enable info-level logging for the player category.
    // Override via environment: QT_LOGGING_RULES="app.player.debug=true"
    QLoggingCategory::setFilterRules("app.player.info=true");

    VideoPlayerWindow window;
    window.resize(kDefaultWindowWidth, kDefaultWindowHeight);
    window.show();

    return app.exec();
}
