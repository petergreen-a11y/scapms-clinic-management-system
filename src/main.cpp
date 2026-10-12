#include <QApplication>
#include "gui/mainwindow.h"
int main(int argc,char*argv[]){QApplication app(argc,argv);clinic::MainWindow window;window.show();return app.exec();}
