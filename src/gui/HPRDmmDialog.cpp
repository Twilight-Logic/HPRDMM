#include "HPRDmmDialog.h"
#include "HPRDmmCfg.h"
#include "HPRDmmFunc.h"
#include "ui_HPRDmmDialog.h"
#include "ui_HPRDmmCfg.h"
#include "ui_HPRDmmFunc.h"

#include <QSettings>
#include <QApplication>
#include <QCoreApplication>
#include <QTimer>
#include <QTime>
#include <QSerialPort>
#include <QtMath>
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // Dialogues
    HPRDmmCfg *DmmCfg = new HPRDmmCfg();
    HPRDmmFunc *DmmFunc = new HPRDmmFunc();
    DmmCfg->setWindowTitle("Configuration");
    DmmFunc->setWindowTitle("Function Configuration");

    // Set up serial port
    serialPort = new QSerialPort(this);
    // Set up serial port buffers
    serBuf.clear();
    lineBuf.clear();
    responseReceived = false;

    // Set up meter timer
    mtrTim = new QTimer(this);
//    mtrInterval = 250;
    mtrTim->setInterval(250);
    connect(mtrTim, SIGNAL(timeout()), this, SLOT(readMeter()) );

    // Set up status polling timer
    statTim = new QTimer(this);
//    statInterval = 3000;
    statTim->setInterval(3000);
    connect(statTim, SIGNAL(timeout()), this, SLOT(chkInstrStat()) );

    // Set up logging
    logITim = new QTimer(this);
    logITim->setInterval(1000);
    logDTim = new QTimer(this);
    logDTim->setInterval(0);
//    logCfg.Interval = 1000;
    logCfg.isEnabled = false;
    logCfg.isActive = false;
    connect(logITim, SIGNAL(timeout()), this, SLOT(recordLog()) );
    connect(logDTim, SIGNAL(timeout()), this, SLOT(stopLogging()) );

    // GUI buttons signals and slots
    connect(DmmCfg, SIGNAL(getCalData()), this, SLOT(backupCalData()) );

    connect(ui->btnPower, SIGNAL(clicked(bool)), this, SLOT(close()) );
    connect(ui->btnAAC, SIGNAL(clicked(bool)), this, SLOT(selectAAC()) );
    connect(ui->btnADC, SIGNAL(clicked(bool)), this, SLOT(selectADC()) );
    connect(ui->btnAddr, SIGNAL(clicked(bool)), this, SLOT(showGpibAddr()) );
    connect(ui->btnAzero, SIGNAL(clicked(bool)), this, SLOT(toggleAutoZero()) );
    connect(ui->btnConfig, SIGNAL(clicked(bool)), DmmCfg, SLOT(show()) );
    connect(ui->btnD3, SIGNAL(clicked(bool)), this, SLOT(selectDigits3()) );
    connect(ui->btnD4, SIGNAL(clicked(bool)), this, SLOT(selectDigits4()) );
    connect(ui->btnD5, SIGNAL(clicked(bool)), this, SLOT(selectDigits5()) );
    connect(ui->btnLocal, SIGNAL(clicked(bool)), this, SLOT(selectLocal()) );
    connect(ui->btnO2w, SIGNAL(clicked(bool)), this, SLOT(selectOhms2Wire()) );
    connect(ui->btnO4w, SIGNAL(clicked(bool)), this, SLOT(selectOhms4Wire()) );
    connect(ui->btnOex, SIGNAL(clicked(bool)), this, SLOT(selectOhmsExtend()) );
    connect(ui->btnReset, SIGNAL(clicked(bool)), this, SLOT(resetMeter()) );
    connect(ui->btnSRQ, SIGNAL(clicked(bool)), this, SLOT(requestSRQ()) );
    connect(ui->btnStrg, SIGNAL(clicked(bool)), this, SLOT(selectSglTrig()) );
    connect(ui->btnVAC, SIGNAL(clicked(bool)), this, SLOT(selectVAC()) );
    connect(ui->btnVDC, SIGNAL(clicked(bool)), this, SLOT(selectVDC()) );
    connect(ui->btnFunc, SIGNAL(clicked(bool)), DmmFunc, SLOT(show()) );
    connect(ui->btnConn, SIGNAL(clicked(bool)), this, SLOT(selectConn()) );
    connect(ui->btnLog, SIGNAL(clicked(bool)), this, SLOT(toggleLogging()) );
    connect(ui->btnDisp, SIGNAL(clicked(bool)), this, SLOT(toggleDisplay()) );
    connect(ui->btnLogCont, SIGNAL(clicked(bool)), this, SLOT(selectLogCont()) );
    connect(ui->btnMax, SIGNAL(clicked(bool)), this, SLOT(selectMax()) );
    connect(ui->btnAvg, SIGNAL(clicked(bool)), this, SLOT(selectAvg()) );
    connect(ui->btnMin, SIGNAL(clicked(bool)), this, SLOT(selectMin()) );
    connect(ui->btnReal, SIGNAL(clicked(bool)), this, SLOT(selectReal()) );
    connect(ui->btnDiod, SIGNAL(clicked(bool)), this, SLOT(selectDiode()) );
    connect(ui->btnCont, SIGNAL(clicked(bool)), this, SLOT(selectCont()) );

    // Custom function button signals and slots
    connect(ui->btnFn1, SIGNAL(clicked(bool)), this, SLOT(selectFn1()) );
    connect(ui->btnFn2, SIGNAL(clicked(bool)), this, SLOT(selectFn2()) );
    connect(ui->btnFn3, SIGNAL(clicked(bool)), this, SLOT(selectFn3()) );
    connect(ui->btnFn4, SIGNAL(clicked(bool)), this, SLOT(selectFn4()) );
    connect(ui->btnFn5, SIGNAL(clicked(bool)), this, SLOT(selectFn5()) );
    connect(ui->btnFn6, SIGNAL(clicked(bool)), this, SLOT(selectFn6()) );
    connect(ui->btnFn7, SIGNAL(clicked(bool)), this, SLOT(selectFn7()) );

    // GUI controls and dialogues signals and solts
    connect(DmmCfg, SIGNAL(accepted()), this, SLOT(loadConfig()) );
    connect(DmmFunc, SIGNAL(accepted()), this, SLOT(custFuncActivate()) );

    // Combo box signals and slots
    connect(ui->cmbRange, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRange()) );
    connect(ui->cmbTrigOpts, SIGNAL(currentIndexChanged(int)), this, SLOT(selectTrig()) );

    // Serial port read/write
    connect(this, SIGNAL(writeRequest(QByteArray)), this, SLOT(serialWrite(QByteArray)) );
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(serialRead()) );

    // Test beeper
    connect(ui->btnBeep, SIGNAL(clicked(bool)), this, SLOT(makeBeep()) );


    // Status flags
    isControllerOk = false;
    isAddrSet = false;
    isStatOk = false;
    isInstrumentConnected = false;
    isDisplayOn = true;
    isDebugModeOn = false;
    waitIsb = false;
    suProfile = 0;
    isCont = false;
    contBeep = 0;
    contThreshold = 50;

    // Set up GPIB parameters
    gpibAddr = "";
    gpibCidStr = "ARDUINO GPIB";

    // Initialise the meter
    initRanges();
    initMeter();
    setDefaultSerialCfg();
    exoR1Val = 10;

    // Pause and then load config
    QTimer::singleShot(2000, this, SLOT(loadConfig()) );

    // Read in any custom functions
    custFuncActivate();

}

Widget::~Widget()
{
    mtrTim->stop();
    serialDisconnect();
    serBuf.clear();
    lineBuf.clear();
    delete mtrTim;
    delete ui;
}



// ***** INITIALISE FUNCTIONS *****

void Widget::initMeter(){
    // Initialise the meter
    mtrVal = 0;
    mtrAvg = 0;
    mtrAvgCnt = 0;
    mtrDigits = 5;
    currentFunc = 1;

//    autoRange = true;  // Think about startup profile
    azOff = false;
    maxMode = false;
    avgMode = false;
    minMode = false;

    updateMeterStr("0.00000");
    updateFuncStr("VDC");
    funcState = "VDC";

    ui->labFunc->setText("VDC");
    ui->labPol->setText("+");
    ui->labOhm->clear();
    ui->labAzoff->hide();
    ui->labStrg->hide();
    ui->labCustFn->hide();
    ui->labLogging->hide();
    ui->labSRQ->hide();
    ui->labMAM->hide();
    ui->labGpibAddr->hide();
    ui->labCont->hide();
    ui->btnLocal->setText("READ");
    // Maybe need to do this only if local profile is set or connect signals and slots are set after this
    // (otherwise tries to set RA eben though meter is not connected yet)
    setMeterRange();
    // Maybe need to do this only if local profile is set
}


void Widget::initRanges(){
    // Init combo boxes with ranges
    QStringList Data;

    // Populate manual ranges
    rangesADC[0] << "RA" << "R-1" << "R0";
    rangesAAC[0] << "RA" << "R-1" << "R0";
    rangesOHM[0] << "RA" << "R1" << "R2" << "R3" << "R4" << "R5" << "R6" << "R7";
    rangesVDC[0] << "RA" << "R-2" << "R-1" << "R0" << "R1" << "R2";
    rangesVAC[0] << "RA" << "R-1" << "R0" << "R1" << "R2";

    rangesADC[1] << "AUTO" << "300mA" << "3A";
    rangesAAC[1] << "AUTO" << "300mA" << "3A";
    rangesOHM[1] << "AUTO" << "30Ω" << "300Ω" << "3kΩ" << "30kΩ" << "300kΩ" << "3MΩ" << "30MΩ";
    rangesVDC[1] << "AUTO" << "30mV" << "300mV" << "3V" << "30V" << "300V";
    rangesVAC[1] << "AUTO" << "300mV" << "3V" << "30V" << "300V";

    // Populate trigger mode combo
    trigOptions << "T1" << "T2" << "T3" << "T4" << "T5";
    Data << "Internal Trig" << "Single Trig" << "External Trig" << "Hold Trig" << "Fast Trig";
    ui->cmbTrigOpts->addItems(Data);
    Data.clear();
    // Display styles (Default, White, Blue-White, Blue-Yellow, Green, Yellow, LCD, LED, Plasma, Custom)
    dispStyle << "color:rgb(170,255,255);background-color:qlineargradient(spread:pad,x1:0,y1:1,x2:0,y2:0,stop:0 rgba(0,0,0,255),stop:0.05 rgba(14,8,73,255),stop:0.36 rgba(28,17,145,255),stop:0.6 rgba(126,14,81,255),stop:0.75 rgba(234,11,11,255),stop:0.79 rgba(244,70,5,255),stop:0.86 rgba(255,136,0,255),stop:0.935 rgba(255,206,55,255));";
    dispStyle << "background-color:rgb(255,255,245);";
    dispStyle << "color:rgb(240,240,240);background-color:rgb(0,0,220);";
    dispStyle << "color:rgb(255,255,127);background-color:rgb(0,0,220);";
    dispStyle << "color:rgb(250,250,0);background-color:rgb(0,100,70);";
    dispStyle << "color:rgb(0,0,0);background-color:rgb(240,250,0);";
    dispStyle << "color:rgb(70,70,70);background-color:rgb(0,240,190);";
    dispStyle << "color:rgb(255,140,0);background-color:rgb(50,30,30);";
    dispStyle << "color:rgb(200,240,255);background-color:rgb(0,0,0);";
    dispStyle << "background-color:rgb(255,255,245);";
}


// ***** END OF INITIALISE FUNCTIONS *****


// ***** APPLICATION FUNCTIONS *****

void Widget::setDefaultSerialCfg(){
    // Set the default configuration for the serial port
    serialCfg.Port.clear();
    serialCfg.BaudRate = "9600";
    serialCfg.Parity = "N";
    serialCfg.DataBits = "8";
    serialCfg.StopBits = "1";
    serialCfg.FlowControl = "None";
}


void Widget::updateMeterStr(QString string){
    // Update the value displayed on the meter
    double reading = string.toDouble();
    bool flg = false;

    // Determine update mode
    if (mtrVal == 0) {
        mtrVal = reading;
        flg = true;
    }else if (maxMode){
        if (reading > mtrVal){
            mtrVal = reading;
            flg = true;
        }
    }else if (avgMode) {
/* **** not working! *****
        if (mtrAvg = 0) {
            mtrVal = reading;
            mtrAvg = reading;
            mtrAvgCnt++;
        }else{
            mtrVal = reading;
//            mtrAvg = mtrAvg + ((mtrAvg - reading)/mtrAvgCnt);
//            mtrAvgCnt++;

            mtrAvg = reading;

//            mtrVal = (reading + mtrVal) / 2;
            string = QString("%10000000").arg(mtrAvg).left(7);
            flg = true;
        }
*/
    }else if (minMode) {
        if (reading < mtrVal){
            mtrVal = reading;
            flg = true;
        }
    }else{
        flg = true;
    }
    // Update the display
    if (flg) {
        switch(mtrDigits){
        case 3: ui->lcdMeter->display(string.left(5)+"  ");
            break;
        case 4: ui->lcdMeter->display(string.left(6)+" ");
            break;
        case 5: ui->lcdMeter->display(string);
            break;
        default:
            ui->lcdMeter->display("0.00000");

        }
    }
}


void Widget::updateFuncStr(QString string){
    // Update the text displayed in the fuction indicator
    ui->labFunc->setText(string);
}


void Widget::loadConfig(){
    // Load the configuration from file
    QByteArray ba;
    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);
    int logInt;
    int dS;
    bool exoEn;


    // This value must exist to read the config
    if (conf.value("GPIB/GpibAddr").toInt() > 0) {

        // If a serial port is already connected then shut it down
        serialDisconnect();

        // Serial port config
        gpibAddr = conf.value("GPIB/GpibAddr", "2").toString();
        gpibCidStr = conf.value("GPIB/GpibControllerIdStr", "").toString();
        serialCfg.Port = conf.value("Serial/Port", "").toString();
        serialCfg.BaudRate = conf.value("Serial/BaudRate", "9600").toString();
        serialCfg.Parity = conf.value("Serial/Parity", "N").toString();
        serialCfg.DataBits = conf.value("Serial/DataBits", "8").toString();
        serialCfg.StopBits = conf.value("Serial/StopBits", "1").toString();
        serialCfg.FlowControl = conf.value("Serial/FlowControl", "None").toString();

        serialPort->setPortName(serialCfg.Port);
        setBaudRate(serialCfg.BaudRate);
        setParity(serialCfg.Parity);
        setDataBits(serialCfg.DataBits);
        setStopBits(serialCfg.StopBits);
        setFlowControl(serialCfg.FlowControl);

        // Log config
        logCfg.isEnabled = conf.value("Logging/Enabled").toBool();
        logCfg.FileName = conf.value("Logging/File").toString();
        logInt = conf.value("Logging/Interval", 1000).toInt();
        logCfg.IntUnit = conf.value("Logging/IntUnit", 0).toInt();
        switch (logCfg.IntUnit) {
        case 0: logITim->setInterval(logInt);
            break;
        case 1: logITim->setInterval(logInt * 1000);
            break;
        case 2: logITim->setInterval(logInt * 60000);
            break;
        default:
            logITim->setInterval(logInt);
        }
        if (logCfg.isEnabled && logCfg.FileName != "") {
            ui->btnLog->setEnabled(true);
            ui->btnLogCont->setEnabled(true);
            ui->spnHrs->setEnabled(true);
            ui->spnMin->setEnabled(true);
            ui->spnSec->setEnabled(true);
            ui->spnHrs->setStyleSheet("background-color: rgb(255, 255, 245);");
            ui->spnMin->setStyleSheet("background-color: rgb(255, 255, 245);");
            ui->spnSec->setStyleSheet("background-color: rgb(255, 255, 245);");
        }else{
            ui->btnLog->setEnabled(false);
            ui->btnLogCont->setEnabled(false);
            ui->spnHrs->setEnabled(false);
            ui->spnMin->setEnabled(false);
            ui->spnSec->setEnabled(false);
        }

        // Debug Mode
        isDebugModeOn = conf.value("Advanced/DebugMode", false).toBool();
        if (isDebugModeOn) {
            this->setFixedHeight(543 );
            ui->textBrowser->append("Configuration loaded.");
        }

        // Start-Up profile (0=default, 1=instrument, 2=custom
        if (conf.value("Startup/Profile","Default").toString() == "Default"){
            suProfile = 0;
        }else if (conf.value("Startup/Profile","Default").toString() == "Instrument"){
            suProfile = 1;
        }else{
            suProfile = 2;
            readSuProfile();
        }

        // Set the display style - idx: 1-8 predefined, 9=custom
        dS = conf.value("StartUp/DisplayStyle",0).toInt();
        if (dS == 9) {
        ui->frmLCD->setStyleSheet(conf.value("Startup/DisplayCustom","background-color: rgb(255, 255, 245);").toString());
            if (isDebugModeOn) ui->textBrowser->append("Display style: custom[" + conf.value("StartUp/DisplayCustom","background-color: rgb(255, 255, 245);").toString()+"]");
        }else{
            ui->frmLCD->setStyleSheet(dispStyle[dS]);
            if (isDebugModeOn) ui->textBrowser->append("Display style: " + QString::number(dS));
        }

        // Extended ohms calibration value
        exoEn = conf.value("Advanced/exoRealValue", 0).toBool();
        if (exoEn) exoR1Val = conf.value("Advanced/exoR1Value", 10).toDouble();

        // Continuity mode
        contBeep = conf.value("Advanced/ContBeepEnable", false).toBool();
        contThreshold = conf.value("Advanced/ContThreshold", 50).toInt();

        // Connect serial port with new settings
        serialConnect();

    }else{
        // Read config failed
        emit ui->btnConfig->click();    // Load configuration dialogue
    }

}


void Widget::readSuProfile(){
    // Read and apply the selected start-up profile
    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);
    QString prof;

    int num;
    bool flg;

    prof = "Profile: " + conf.value("StartUp/Profile", "").toString();

    num = conf.value(prof+"/Function", 1).toInt();
    switch (num) {
    case 1: selectVDC();
        break;
    case 2: selectVAC();
        break;
    case 3: selectOhms2Wire();
        break;
    case 4: selectOhms4Wire();
        break;
    case 5: selectADC();
        break;
    case 6: selectAAC();
        break;
    case 7: selectOhmsExtend();
        break;
    default:
        selectVDC();
    }

    // Display digits
    num = conf.value(prof+"/DisplayDigits", 5).toInt();
    switch (num) {
    case 3: selectDigits3();
        break;
    case 4: selectDigits4();
        break;
    case 5: selectDigits5();
        break;
    default:
        selectDigits5();
    }

    num = conf.value(prof+"/MeterRange", 0).toInt();
    if (num > 0 && num < 8) {
        ui->cmbRange->setCurrentIndex(num);
    }
    selectRange();

    num = conf.value(prof+"/TriggerMode", 0).toInt();
    if (num > 0 && num < 5) {
        ui->cmbTrigOpts->setCurrentIndex(num);
    }

    // Normal, min, max average
//    ui->cmbDisplayMode->setCurrentIndex(conf.value(prof+"/DisplayMode", 0).toInt());

    flg = conf.value(prof+"/AutoZeroOff", false).toBool();
    if (flg) toggleAutoZero();

    flg = conf.value(prof+"/MeterDisplayOff", false).toBool();
    if (flg) {
        isDisplayOn = true;
        toggleDisplay();
    }

}


void Widget::serialDisconnect() {
    // Disconnect the serial port
    if (serialPort->isOpen()) {
        serialPort->write("++loc\r\n");
        serialPort->close();
        serBuf.clear();
        lineBuf.clear();
        if (isDebugModeOn) ui->textBrowser->append("Serial port disconnected.");
        if (mtrTim->isActive()) mtrTim->stop();
        if (statTim->isActive()) statTim->stop();
        if (logCfg.isActive) stopLogging();
        ui->lcdMeter->display("0.00000");
//        ui->btnConn->setText("CONN");
        // Reset indicator lights
        ui->pbSer->setValue(0);
        ui->pbGpib->setValue(0);
        ui->pbInstr->setValue(0);
        // Reset flags
        isControllerOk = false;
        isAddrSet = false;
        isStatOk = false;
        isInstrumentConnected = false;
        waitIsb = false;
        initMeter();
    }
}


void Widget::serialConnect(){
    // Attempt to connect to the serial port and proceed to identifiying the controller
//    serialPort->reset();
//    serialPort->clear();
    if (serialPort->open(QIODevice::ReadWrite)){
        // Set panel light
        ui->pbSer->setValue(1);
        if (isDebugModeOn) ui->textBrowser->append("Serial port connected: " + serialPort->portName());
        // Connect to GPIB device
        QTimer::singleShot(3000, this, SLOT(ctrlrConnect()) );
    }else{
        // Connect failed
        if (serialPort->isOpen()) serialPort->close();
        if (isDebugModeOn) ui->textBrowser->append("Failed to open serial port: " + serialPort->portName());
    }
}


void Widget::ctrlrConnect() {
    QByteArray line;
    // Confirm that the controller is detected (by sending ++ver command and checking the response)
    int rtrcnt = 0;
    // Max 3 reties (Arduino controller sometimes fails to return ver)
    while (rtrcnt<3){
        ui->textBrowser->append("++ver\r");
        // Write the '++ver' command
        serialWriteWait("++ver\r");
        if (isControllerOk){
            rtrcnt = 3;
        }else{
            rtrcnt++;
            if (isDebugModeOn) ui->textBrowser->append("Attempt"+QString::number(rtrcnt)+": Unable to identify GPIB controller.");
        }
    }
    if (isControllerOk) {
        setGpibAddr();
    }else{
        serialDisconnect();
    }
}


void Widget::setGpibAddr() {
    // Set the GPIB address
    QByteArray ba;

    // remove any remaining characters from buffer (seems to work better than port.clear);
    delay(200);
    ba = serialPort->readAll();
    ba.clear();

    // Send address command
    ba.append("++addr " + gpibAddr + "\r");
    ui->textBrowser->append(ba);
    emit serialWrite(ba);
    // Read back the address to verify
    serialWriteWait("++addr\r");
    if (isAddrSet) {
        ui->labGpibAddr->setText("GPIB Address: " + gpibAddr);
        if (isDebugModeOn) ui->textBrowser->append("Instrument GPIB address set: " + gpibAddr);
        verifyInstrConnect();
    }else{
        if (isDebugModeOn) ui->textBrowser->append("Could not set the instrument GPIB address.");
        serialDisconnect();
    }
}


void Widget::verifyInstrConnect() {
    // Verify connecton to instrument (by checking response to the 'S' command)
    QByteArray ba;
//    isStatOk = false;

    emit serialWrite("S\r");
    if (isDebugModeOn) ui->textBrowser->append("Command sent: S");
    delay(200);
    serialWriteWait("++read eoi\r");

    if (isStatOk) {

        isInstrumentConnected = true;
        if (isDebugModeOn) ui->textBrowser->append("Instrument is connected.");

        ui->btnConn->setText("DISC");
        ui->pbInstr->setValue(1);

//        emit serialWrite("F1RA\r");
//        serialWriteWait("F1RA\r");
//        if (isDebugModeOn) ui->textBrowser->append("Instrument set to VDC and autorange modes.");
//        mtrTim->setInterval(mtrInterval);
//        mtrTim->start();

//        chkInstrStat();
//        statTim->setInterval(statInterval);

        switch (suProfile) {
        case 0: statTim->start();
            break;
        case 1: chkInstrStat();
            statTim->start();
            break;
        case 2: readSuProfile();
            statTim->start();
            break;
        }

    }else{
        if (isDebugModeOn) ui->textBrowser->append("Could not get a response from the instrument.");
    }

}


void Widget::readMeter() {
    // Read the measured value
    emit serialWrite("++read eoi\r");
}


void Widget::serialWrite(const QByteArray &data) {
    // Write data to the serial port
    serialPort->write(data);
}


void Widget::serialWriteWait (QByteArray data){
    // Wait for response wrapper for writing data to the serial port
    responseReceived = false;
//    ui->textBrowser->append("Command: "+data);
    emit serialWrite(data);
    waitForResponse();
}


void Widget::sendCMD(QString cmd) {
    // Send a pre-formatted command to the serial port
    QByteArray ba;
    bool tim = false;
    ba.append(cmd);

    if (isInstrumentConnected){
        if (mtrTim->isActive()) {
            tim = true;
            mtrTim->stop();
            serialPort->flush();
            if (isDebugModeOn) ui->textBrowser->append("Meter timer stopped");
        }

        delay(500);
    // Send a ++loc to get the attention of the controller
    //    emit serialWrite("++loc\x0D\x0D");
        // Send the command
        emit serialWrite(ba);
        if (isDebugModeOn) ui->textBrowser->append("Command sent: " + cmd);
        if (tim) {
            mtrTim->start();
            if (isDebugModeOn) ui->textBrowser->append("Meter timer started");
        }

    }
}


void Widget::waitForResponse() {
    // Wait for response from serial port
    // Waits until response received or timeout
    const int timeout = 3000;
    int responseTimeout = 0;
    int wait = 20;
    while (responseReceived == false && responseTimeout < timeout){
        delay(wait);
        responseTimeout += wait;
    }
}


void Widget::delay(int msec){
    // Wait loop
    QTime intTime = QTime::currentTime().addMSecs( msec );
    while( QTime::currentTime() < intTime ) {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}


void Widget::serialRead() {
    // Read serial port when data ready
    QByteArray lb;
    if (waitIsb == false){
        if (serialPort->canReadLine()) {
            lb = serialPort->readLine();
            if (lb.at(0) != 0x13 || lb.at(0) != 0x10) {
                processLine(lb);
            }
        }
//        serBuf.clear();
    }
}


/*
void Widget::readLineOfData() {
    // Process data received via the serial port into lines
    int dsize = serBuf.length();
    int i = 0;
    char cr = 13; // Carriage return
    char lf = 10; // Line feed

    // Read data bytes (to CR or LF)
    while(i<dsize) {
        // The CR/LF indicates the end of a line
        if (serBuf.at(i) == cr || serBuf.at(i) == lf) {
            // Process the data into lines
            if (lineBuf.length() > 0) {    // Ignore blank lines
                responseReceived = true;

                processLine(lineBuf);
                // Reset the line buffer
                lineBuf.clear();
            }
        }
        // Append characters to line, ignor CR and LF
        if (serBuf.at(i) != cr && serBuf.at(i) != lf){  // Ignore CR and LF
            lineBuf.append(serBuf.at(i));
        }
        i++;
    }
}
*/


void Widget::processLine(QByteArray line){
    // Process each line received and take appropriate action to the response
    QString response;
    QMessageBox msgBox;
//    QString mtrRange;
//    QString mtrPol;
    int addr = 0;
    char stat = 0;

    // Place received line into variable
    response.append(line);

    // Response to ++ver
    if (!isControllerOk) {
        if (line.contains(gpibCidStr.toLatin1())){
            ui->pbGpib->setValue(1);
            isControllerOk = true;
            responseReceived = true;
            if (isDebugModeOn) ui->textBrowser->append("GPIB controller: "+line);
        }
    }else if (!isAddrSet){
        addr = line.left(1).toInt();
        if (addr > 0) { // Must start with a numeric character
            addr = line.left(2).toInt();
            if (addr == gpibAddr.toInt()) {
                isAddrSet = true;
                responseReceived = true;
            }
        }
    }else if (!isStatOk){
        stat = line.at(0);
//        ui->textBrowser->append("Response to S: "+line);
        if (stat == 0x30 || stat == 0x31){
            if (isDebugModeOn) {
                if (stat == 0x30) ui->textBrowser->append("Instrument is using rear connectors.");
                if (stat == 0x31) ui->textBrowser->append("Instrument is using front connectors.");
            }
            // Set connected status
            isStatOk = true;
            responseReceived = true;
        }
    }

    if (isInstrumentConnected) {
        // Response beggining + or - indicates reading polarity
        if (response[0] == '+' || response[0] == '-'){
            if (ui->labOhm->text() == "ExΩ"){
                updateMeterStr(calcExOhm(response));
                mtrReading = calcExOhm(response) + response.mid(8,3);
            }else{
                updateMeterStr(response.mid(1,7));
                mtrReading = response.mid(1,10);
            }
            mtrPol = response.left(1);
            ui->labPol->setText(response.left(1));
            mtrXply = response.mid(9,2);
            // Exponent can be used to indicate m, k or M
            if (mtrXply == "-3"){
                updateFuncStr("m"+funcState);
            }else if (mtrXply == "+3"){
                updateFuncStr("k"+funcState);
            }else if (mtrXply == "+6"){
                updateFuncStr("M"+funcState);
            }else if (mtrXply == "+9"){
                updateFuncStr("M"+funcState);
                updateMeterStr("-------");
            }else{
                updateFuncStr(funcState);
            }

            // Continuity beeper
            if (funcState == "CONT"){
                if (mtrReading.toFloat() < contThreshold ){
                    if (contBeep) makeBeep();
                }
            }
        }
    }

    if (isDebugModeOn) ui->textBrowser->append("DATA: " + line);


}


QString  Widget::calcExOhm(QString reading){
    // Calculates the extended ohms resistance value from the indicated reading
    double r1 = exoR1Val;
    double r2 = reading.mid(1,7).toDouble();
    double result = 0;
    double divd = r1 - r2;
    QString resultStr;

    // Prevent division by zero issue (should not ocurr in practice)
    if (divd == 0) divd = 1e+10;
    // Calculate value of resistor
    result = (r1 * r2)/divd;
    // Return formatted result
    resultStr = QString("%10000000").arg(result).left(7);
    return resultStr.left(7);
}


void Widget::initLogging(){
    // Initialise the logging function
    logFile.setFileName(logCfg.FileName);
// Note - streaming text may not be required!
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)){
        logCfg.isActive = true;
        // Start logging timer
//        logITim->setInterval(logCfg.Interval);
        logITim->start();
    }else{
        // Do something to indicate an error
    }

}


void Widget::stopLogging(){
    // Stop the logging process
    logITim->stop();
    logFile.close();
    if (logDTim->isActive()) {
        logDTim->stop();
        ui->spnHrs->setEnabled(true);
        ui->spnMin->setEnabled(true);
        ui->spnSec->setEnabled(true);
    }
    logCfg.isActive = false;
    ui->labLogging->hide();
}


void Widget::recordLog() {
    QByteArray line = "";
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    line.append(timestamp + ",");
    line.append(mtrPol.toLatin1() + mtrReading.toLatin1() + mtrXply.toLatin1() + ",");
    line.append(funcState + "\n");
    const char *strng = line.data();
    logFile.write(strng);
//    logStrm << strng;

}


void Widget::setMeterRange(){
    // Set the meter range for a given function and default to AUTO range
    currentRange[0].clear();
    currentRange[1].clear();

    if (funcState == "CUS" || ui->labOhm->text() == "ExΩ" ) {
        ui->cmbRange->clear();
        ui->cmbRange->addItem("AUTO");
        ui->cmbRange->setCurrentIndex(0);
    }else if (funcState == "DIODE") {
        ui->cmbRange->clear();
        ui->cmbRange->addItem("DIODE");
    }else if (funcState == "CONT") {
        ui->cmbRange->clear();
        ui->cmbRange->addItem("CONT");
    }else{

//    if (funcState != "CUS" && funcState != "DIODE" && funcState != "CONT" && ui->labOhm->text() != "ExΩ") {
        // Set up new clear lists
        currentRange[0].clear();
        currentRange[1].clear();
        ui->cmbRange->clear();
        // Copy appropriate range
        if (funcState == "AAC") {
            ui->cmbRange->addItems(rangesAAC[1]);
            currentRange[0] = rangesAAC[0];
        }else if (funcState == "ADC") {
            currentRange[0] = rangesADC[0];
            ui->cmbRange->addItems(rangesADC[1]);
        }else if (funcState == "OHM") {
            currentRange[0] = rangesOHM[0];
            ui->cmbRange->addItems(rangesOHM[1]);
        }else if (funcState == "VDC") {
            currentRange[0] = rangesVDC[0];
            ui->cmbRange->addItems(rangesVDC[1]);
        }else if (funcState == "VAC") {
            currentRange[0] = rangesVAC[0];
            ui->cmbRange->addItems(rangesVAC[1]);
        }
        ui->cmbRange->setCurrentIndex(0);

/*
//        sendCMD(currentRange[0].at(0)+"\r");
    }else if (funcState == "DIODE"){
        ui->cmbRange->clear();
        ui->cmbRange->addItem("DIODE");
    }else{
        ui->cmbRange->clear();
        ui->cmbRange->addItem("AUTO");
        ui->cmbRange->setCurrentIndex(0);
//        sendCMD("RA\r");
    }
*/
    }
}


void Widget::chkInstrStat() {
    // Send commands to check the instrument status ('B' command followed by ++read eoi)
    char statData[5] = {0};
    unsigned char c;

    serialPort->flush();

    // Send the 'B' command
    waitIsb = true;
    emit serialWrite("B\r");
    if (isDebugModeOn) ui->textBrowser->append("Command sent: B");
    delay(200);
    // Read the result
    emit serialWrite("++read eoi\r");
    delay(200);
    // Collect 5 bytes
    serialPort->read(statData, 5);
    delay(200);
    // Give control back to instrument
    emit serialWrite("++loc\r");
    // Must have some data to proceed
    if  (statData[0] != '\0'){
        // Has anything changed?
        if (strcmp(iStat, statData) == 0) {
            ui->textBrowser->append("Status unchanged.");
        }else{
            // If so then update status
            ui->textBrowser->append("Instrument status has changed.");
            strncpy(iStat, statData, 5);
            if (isDebugModeOn) {
                ui->textBrowser->append("Status data received.");
               // Display bytes in debug window
                for (int i=0; i<5; i++) {
                    c = statData[i];
                    ui->textBrowser->append("Byte" + QString::number(i+1) + ": " + QString("%1").arg(c, 8, 2, QChar('0')));
                }
            }
            updInstrStat(statData);
        }
    }
}


void Widget::updInstrStat(char data[5]){
    // Update the program status from the instrument status data
    quint8 byte;
    uint r = 0;

    // >>>>> Byte 1 >>>>> Function, range and number of digits displayed
    byte = data[0];
    r = byte & 3;
    switch (r) {
    case 1: actionDigits5();
        if (isDebugModeOn) ui->textBrowser->append("5.5 digit mode selected");
        break;
    case 2: actionDigits4();
        if (isDebugModeOn) ui->textBrowser->append("4.5 digit mode selected");
        break;
    case 3: actionDigits3();
        if (isDebugModeOn) ui->textBrowser->append("3.5 digit mode selected");
        break;
    }

    r = (byte >> 5) & 7;
    switch (r) {
    case 1: actionVDC();
        if (isDebugModeOn) ui->textBrowser->append("DC volts mode selected");
        break;
    case 2: actionVAC();
        if (isDebugModeOn) ui->textBrowser->append("AC volts mode selected");
        break;
    case 3: actionOhms2Wire();
        if (isDebugModeOn) ui->textBrowser->append("2W Ohms mode selected");
        break;
    case 4: actionOhms4Wire();
        if (isDebugModeOn) ui->textBrowser->append("4W Ohms mode selected");
        break;
    case 5: actionADC();
        if (isDebugModeOn) ui->textBrowser->append("DC current mode selected");
        break;
    case 6: actionAAC();
        if (isDebugModeOn) ui->textBrowser->append("AC current mode selected");
        break;
//    case 7:
//        if (isDebugModeOn) ui->textBrowser->append("Extended Ohms mode selected");
//        break;
    }

    // >>>>> Autorange (byte 2, bit 2 >>>>>
    r = data[1];
    if (r & 2) {
        ui->cmbRange->setCurrentIndex(0);
        actionRange();
        if (isDebugModeOn) ui->textBrowser->append("Autorange is on");
    }else{
        if (isDebugModeOn) ui->textBrowser->append("Autorange is off");
        // >>>>> Byte 1 bits 3,4,5 >>>>> Manual range
        byte = data[0];
        r = (byte >> 2) & 7;
        ui->cmbRange->setCurrentIndex(r);
        actionRange();
        if (isDebugModeOn) {
            switch (r) {
            case 0: ui->textBrowser->append("No manual range selected");
                break;
            case 1: ui->textBrowser->append("Range 1 selected");
                break;
            case 2: ui->textBrowser->append("Range 2 selected");
                break;
            case 3: ui->textBrowser->append("Range 3 selected");
                break;
            case 4: ui->textBrowser->append("Range 4 selected");
                break;
            case 5: ui->textBrowser->append("Range 5 selected");
                break;
            case 6: ui->textBrowser->append("Range 6 selected");
                break;
            case 7: ui->textBrowser->append("Range 7 selected");
                break;
            }
        }
    }


    // >>>>> Rest of byte 2 >>>>> Instrument status

    r = data[1];
    if (r & 1) {
        ui->cmbTrigOpts->setCurrentIndex(0);
        actionSglTrig(0);
        if (isDebugModeOn) ui->textBrowser->append("Internal trigger enabled");
    }
    if (r & 4) {
        actionAutoZeroOff(false);
        if (isDebugModeOn) ui->textBrowser->append("Autozero enabled");
    }else{
        actionAutoZeroOff(true);
        if (isDebugModeOn) ui->textBrowser->append("Autozero disabled");
    }
    if (isDebugModeOn){
        if (r & 8) ui->textBrowser->append("Set for 50Hz operation");
        if (r & 16) ui->textBrowser->append("Front connectors enabled");
        if (r & 32) ui->textBrowser->append("Calibration mode enabled");
    }
    if (r & 64) {
        ui->cmbTrigOpts->setCurrentIndex(1);
        actionSglTrig(1);
        if (isDebugModeOn) ui->textBrowser->append("External trigger enabled");
    }
    if (r & 128) ui->textBrowser->append("Always zero (byte 2 bit 7)");


    if (isDebugModeOn) {

        // >>>>> Byte 3 >>>>> SRQ mask

        r = data[2];
        if (r & 1) ui->textBrowser->append("SRQ - data ready");
        if (r & 2) ui->textBrowser->append("Always zero (byte 3 bit 1)");
        if (r & 4) ui->textBrowser->append("SRQ - syntax error");
        if (r & 8) ui->textBrowser->append("SRQ - Internal error");
        if (r & 16) ui->textBrowser->append("SRQ - front pannel button pressed");
        if (r & 32) ui->textBrowser->append("Calibration error");
        if (r & 64) ui->textBrowser->append("Power-on SRQ enabled");
        if (r & 128) ui->textBrowser->append("Always zero (byte 3 bit 7");

        // >>>>> Byte 4 >>>>> Internal error information

        r = data[3];
        if (r & 1) ui->textBrowser->append("Calibration - incorrect checksums");
        if (r & 2) ui->textBrowser->append("CPU RAM self-test failed");
        if (r & 4) ui->textBrowser->append("Control RAM self-test failed");
        if (r & 8) ui->textBrowser->append("A/D slope error detected");
        if (r & 16) ui->textBrowser->append("A/D internal self-test failed");
        if (r & 32) ui->textBrowser->append("A/D link [U403-U462] failure");
        if (r & 64) ui->textBrowser->append("Always zero (byte 4 bit 6)");
        if (r & 128) ui->textBrowser->append("Always zero (byte 4 bit 7) ");

        // >>>>> Byte 5 >>>>>

        r = data[4];  // internal DAC value
        ui->textBrowser->append("Internal DAC value:" + QString::number(r));
    }
}


// ***** END OF APPLICATION FUNCTIONS *****


// ***** BUTTON HANDLERS *****

// Close program
void Widget::closeHPRDMM(){
//    ui->labTest->setText("LINE");
}


// Select Current AC
void Widget::selectAAC(){
//    initMeter();
    sendCMD("F6RA\r");
    actionAAC();
}


// Select Current DC
void Widget::selectADC(){
//    initMeter();
    sendCMD("F5RA\r");
    actionADC();
}


// Get instrument address
void Widget::showGpibAddr(){
    ui->labGpibAddr->show();
    sendCMD("D2HPIB ADRS. "+gpibAddr+"\r");
    delay(3000);
    ui->labGpibAddr->hide();
    sendCMD("D1\r");
}


// Toggle auto zero
void Widget::toggleAutoZero(){
    if (azOff) {
        sendCMD("Z1\r");
        actionAutoZeroOff(false);
    }else{
        sendCMD("Z0\r");
        actionAutoZeroOff(true);
    }
}


// Select 3 digit display
void Widget::selectDigits3(){
    sendCMD("N3\r");
    actionDigits3();
}


// Select 4 digit display
void Widget::selectDigits4(){
    sendCMD("N4\r");
    actionDigits4();
}


// Select 5 digit display
void Widget::selectDigits5(){
    sendCMD("N5\r");
    actionDigits5();
}


// Change selection in manual range selection mode
void Widget::selectRange(){
    // Set the range indicated by the combo box
    int r = ui->cmbRange->currentIndex();
    if (r > -1 && r < currentRange[0].count() ) {
        sendCMD(currentRange[0].at(r)+"\r");
        actionRange();
    }
}


// Select Trigger Mode
void Widget::selectTrig(){
    int r = ui->cmbTrigOpts->currentIndex();
    if (r > -1 && r < trigOptions.count() ){
        sendCMD(trigOptions[r]+"\r");
        actionSglTrig(r);
    }
}


/*
// Select INT trig
void Widget::selectIntTrig(){

}
*/


// Select Local (disconnect meter and operate from front pannel)
void Widget::selectLocal(){
    QMessageBox msgBox;
    if (isInstrumentConnected){
        if (ui->btnLocal->text() == "READ") {
            statTim->stop();
            waitIsb = false;
            if (isDebugModeOn) ui->textBrowser->append("Status polling stopped.");
            mtrTim->start();
            if (isDebugModeOn) ui->textBrowser->append("Meter reading started.");
            ui->btnLocal->setText("LOCAL");
            sendCMD("F"+QString::number(currentFunc));
        }else{
            sendCMD("++loc\x0D");
            mtrTim->stop();
            waitIsb = true;
            if (isDebugModeOn) ui->textBrowser->append("Meter reading stopped.");
            statTim->start();
            if (isDebugModeOn) ui->textBrowser->append("Status polling started.");
            ui->btnLocal->setText("READ");
        }
    }else{
        msgBox.setText("Instrument is not connected.");
        msgBox.exec();
    }
}


// Select Ohms 2 wire measurement
void Widget::selectOhms2Wire(){
    sendCMD("F3RA\r");
    actionOhms2Wire();
}


// Select Ohms 4 wire measurement
void Widget::selectOhms4Wire(){
    sendCMD("F4RA\r");
    actionOhms4Wire();
}


// Select Ohms extended measurement
void Widget::selectOhmsExtend(){
    sendCMD("F7RA\r");
    actionOhmsExtend();
}


// Reset multimeter
void Widget::resetMeter(){
    sendCMD("++clr\x0D");
    initMeter();
}


// Request SRQ
void Widget::requestSRQ(){

}


// Select SGL trig
void Widget::selectSglTrig(){
    sendCMD("++trg\r");
}


// Select AC voltage measurement
void Widget::selectVAC(){
    sendCMD("F2RA\r");
    actionVAC();
}


// Select DC voltage measurement
void Widget::selectVDC(){
//    initMeter();
    funcState = "VDC";
    updateFuncStr(funcState);
//    ui->labRng->hide();
//    ui->labRval->hide();
    sendCMD("F1RA\r");
    setMeterRange();
    actionVDC();
}


void Widget::selectConn(){
    // Connects/disconnects the instrument
    if (isInstrumentConnected) {
        // Send 'Local'/'reset' to instrument
        serialDisconnect();
        ui->btnConn->setText("CONN");
//        serialPort->close();
//        mtrTim->stop();
//        ui->lcdMeter->display("0.00000");
//        ui->btnConn->setText("CONN");
//        ui->pbSer->setValue(0);
//        ui->pbGpib->setValue(0);
//        ui->pbInstr->setValue(0);
//        isInstrumentConnected = false;
//        isControllerOk = false;
    }else{
        serialConnect();
        if (isInstrumentConnected) ui->btnConn->setText("DISC");
    }
}


void Widget::toggleLogging(){
    // Toggle logging on/off
    int dur = (ui->spnHrs->value()*3600)+(ui->spnMin->value()*60)+ui->spnSec->value();
    if (logCfg.isActive) {
        stopLogging();
        ui->labLogging->hide();
        if (logDTim->isActive()) logDTim->stop();
    }else{
        if (dur>0) {
            logDTim->setInterval(dur*1000);
            logDTim->start();
            ui->spnHrs->setEnabled(false);
            ui->spnMin->setEnabled(false);
            ui->spnSec->setEnabled(false);
        }
        initLogging();
        ui->labLogging->show();
    }
}


// Toggle instument display on/off
void Widget::toggleDisplay(){
    if (isDisplayOn){
        sendCMD("D2DISPLAY OFF\r");
        delay(2000);
        sendCMD("D3\r");
        isDisplayOn = false;
    }else{
        sendCMD("D3DISPLAY ON\r");
        delay(2000);
        sendCMD("D1\r");
        isDisplayOn = true;
    }
}


void Widget::selectLogCont(){
    // Select continuous logging (set timer to zero)
    ui->spnHrs->setValue(0);
    ui->spnMin->setValue(0);
    ui->spnSec->setValue(0);
}


void Widget::selectMax(){
    // Select maximum reading display
    ui->labMAM->setText("MAX");
    ui->labMAM->show();
    maxMode = true;
    avgMode = false;
    minMode = false;
}


void Widget::selectAvg() {
    // Select average reading display
    ui->labMAM->setText("AVG");
    ui->labMAM->show();
    mtrAvg = 0;
    mtrAvgCnt = 0;
    maxMode = false;
    avgMode = true;
    minMode = false;
}


void Widget::selectMin() {
    // Select minimum reading display
    ui->labMAM->setText("MIN");
    ui->labMAM->show();
    maxMode = false;
    avgMode = false;
    minMode = true;
}


void Widget::selectReal() {
    // Select minimum reading display
    ui->labMAM->setText("MAM");
    ui->labMAM->hide();
    maxMode = false;
    avgMode = false;
    minMode = false;
}


void Widget::selectDiode() {
    // Select diode test mode
    sendCMD("F3R3\r");
    funcState = "DIODE";
    currentFunc = 3;
    updateFuncStr(funcState);
    ui->labOhm->clear();
    ui->labRval->clear();
//    actionDigits3();
    setMeterRange();
}


void Widget::selectCont() {
    // Select continuity test mode
    sendCMD("F3R2\r");
    funcState = "CONT";
    currentFunc = 3;
    ui->labOhm->setText("2wΩ");
    ui->labRval->clear();
    updateFuncStr(funcState);
    setMeterRange();

}


// Select function 1
void Widget::selectFn1(){
    runCustomCMD(1);
}


// Select function 2
void Widget::selectFn2(){
    runCustomCMD(2);
}


// Select function 3
void Widget::selectFn3(){
    runCustomCMD(3);
}


// Select function 4
void Widget::selectFn4(){
    runCustomCMD(4);
}

//Select function 5
void Widget::selectFn5(){
    runCustomCMD(5);
}


// Select function 6
void Widget::selectFn6(){
    runCustomCMD(6);
}

// Select function 7
void Widget::selectFn7(){
    runCustomCMD(7);
}


// Select function 8
void Widget::selectFn8(){
    runCustomCMD(8);
}


void Widget::runCustomCMD(int func) {
    // Run a custom command
    initMeter();
    funcState = "CUS";
    updateFuncStr(funcState);
    if (custFuncs[func-1][0] != 0) {
        ui->labCustFn->setText("CUSTOM "+custFuncs[func-1][0]);
    }else{
        ui->labCustFn->setText("CUSTOM FN"+QString::number(func));
    }
    ui->labCustFn->show();
    sendCMD(custFuncs[func-1][1]+"\r");
}


/*
QString Widget::packageOutputCmd(QString cmd){
    // Package commands into an 'OUTPUT' command string
    cmd = "OUTPUT 7" + gpibAddr + ";" + '"' + cmd + '"' + "\r";
    return cmd;
}
*/

//##### BUTTON ACTIONS #####

void Widget::actionClose(){
}


void Widget::actionAAC(){
    funcState = "AAC";
    currentFunc = 6;
    updateFuncStr(funcState);
    ui->labOhm->setText("");
    setMeterRange();
}


void Widget::actionADC(){
    funcState = ("ADC");
    currentFunc = 5;
    updateFuncStr(funcState);
    ui->labOhm->setText("");
    setMeterRange();
}


void Widget::actionAutoZeroOff(bool stat){
    azOff = stat;
    if (azOff) {
        ui->labAzoff->show();
    }else{
        ui->labAzoff->hide();;
    }
}


void Widget::actionDigits3(){
    mtrDigits = 3;
    updateMeterStr("0.000  ");
}


void Widget::actionDigits4(){
    mtrDigits = 4;
    updateMeterStr("0.0000 ");
}


void Widget::actionDigits5(){
    mtrDigits = 5;
    updateMeterStr("0.00000");
}


void Widget::actionOhms2Wire(){
    funcState = "OHM";
    currentFunc = 3;
    updateFuncStr(funcState);
    ui->labOhm->setText("2wΩ");
    setMeterRange();
}


void Widget::actionOhms4Wire(){
    funcState = "OHM";
    currentFunc = 4;
    updateFuncStr(funcState);
    ui->labOhm->setText("4wΩ");
    setMeterRange();
}


void Widget::actionOhmsExtend(){
    funcState = "OHM";
    currentFunc = 7;
    updateFuncStr(funcState);
    ui->labOhm->setText("ExΩ");
    setMeterRange();
}

void Widget::actionRequestSRQ(){

}


void Widget::actionSglTrig(int opt){
    switch (opt){
        case 1:
        case 3:
        case 4:
            ui->btnStrg->setEnabled(true);
            ui->labStrg->show();
            break;
        case 0:
        case 2:
            ui->btnStrg->setEnabled(false);
            ui->labStrg->hide();
            break;
    }
}


void Widget::actionRange(){
    int r = ui->cmbRange->currentIndex();
    if (r > -1 && r < currentRange[0].count() ) {
        if (r>0) {
            ui->labRng->show();
        }else{
            ui->labRng->hide();
        }
        ui->labRval->setText(ui->cmbRange->currentText());
    }
}


void Widget::actionTrig(){

}


void Widget::actionVAC(){
    funcState = "VAC";
    currentFunc = 2;
    updateFuncStr(funcState);
    ui->labOhm->setText("");
    setMeterRange();
}


void Widget::actionVDC(){
    funcState = "VDC";
    currentFunc = 1;
    updateFuncStr(funcState);
    ui->labOhm->setText("");
    setMeterRange();
}



//##### SERIAL COFIGURATION #####

// Set baud rate
void Widget::setBaudRate(QString baudrate){
    if (baudrate == "115200"){
        serialPort->setBaudRate(QSerialPort::Baud115200);
    }else if (baudrate == "57600"){
        serialPort->setBaudRate(QSerialPort::Baud57600);
    }else if (baudrate == "38400"){
        serialPort->setBaudRate(QSerialPort::Baud38400);
    }else if (baudrate == "19200"){
        serialPort->setBaudRate(QSerialPort::Baud19200);
    }else if (baudrate == "9600"){
        serialPort->setBaudRate(QSerialPort::Baud9600);
    }else if (baudrate == "4800"){
        serialPort->setBaudRate(QSerialPort::Baud4800);
    }else if (baudrate == "2400"){
        serialPort->setBaudRate(QSerialPort::Baud2400);
    }else if (baudrate == "1200"){
        serialPort->setBaudRate(QSerialPort::Baud1200);
    }else{
        serialPort->setBaudRate(QSerialPort::Baud9600);
    }
}


void Widget::setParity(QString parity){
    if (parity == "N"){
        serialPort->setParity(QSerialPort::NoParity);
    }else if (parity == "E"){
        serialPort->setParity(QSerialPort::EvenParity);
    }else if (parity == "O"){
        serialPort->setParity(QSerialPort::OddParity);
    }else if (parity == "M"){
        serialPort->setParity(QSerialPort::MarkParity);
    }else if (parity == "S"){
        serialPort->setParity(QSerialPort::SpaceParity);
    }else{
        serialPort->setParity(QSerialPort::NoParity);
    }
}


void Widget::setDataBits(QString dbits){
    if (dbits == "5"){
        serialPort->setDataBits(QSerialPort::Data5);
    }else if (dbits == "6"){
        serialPort->setDataBits(QSerialPort::Data6);
    }else if (dbits == "7"){
        serialPort->setDataBits(QSerialPort::Data7);
    }else if (dbits == "8"){
        serialPort->setDataBits(QSerialPort::Data8);
    }else{
        serialPort->setDataBits(QSerialPort::Data8);
    }
}


void Widget::setStopBits(QString sbits){
    if (sbits == "1") {
        serialPort->setStopBits(QSerialPort::OneStop);
    }else if (sbits == "1.5"){
        serialPort->setStopBits(QSerialPort::OneAndHalfStop);
    }else if (sbits == "1.5"){
        serialPort->setStopBits(QSerialPort::TwoStop);
    }else{
        serialPort->setStopBits(QSerialPort::OneStop);
    }
}


void Widget::setFlowControl(QString flow){
    if (flow == "None") {
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
    }else if (flow == "XONXOFF") {
        serialPort->setFlowControl(QSerialPort::SoftwareControl);
    }else if (flow == "CRTSCTS") {
         serialPort->setFlowControl(QSerialPort::HardwareControl);
    }else{
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
    }
}


void Widget::custFuncActivate(){
    QString r;
    QString func;

    QString cfgFile = QApplication::applicationDirPath() + "/HPRDmm.conf";
    QSettings conf(cfgFile, QSettings::IniFormat);

    for (int i=0; i<7; i++){
        func = "Function " + r.setNum(i+1);
        if (conf.status() == 0) {
            custFuncs[i][0] = conf.value(func+"/Mnemonic", "").toString();
            custFuncs[i][1] = conf.value(func+"/FunctionScript", "").toString();
            if (custFuncs[i][1] != "") {
                switch(i) {
                case 0: ui->btnFn1->setText(custFuncs[i][0].left(4)); ui->btnFn1->setEnabled(true);
                    break;
                case 1: ui->btnFn2->setText(custFuncs[i][0].left(4)); ui->btnFn2->setEnabled(true);
                    break;
                case 2: ui->btnFn3->setText(custFuncs[i][0].left(4)); ui->btnFn3->setEnabled(true);
                    break;
                case 3: ui->btnFn4->setText(custFuncs[i][0].left(4)); ui->btnFn4->setEnabled(true);
                    break;
                case 4: ui->btnFn5->setText(custFuncs[i][0].left(4)); ui->btnFn5->setEnabled(true);
                    break;
                case 5: ui->btnFn6->setText(custFuncs[i][0].left(4)); ui->btnFn6->setEnabled(true);
                    break;
                case 6: ui->btnFn7->setText(custFuncs[i][0].left(4)); ui->btnFn7->setEnabled(true);
                    break;
                case 7: ui->btnFn8->setText(custFuncs[i][0].left(4)); ui->btnFn8->setEnabled(true);
                    break;
                }
            }else{
                switch(i) {
                case 0: ui->btnFn1->setText("Func1"); ui->btnFn1->setEnabled(false);
                    break;
                case 1: ui->btnFn2->setText("Func2"); ui->btnFn2->setEnabled(false);
                    break;
                case 2: ui->btnFn3->setText("Func3"); ui->btnFn3->setEnabled(false);
                    break;
                case 3: ui->btnFn4->setText("Func4"); ui->btnFn4->setEnabled(false);
                    break;
                case 4: ui->btnFn5->setText("Func5"); ui->btnFn5->setEnabled(false);
                    break;
                case 5: ui->btnFn6->setText("Func6"); ui->btnFn6->setEnabled(false);
                    break;
                case 6: ui->btnFn7->setText("Func7"); ui->btnFn7->setEnabled(false);
                    break;
                case 7: ui->btnFn7->setText("Func8"); ui->btnFn8->setEnabled(false);
                    break;
                }
            }
        }
    }
}


QByteArray Widget::backupCalData(){
    // Backup the calibration data
    QByteArray calData;
    QByteArray cmd;
    char val[1] = {0};
    int i = 0;
    bool mtrflg = mtrTim->isActive();
    bool statflg = statTim->isActive();
    bool lITflg = logITim->isActive();
    bool lDTflg = logDTim->isActive();
    int wflg = waitIsb;

    if (isDebugModeOn) ui->textBrowser->append("Getting calibration data....");

    if (isInstrumentConnected){
        if (mtrflg) mtrTim->stop();
        if (statflg) statTim->stop();
        if (lITflg) logITim->stop();
        if (lDTflg) logDTim->stop();
        waitIsb = 1;

        calData.clear();

        cmd[0] = 0x57;
        cmd[2] = 0x0D;
        for (i=0; i<256; i++){
            cmd[1] = i;
            serialWrite(cmd);
            delay(200);
            serialWrite("++read eoi\r");
            delay(200);
            serialPort->read(val, 1);
            calData.append(val);
            delay(200);
        }

        if (isDebugModeOn) ui->textBrowser->append("CAL data: " + calData + "/r");

    }else{
        calData.append("Instrument not connected.");
    }

    if (lDTflg) logITim->start();
    if (lITflg) logDTim->start();
    if (statflg) statTim->start();
    if (mtrflg) mtrTim->start();
    waitIsb = wflg;

    if (isDebugModeOn) ui->textBrowser->append("Calibration data read.");

    return calData;
}


void Widget::restoreCalData(QByteArray calData){

}



#ifdef Q_OS_WIN32
    #include <Windows.h>
    void Widget::makeBeep(){

        ui->labFunc->setEnabled(false);
        ui->labCont->setEnabled(true);
        Beep(659.25, 200);
        ui->labFunc->setEnabled(true);
        ui->labCont->setEnabled(false);
    }
#elif Q_OS_LINUX
    #include <stdio.h>
    void Widget::makeBeep(){
        system("echo -e "\007" >/dev/tty10");
    }
#else
    #include <stdio.h>
    void Widget::makeBeep(){
        cout << "\a" << flush;
    }
#endif
