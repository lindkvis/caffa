
#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle("CAFFA Framework Test Application");
    window.resize(1000, 810);
    window.show();

    return app.exec();
}
