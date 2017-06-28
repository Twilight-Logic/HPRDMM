#ifndef HPRDMMDIALOG_H
#define HPRDMMDIALOG_H

#include "HPRDmmDialog.h"

#include <QWidget>
#include <QSerialPort>
#include <QFile>
#include <QFileDevice>
#include <QTextStream>

namespace Ui {
class Widget;
}

struct serialConf {
    QString Port;
    QString BaudRate;
    QString Parity;
    QString DataBits;
    QString StopBits;
    QString FlowControl;
};

struct logConf {
    bool isEnabled;
    bool isActive;
//!    int Interval;
    int IntUnit;
    QString FileName;
};


//!struct mtrStatus {
//!    int digits;
//!    int range;
//!    int func;
//!    bool intTrig;
//!    bool aRng;
//!    bool aZero;
//!    bool fiftyHz;
//!    bool front;
//!    bool cal;
//!    bool exTrig;
//!    bool sqrDr;
//!    bool srqSyn;
//!    bool srqInt;
//!    bool srqFront;
//!    bool calErr;
//!    bool srqPwon;
//!    int dacVal;
//!};


class Widget : public QWidget
{
    Q_OBJECT

signals:
    void writeRequest(const QByteArray &data);

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void initMeter();
//!    void initFunc();
    void updateMeterStr(QString string);
    void updateFuncStr(QString string);
    void restoreCalData(QByteArray calData);


private:
    Ui::Widget *ui;
    int mtrDigits;

    //! Serial Port
    QSerialPort *serialPort;
    serialConf serialCfg;
    //! Serial Buffers
    QByteArray serBuf;
    QByteArray lineBuf;
    //! Serial config
    void setBaudRate(QString brate);
    void setParity(QString parity);
    void setDataBits(QString dbits);
    void setStopBits(QString sbits);
    void setFlowControl(QString flow);
    //! Serial comms
    //! Connect serial port
    void serialConnect();
    void setDefaultSerialCfg();
    void serialWriteWait(QByteArray data);
//!    void readLineOfData();
    void processLine(QByteArray data);
    void waitForResponse();
    //! GPIB
    void setGpibAddr();
    void verifyInstrConnect();
    void setMeterRange();

    //! Instrument status
//!    mtrStatus instrStat;
    QTimer *statTim;
    QByteArray instrStat;
    char iStat[5] = {0};
//!    int statInterval;
//!    void chkInstrStat();
//!    void rcvInstrStat(QByteArray data);
    void updInstrStat(char data[5]);
    bool waitIsb;

    //! Logging and log config
    logConf logCfg;
    QTimer *logITim;
    QTimer *logDTim;
    QFile logFile;
    QTextStream logStrm;
    //! Log functions
    void initLogging();

    //! Button Actions
    void actionClose();
    void actionAAC();
    void actionADC();
//!    void actionGetInstrumentAddr();
    void actionAutoZeroOff(bool stat);
    void actionDigits3();
    void actionDigits4();
    void actionDigits5();
//!    void actionLocal();
    void actionOhms2Wire();
    void actionOhms4Wire();
    void actionOhmsExtend();
//!    void actionResetMeter();
    void actionRequestSRQ();
    void actionSglTrig(int opt);
    void actionRange();
    void actionTrig();
    void actionVAC();
    void actionVDC();
//!    void actionConn();
//!    void actionLogging();
//!    void actionDisplay();
//!    void actionLogCont();


    //! Misc functions
//!    QString packageOutputCmd(QString cmd);
    QString calcExOhm(QString reading);
    void sendCMD(QString cmd);
    void initRanges();
    void runCustomCMD(int func);
    void delay(int msec);
    void readSuProfile();
    void setDisplayStyle(int idx);


    //! Meter reading interval timer
    QTimer *mtrTim;
//!    int mtrInterval;

    //! GPIB address and controller ID
    QString gpibAddr;
    QString gpibCidStr;

    //! Custom functons array
//!    QString mainFuncs[7];
    QString custFuncs[7][2];
    QStringList rangesVDC[2];
    QStringList rangesVAC[2];
    QStringList rangesOHM[2];
    QStringList rangesAAC[2];
    QStringList rangesADC[2];
    QStringList rangesEXO[2];
    QStringList currentRange[2];
    QStringList trigOptions;
    QStringList dispStyle;

    //! State flags
    bool isControllerOk;
    bool isAddrSet;
    bool isStatOk;
    bool isInstrumentConnected;
    bool responseReceived;
    bool azOff;
    bool autoRange;
    bool isDisplayOn;
    bool isDebugModeOn;
    bool minMode;
    bool avgMode;
    bool maxMode;
    bool isCont;
    //! Parameters
    QString mtrPol;
    QString mtrReading;
    QString mtrXply;
    QString funcState;
    QString trigState;
    double mtrVal;
    double mtrAvg;
    double exoR1Val;
    int mtrAvgCnt;
    int currentFunc;
    int suProfile;
    bool contBeep;
    int contThreshold;

private slots:
    //! BUTTON SLOTS
    //! Line (power on/off) button - close application
    void closeHPRDMM();
    //! Select current AC
    void selectAAC();
    //! Select current DC
    void selectADC();
    //! Get device address
    void showGpibAddr();
    //! Auto zero on / off
    void toggleAutoZero();
    //! 3 digit display
    void selectDigits3();
    //! 4 digit display
    void selectDigits4();
    //! 5 digit display
    void selectDigits5();
    //! INT Trig
//!    void selectIntTrig();
    //! Local
    void selectLocal();
    //! Ohms 2 wire
    void selectOhms2Wire();
    //! Ohms 4 wire
    void selectOhms4Wire();
    //! Ohms Extended
    void selectOhmsExtend();
    //! Reset meter
    void resetMeter();
    //! SRQ
    void requestSRQ();
    //! SGL trig
    void selectSglTrig();
    //! Select range
    void selectRange();
    //! Select trigger mode
    void selectTrig();
    //! Volts AC
    void selectVAC();
    //! Volts DC
    void selectVDC();
    //! Connect/disconnect instrument
    void selectConn();
    //! Toggle Logging status
    void toggleLogging();
    //! Instrument display on/off
    void toggleDisplay();
    //! Log continuous (reset timer to zero)
    void selectLogCont();
    //! Maximum reading mode
    void selectMax();
    //! Average reading mode
    void selectAvg();
    //! Minimum reading mode
    void selectMin();
    //! Clear to standard reading mode
    void selectReal();
    //! Diode test mode
    void selectDiode();
    //! Continuity test mode
    void selectCont();

    //! Function 1
    void selectFn1();
    //! Function 2
    void selectFn2();
    //! Function 3
    void selectFn3();
    //! Function 4
    void selectFn4();
    //! Function 5
    void selectFn5();
    //! Function 6
    void selectFn6();
    //! Function 7
    void selectFn7();
    //! Function 8
    void selectFn8();


    //! ACTION SLOTS
    //! Load configuration
    void loadConfig();
    //! Disconnect serial port
    void serialDisconnect();
    //! Write to serial port
    void serialWrite(const QByteArray &data);
    //! Read serial port (data ready)
    void serialRead();
    //! Connect GPIB controller
    void ctrlrConnect();
    //! Function button activation
    void custFuncActivate();
    //! Read the meter
    void readMeter();
    //! Restart timer after sending command
//!    void restartMeterTimer();
    //! Start log running
    void recordLog();
    //! Stop the running log
    void stopLogging();
    //! Check the instrument status
    void chkInstrStat();

    void makeBeep();

    QByteArray backupCalData();


};

#endif // HPRDMMDIALOG_H
