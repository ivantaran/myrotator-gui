/*
 * main.cpp
 *
 *  Created on: Jun 29, 2019
 *      Author: taran
 */


#include <QApplication>

#include "Monster.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow mw;
    mw.show();
    return app.exec();
}
