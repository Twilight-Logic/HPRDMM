#ifndef HPRDMMFUNC_H
#define HPRDMMFUNC_H

#include <QDialog>

namespace Ui {
class HPRDmmFunc;
}

class HPRDmmFunc : public QDialog
{
    Q_OBJECT

public:
    explicit HPRDmmFunc(QWidget *parent = 0);
    ~HPRDmmFunc();



private:
    Ui::HPRDmmFunc *ui;

    void initFuncCfgBox();

private slots:

    void saveFunction();
    void readFunction();
    void clrFunction();


};

#endif // HPRDMMFUNC_H
