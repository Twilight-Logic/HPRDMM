#ifndef HPRDMMCFG_H
#define HPRDMMCFG_H

#include <QDialog>

namespace Ui {
class HPRDmmCfg;
}

class HPRDmmCfg : public QDialog
{
    Q_OBJECT

signals:
    QByteArray getCalData();

public:
    explicit HPRDmmCfg(QWidget *parent = 0);
    ~HPRDmmCfg();


private:
    Ui::HPRDmmCfg *ui;

    void initCfgBox();
    void loadConfig();

private slots:
    void saveConfig();
    void setLogCfgStatus();
    void selectSprofCfg();
    void getProfileList();
    void enableR1Cfg();
    void readCalData();

};

#endif // HPRDMMCFG_H
