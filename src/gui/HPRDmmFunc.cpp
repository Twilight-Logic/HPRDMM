#include "HPRDmmFunc.h"
#include "ui_HPRDmmFunc.h"

#include <QSettings>
#include <QStandardPaths>
#include <QApplication>
#include <QCoreApplication>

HPRDmmFunc::HPRDmmFunc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HPRDmmFunc)
{
    ui->setupUi(this);

    connect(ui->cmbFuncSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(readFunction()) );
    connect(ui->btnSave, SIGNAL(clicked(bool)), this, SLOT(saveFunction()) );
    connect(ui->btnClear, SIGNAL(clicked(bool)), this, SLOT(clrFunction()) );

    initFuncCfgBox();

}

HPRDmmFunc::~HPRDmmFunc()
{
    delete ui;
}


void HPRDmmFunc::initFuncCfgBox() {
    QString r;
    QStringList Data;
    QString item;

    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);

    for (int i=0; i<3; i++){
        item = "Function " + r.setNum(i+1);
        if (conf.status() == 0) {
            ui->cmbFuncSelect->addItem( item+": "+conf.value(item+"/FunctionName", "").toString() );
        }else{
            ui->cmbFuncSelect->addItem(item);
        }
    }

    ui->cmbFuncSelect->setCurrentIndex(0);
    readFunction();
}


void HPRDmmFunc::saveFunction() {
    QString f;
    QString func;
    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);

    func = "Function " + f.setNum(ui->cmbFuncSelect->currentIndex() + 1);

    conf.setValue(func+"/Mnemonic", ui->txtMnem->text() );
    conf.setValue(func+"/FunctionName", ui->txtName->text() );
    conf.setValue(func+"/FunctionScript", ui->tedFunc->document()->toPlainText() );

    conf.sync();

    func = func + ": " + ui->txtName->text();
    ui->cmbFuncSelect->setItemText(ui->cmbFuncSelect->currentIndex(), func);

}


void HPRDmmFunc::readFunction() {
    QString f;
    QString func;
    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);

    if (conf.status() == 0) {
        f.setNum(ui->cmbFuncSelect->currentIndex() + 1);
        func = "Function " + f;
        ui->txtMnem->setText(conf.value(func+"/Mnemonic", "").toString());
        ui->txtName->setText(conf.value(func+"/FunctionName", "").toString());
        ui->tedFunc->document()->setPlainText(conf.value(func+"/FunctionScript", "").toString());

        func = func + ": " + ui->txtName->text();
        ui->cmbFuncSelect->setItemText(ui->cmbFuncSelect->currentIndex(), func);
    }
}


void HPRDmmFunc::clrFunction() {
    QString f;
    QString func;
    f.setNum(ui->cmbFuncSelect->currentIndex() + 1);
    func = "Function " + f + ": ";
    ui->txtMnem->clear();
    ui->txtName->clear();
    ui->tedFunc->document()->clear();
    ui->cmbFuncSelect->setItemText(ui->cmbFuncSelect->currentIndex(), func);
}
