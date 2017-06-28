#include "gui/HPRDmmDialog.h"
#include "gui/HPRDmmCfg.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget gui;
    gui.show();

    HPRDmmCfg cfg;
    cfg.hide();

    gui.updateFuncStr("VDC");
//    gui.updateMeterStr("-3.23845");



    return a.exec();
}
