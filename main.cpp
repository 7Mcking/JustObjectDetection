#include <QApplication>
#include "MainWindow.h"


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow window;
    window.setWindowTitle("Just Object Detection App");
    window.show();
    return QApplication::exec();
}
