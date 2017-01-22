#include "mainwidget.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>


#include <iostream>
#include "pdp.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w;

    w.show();
    return a.exec();
}
