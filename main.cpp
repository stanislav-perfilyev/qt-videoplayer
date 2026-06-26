#include <QApplication>
#include <QWidget>
#include "ui_player.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QMainWindow window(nullptr);
    Ui::MainWindow player;
    player.setupUi(&window);
    window.show();
    return app.exec();
}
