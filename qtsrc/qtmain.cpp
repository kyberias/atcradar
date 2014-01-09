#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <time.h>
#include "mainwindow.h"

int main(int argc, char** argv) {
	srand(time(NULL));

    QApplication app(argc, argv);

    Kieli kieli("fi_FI");
    MainWindow window(kieli);

    window.resize(700, 700);
    window.setWindowTitle("ATC RADAR");
    window.show();

    return app.exec();}
