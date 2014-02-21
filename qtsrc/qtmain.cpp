#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <qdesktopwidget.h>
#include <QTranslator>

#include <ctime>
#include "mainwindow.hpp"

int main(int argc, char** argv) {
    std::srand(std::time(NULL));

    try {
        QApplication app(argc, argv);
        QTranslator kaantaja;
        kaantaja.load("radar_en");
        MainWindow window;

        window.resize(800, 750);
        window.setWindowTitle("ATC RADAR");
        window.show();

        app.installTranslator(&kaantaja);

        return app.exec();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
