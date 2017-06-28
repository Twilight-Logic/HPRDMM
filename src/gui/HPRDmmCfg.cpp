#include "HPRDmmCfg.h"
#include "ui_HPRDmmCfg.h"

#include "HPRDmmDialog.h"
#include "HPRDmmSprof.h"

#include <QSettings>
#include <QStandardPaths>
#include <QApplication>
#include <QCoreApplication>
#include <QtSerialPort/QSerialPortInfo>


HPRDmmCfg::HPRDmmCfg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HPRDmmCfg)
{
    ui->setupUi(this);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveConfig()) );
    connect(ui->chkLogEnabled, SIGNAL(stateChanged(int)), this, SLOT(setLogCfgStatus()) );
    connect(ui->btnSprofCfg, SIGNAL(clicked(bool)), this, SLOT(selectSprofCfg()) );
    connect(ui->chkExoReal, SIGNAL(stateChanged(int)), this, SLOT(enableR1Cfg()) );
    connect(ui->btnReadCal, SIGNAL(clicked(bool)), this, SLOT(readCalData()) );

    initCfgBox();

}

HPRDmmCfg::~HPRDmmCfg()
{
    delete ui;
}


void HPRDmmCfg::initCfgBox(){
    QStringList Data;
    Data << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "10";
    Data << "11" << "12" << "13" << "14" << "15" << "16" << "17" << "18" << "19" << "20";
    Data << "21" << "22" << "23" << "24" << "25" << "26" << "27" << "28" << "29" << "30";
    ui->cmbGpibAddr->addItems(Data);
    Data.clear();
    ui->txtIDStr->setText("ARDUINO GPIB");
    Data << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    ui->cmbBaud->addItems(Data);
    Data.clear();
    Data << "N" << "E" << "O" << "M" << "S";
    ui->cmbParity->addItems(Data);
    Data.clear();
    Data << "5" << "6" << "7" << "8";
    ui->cmbDbits->addItems(Data);
    Data.clear();
    Data << "1" << "1.5" << "2";
    ui->cmbSbits->addItems(Data);
    Data.clear();
    Data << "None" << "XONXOFF" <<"CRTSCTS";
    ui->cmbFlow->addItems(Data);
    Data.clear();
    Data << "msec" << "sec" << "min";
    ui->cmbLogIntUnit->addItems(Data);
    Data.clear();
    Data << "Default" << "Instrument";
    ui->cmbSp->addItems(Data);
    Data.clear();
    Data << "Default" << "White" << "Blue-White" << "Blue-Yellow" << "Green" << "Yellow" << "LCD" << "LED" << "PLasma" << "Custom";
    ui->cmbDispStyle->addItems(Data);
    Data.clear();

    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
        ui->cmbPort->addItem(port.portName());
    }

    loadConfig();

}


void HPRDmmCfg::saveConfig () {

    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);

    // Comms
    conf.setValue("GPIB/GpibAddr", ui->cmbGpibAddr->currentText());
    conf.setValue("GPIB/ControllerIDString", ui->txtIDStr->text());
    conf.setValue("Serial/Port", ui->cmbPort->currentText());
    conf.setValue("Serial/BaudRate", ui->cmbBaud->currentText());
    conf.setValue("Serial/Parity", ui->cmbParity->currentText());
    conf.setValue("Serial/DataBits", ui->cmbDbits->currentText());
    conf.setValue("Serial/StopBits", ui->cmbSbits->currentText());
    conf.setValue("Serial/FlowControl", ui->cmbFlow->currentText());

    // Logging
    if (ui->chkLogEnabled->isChecked()) {
        conf.setValue("Logging/Enabled", "1");
        conf.setValue("Logging/File", ui->txtLogFile->text());
        conf.setValue("Logging/Interval", ui->txtLogInt->text());
        conf.setValue("Logging/IntUnit", ui->cmbLogIntUnit->currentIndex());
    }else{
        conf.setValue("Logging/Enabled", "0");
        conf.remove("Logging/File");
        conf.remove("Logging/Interval");
        conf.remove("Logging/IntUnit");
    }

    // Start-up
    conf.setValue("StartUp/Profile", ui->cmbSp->currentText());
    conf.setValue("StartUp/DisplayStyle", ui->cmbDispStyle->currentIndex());

    // Extended Ohms
    if(ui->chkExoReal->isChecked()) {
        conf.setValue("Advanced/ExoRealValue", "1");
        conf.setValue("Advanced/ExoR1Value", ui->spnExoR1Val->value());
    }else{
        conf.setValue("Advanced/ExoRealValue", "0");
        conf.setValue("Advanced/ExoR1Value", ui->spnExoR1Val->value());
    }

    // Continuity
    conf.setValue("Advanced/ContBeepEnable", ui->chkEnableBeep->isChecked());
    conf.setValue("Advanced/ContThreshold", ui->spnContThreshold->value());

    conf.sync();

    this->accept();
}


void HPRDmmCfg::loadConfig () {

    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);

    if (conf.status() == 0) {  // Read config OK
        QString gpibAddr = conf.value("GPIB/GpibAddr", "2").toString();
        QString ctrlIDStr = conf.value("GPIB/GpibControllerIdStr", "ARDUINO GPIB").toString();
        QString serialPort = conf.value("Serial/Port", "").toString();
        QString serialBaud = conf.value("Serial/BaudRate", "9600").toString();
        QString serialParity = conf.value("Serial/Parity", "N").toString();
        QString serialDbits = conf.value("Serial/DataBits", "8").toString();
        QString serialSbits = conf.value("Serial/StopBits", "1").toString();
        QString serialFlow = conf.value("Serial/FlowContol", "None").toString();
        bool logEn = conf.value("Logging/Enabled", "0").toBool();
        QString logFile = conf.value("Logging/File", "").toString();
        QString logInt = conf.value("Logging/Interval", "1000").toString();
        int logIntUnit = conf.value("Logging/IntUnit", 0).toInt();
        QString profile = conf.value("StartUp/Profile").toString();
        int dispStyle = conf.value("StartUp/DisplayStyle").toInt();
        bool exoEn = conf.value("Advanced/exoRealValue").toBool();
        bool enBeep = conf.value("Advanced/ContBeepEnable").toBool();
        int contThr = conf.value("Advanced/ContThreshold", 50).toInt();

        ui->cmbGpibAddr->setCurrentIndex(ui->cmbGpibAddr->findText(gpibAddr));
        ui->cmbPort->setCurrentIndex(ui->cmbPort->findText(serialPort));
        ui->cmbBaud->setCurrentIndex(ui->cmbBaud->findText(serialBaud));
        ui->cmbParity->setCurrentIndex(ui->cmbParity->findText(serialParity));
        ui->cmbDbits->setCurrentIndex(ui->cmbDbits->findText(serialDbits));
        ui->cmbSbits->setCurrentIndex(ui->cmbSbits->findText(serialSbits));
        ui->cmbFlow->setCurrentIndex(ui->cmbFlow->findText(serialFlow));
        ui->chkLogEnabled->setChecked(logEn);
        setLogCfgStatus();
        ui->txtLogFile->setText(logFile);
        ui->txtLogInt->setText(logInt);
        ui->cmbLogIntUnit->setCurrentIndex(logIntUnit);
        // Get list of profiles
        getProfileList();
        // Set combo to selected profile
        ui->cmbSp->setCurrentText(profile);
        ui->cmbDispStyle->setCurrentIndex(dispStyle);
        // Extended Ohms
        ui->chkExoReal->setChecked(exoEn);
        if (exoEn) ui->spnExoR1Val->setValue(conf.value("Advanced/exoR1Value", 10.00000).toDouble());
        // Continuity
        ui->chkEnableBeep->setChecked(enBeep);
        ui->spnContThreshold->setValue(contThr);

    }
}

void HPRDmmCfg::setLogCfgStatus() {
    // Enable/disable log configuration
    if (ui->chkLogEnabled->isChecked()) {
        ui->labLogF->setEnabled(true);
        ui->labLogInt->setEnabled(true);
        ui->txtLogFile->setEnabled(true);
        ui->txtLogInt->setEnabled(true);
        ui->cmbLogIntUnit->setEnabled(true);
    }else{
        ui->labLogF->setEnabled(false);
        ui->labLogInt->setEnabled(false);
        ui->txtLogFile->setEnabled(false);
        ui->txtLogInt->setEnabled(false);
        ui->cmbLogIntUnit->setEnabled(false);
    }
}

void HPRDmmCfg::selectSprofCfg(){
    HPRDmmSprof *Sprof = new(HPRDmmSprof);
    Sprof->setWindowTitle("Configure Start-up Profile");
    connect(Sprof, SIGNAL(accepted()), this, SLOT(getProfileList()) );
    Sprof->show();
}


void HPRDmmCfg::getProfileList(){
    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);
    QString profile = conf.value("StartUp/Profile").toString();
    ui->cmbSp->clear();
    ui->cmbSp->addItem("Default");
    ui->cmbSp->addItem("Instrument");
    conf.beginGroup("");
    foreach (const QString &group, conf.childGroups()) {
        if (group.contains("Profile:")) {
            if (ui->cmbSp->findText(group) < 0) {
                ui->cmbSp->addItem(group.mid(9,group.length()));
            }
        }
    }
    // Set combo to saved profile
    ui->cmbSp->setCurrentText(profile);
}

void HPRDmmCfg::enableR1Cfg(){
    if (ui->chkExoReal->isChecked()){
        ui->labIntRes->setEnabled(true);
        ui->spnExoR1Val->setEnabled(true);
    }else{
        ui->labIntRes->setEnabled(false);
        ui->spnExoR1Val->setEnabled(false);
    }
}


void HPRDmmCfg::readCalData(){
    // Read the instrument calibration data
    QByteArray calData;

    calData = emit getCalData();

    ui->txtCalData->append(calData);
}
