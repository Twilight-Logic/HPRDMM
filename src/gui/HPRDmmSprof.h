#ifndef HPRDmmSprof_H
#define HPRDmmSprof_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class HPRDmmSprof;
}

class HPRDmmSprof : public QDialog
{
    Q_OBJECT

public:
    explicit HPRDmmSprof(QWidget *parent = 0);
    ~HPRDmmSprof();



private:
    Ui::HPRDmmSprof *ui;

    void initProfCfgBox();
    QSettings conf;
    QString cfgFile;
    QStringList rangesVDC;
    QStringList rangesVAC;
    QStringList rangesOHM;
    QStringList rangesAAC;
    QStringList rangesADC;
    QStringList rangesEXO;

private slots:

    void saveProfile();
    void readProfile();
    void rstProfile();
    void delProfile();
    void setRange();



};

#endif // HPRDmmSprof_H
