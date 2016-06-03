#ifndef BROWSEDIALOG_H
#define BROWSEDIALOG_H
#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include "ui_browsedialog.h"

//namespace Ui {     //this class is in the Ui   ,but the below get the feature of Ui::BrowseDialog   and QDialog
//class BrowseDialog;
//}

class BrowseDialog : public QDialog ,public Ui::BrowseDialog
{
    Q_OBJECT

public:
    explicit BrowseDialog(QWidget *parent = 0);
    ~BrowseDialog();

    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
    };
private slots:
    void browseFile();    //the browseButton
    //  void quit                //the canselButton
    void openFile();  // the okButton

private:
    void fillPortsParameters();
private:
    Settings currentSettins;
};

#endif // BROWSEDIALOG_H
