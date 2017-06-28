#include "HPRDmmSprof.h"
#include "ui_HPRDmmSprof.h"

#include <QSettings>
#include <QStandardPaths>
#include <QApplication>
#include <QCoreApplication>

HPRDmmSprof::HPRDmmSprof(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HPRDmmSprof)
{
    ui->setupUi(this);

    cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";

    connect(ui->cmbProfSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(readProfile()) );
    connect(ui->btnSave, SIGNAL(clicked(bool)), this, SLOT(saveProfile()) );
    connect(ui->btnDefault, SIGNAL(clicked(bool)), this, SLOT(rstProfile()) );
    connect(ui->btnDel, SIGNAL(clicked(bool)), this, SLOT(delProfile()) );
    connect(ui->cmbFuncs, SIGNAL(currentIndexChanged(int)), this, SLOT(setRange()) );

    initProfCfgBox();

}

HPRDmmSprof::~HPRDmmSprof()
{
    delete ui;
}


void HPRDmmSprof::initProfCfgBox() {
    QStringList Data;
    QString prof;
    QStringList cmbList;

    // Populate manual range info
    rangesADC << "AUTO" << "300mA" << "3A";
    rangesAAC << "AUTO" << "300mA" << "3A";
    rangesOHM << "AUTO" << "30Ω" << "300Ω" << "3kΩ" << "30kΩ" << "300kΩ" << "3MΩ" << "30MΩ";
    rangesVDC << "AUTO" << "30mV" << "300mV" << "3V" << "30V" << "300V";
    rangesVAC << "AUTO" << "300mV" << "3V" << "30V" << "300V";

    // Populate function combo
    cmbList << "DC Voltage" << "AC Voltage" << "Ohms - 2Wire" << "Ohms - 4Wire" << "DC Current" << "AC Current" << "Ohms - Extended";
    ui->cmbFuncs->addItems(cmbList);

    // Populate range combo
    setRange();

    // Populate trigger combo
    cmbList.clear();
    cmbList << "Internal Trig" << "Single Trig" << "External Trig" << "Hold Trig" << "Fast Trig";
    ui->cmbTrigOpts->addItems(cmbList);

    // Populate display mode combo
    cmbList.clear();
    cmbList << "Real Value" << "Maximum Value" << "Average Value" << "Minimum Value";
    ui->cmbDisplayMode->addItems(cmbList);

    // Get list of profiles
    QSettings conf(cfgFile, QSettings::IniFormat);
    if (conf.status() == 0) {
        conf.beginGroup("");
        foreach (const QString &group, conf.childGroups()) {
            if (group.contains("Profile:")) {
                ui->cmbProfSelect->addItem(group);
//                ui->plainTextEdit->appendPlainText(group);
            }
        }
    }
    // Set to index zero and display first profile
    ui->cmbProfSelect->setCurrentIndex(0);
    readProfile();
}


void HPRDmmSprof::setRange(){
    // Set the range for a given function
    int idx = ui->cmbFuncs->currentIndex();

    // Clear the combo
    ui->cmbRange->clear();
    // Copy in the appropriate range
    switch (idx) {
    case 0: ui->cmbRange->addItems(rangesVDC);
        break;
    case 1: ui->cmbRange->addItems(rangesVAC);
        break;
    case 2: ui->cmbRange->addItems(rangesOHM);
        break;
    case 3: ui->cmbRange->addItems(rangesOHM);
        break;
    case 4: ui->cmbRange->addItems(rangesADC);
        break;
    case 5: ui->cmbRange->addItems(rangesAAC);
        break;
    case 6: ui->cmbRange->addItem("AUTO");
        break;
    }
    // Set index to zero
    ui->cmbRange->setCurrentIndex(0);

}


void HPRDmmSprof::saveProfile() {
    QString prof;
    QSettings conf(cfgFile, QSettings::IniFormat);

    if (ui->txtProfName->text() != "Default" && ui->txtProfName->text() != "Instrument") {

        prof = "Profile: " + ui->txtProfName->text();

        conf.setValue(prof+"/ProfileName", ui->txtProfName->text() );
        conf.setValue(prof+"/Function", ui->cmbFuncs->currentIndex()+1 );
        conf.setValue(prof+"/DisplayDigits", ui->spnDigits->value() );
        conf.setValue(prof+"/MeterRange", ui->cmbRange->currentIndex() );
        conf.setValue(prof+"/TriggerMode", ui->cmbTrigOpts->currentIndex() );
        conf.setValue(prof+"/DisplayMode", ui->cmbDisplayMode->currentIndex() );
        conf.setValue(prof+"/AutoZeroOff", ui->chkAzero->isChecked() );
        conf.setValue(prof+"/MeterDisplayOff", ui->chkDisp->isChecked() );

        conf.sync();

        if (ui->cmbProfSelect->findText(prof) < 0) {
            ui->cmbProfSelect->addItem(prof);
            ui->cmbProfSelect->setCurrentText(prof);
        }
    }
}


void HPRDmmSprof::readProfile() {
    QString prof;
    QSettings conf(cfgFile, QSettings::IniFormat);

    if (conf.status() == 0) {
        prof = ui->cmbProfSelect->currentText();
        ui->txtProfName->setText(conf.value(prof+"/ProfileName", "").toString());
        ui->cmbFuncs->setCurrentIndex(conf.value(prof+"/Function", 1).toInt()-1);
        ui->spnDigits->setValue(conf.value(prof+"/DisplayDigits", 5).toInt());
        ui->cmbRange->setCurrentIndex(conf.value(prof+"/MeterRange", 0).toInt());
        ui->cmbTrigOpts->setCurrentIndex(conf.value(prof+"/TriggerMode", 0).toInt());
        ui->cmbDisplayMode->setCurrentIndex(conf.value(prof+"/DisplayMode", 0).toInt());
        ui->chkAzero->setChecked(conf.value(prof+"/AutoZeroOff", false).toBool());
        ui->chkDisp->setChecked(conf.value(prof+"/MeterDisplayOff", false).toBool());

    }
}

void HPRDmmSprof::rstProfile(){
    ui->txtProfName->clear();
    ui->cmbFuncs->setCurrentIndex(0);
    ui->spnDigits->setValue(5);
    ui->cmbRange->setCurrentIndex(0);
    ui->cmbTrigOpts->setCurrentIndex(0);
    ui->cmbDisplayMode->setCurrentIndex(0);
    ui->chkAzero->setChecked(false);
    ui->chkDisp->setChecked(false);
}


void HPRDmmSprof::delProfile() {
    QString prof;
    QSettings conf(cfgFile, QSettings::IniFormat);

    prof = ui->cmbProfSelect->currentText();
    ui->txtProfName->clear();
    ui->cmbProfSelect->removeItem(ui->cmbProfSelect->currentIndex());
    conf.beginGroup(prof);
    conf.remove("");
    conf.endGroup();
    readProfile();

    ui->cmbProfSelect->setCurrentIndex(0);
}
