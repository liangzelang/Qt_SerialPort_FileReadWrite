#include "browsedialog.h"
//#include "settingsdialog.h"
#include "ui_browsedialog.h"
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QDataStream>
#include <QByteArray>
#include <QComboBox>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QRadioButton>

BrowseDialog::BrowseDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    serial = new QSerialPort(this);
    connect(radioButton,SIGNAL(toggled(bool)),this,SLOT(toggleSerialPort()));
    connect(browsepushButton,SIGNAL(clicked(bool)),this,SLOT(browseFile()));
    connect(cancelpushButton,SIGNAL(clicked(bool)),this,SLOT(reject()));
   // connect(okpushButton,SIGNAL(clicked(bool)),this,SLOT(accept()));
    connect(okpushButton,SIGNAL(clicked(bool)),this,SLOT(writeSerialData()));
    connect(refreshpushButton,SIGNAL(clicked(bool)),this,SLOT(refreshSerialPorts()));
    connect(clearpushButton,SIGNAL(clicked()),this,SLOT(clearBufferCache()));
    connect(serial,SIGNAL(readyRead()),this,SLOT(readSerialData()));
  //  connect(plainTextEdit, SIGNAL(getData(QByteArray)), this, SLOT(writeSerialData(QByteArray)));
    fillPortsParameters();
    fillPortsNames();
}

BrowseDialog::~BrowseDialog()
{
    //delete ui;
}

void BrowseDialog::browseFile()
{
    QString initialName = lineEdit->text();     //default get the path from the lineEdit component
    if(initialName.isEmpty())                            //if the default path is empty ,I set the home path as the default
            initialName = QDir::homePath();
    QString fileName =
            QFileDialog::getOpenFileName(this,tr("choose file"),initialName);  //at the path:initialName ,user choose which file to upload
    fileName = QDir::toNativeSeparators(fileName);
    if(!fileName.isEmpty())
    {
        lineEdit->setText(fileName);
        okpushButton->setEnabled(true);
        plainTextEdit->insertPlainText(fileName);
        QFile file(fileName);
       plainTextEdit->appendPlainText("\n");
       if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
           qDebug()<<"Can't open the file!"<<endl;
       }
       while(!file.atEnd())
       {
            binByteArray = file.readAll();
           //binData = str.toHex();
          // binData = str.toHex().data();
         plainTextEdit->insertPlainText(binByteArray.toHex());
       }
    }

}

void BrowseDialog::openFile()
{
    close();
}

void BrowseDialog::fillPortsParameters()
{
    baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    baudRateBox->addItem(QStringLiteral("115200"),QSerialPort::Baud115200);
    baudRateBox->addItem(tr("Custom"));

    stopBitsBox->addItem(QStringLiteral("1"),QSerialPort::OneStop);
    stopBitsBox->addItem(QStringLiteral("2"),QSerialPort::TwoStop);

    dataBitsBox->addItem(QStringLiteral("7"),QSerialPort::Data7);
    dataBitsBox->addItem(QStringLiteral("8"),QSerialPort::Data8);
    dataBitsBox->setCurrentIndex(1);

    parityBox->addItem(QStringLiteral("Even"),QSerialPort::EvenParity);
    parityBox->addItem(QStringLiteral("Odd"),QSerialPort::OddParity);
    parityBox->addItem(QStringLiteral("None"),QSerialPort::NoParity);
    parityBox->setCurrentIndex(2);
}

void BrowseDialog::fillPortsNames()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        list << info.portName();
        comNameBox->addItem(list.first(),list);
    }
    comNameBox->addItem(tr("Custom"));
}

void BrowseDialog::refreshSerialPorts()
{
    comNameBox->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        list << info.portName()  ;
        //ui->serialPortInfoListBox->addItem(list.first(), list);
        comNameBox->addItem(list.first(),list);
    }
    comNameBox->addItem(tr("Custom"));
}

void BrowseDialog::clearBufferCache()
{
        plainTextEdit->clear();
}


//first ,get the current setings ,
//second, configure the com with the setting
//third, open the COM
//forth, enable the receive
//fiveth, if possiple ,add the send feature.
//Settings p=new BrowseDialog::Settings;
void BrowseDialog::startSerialPort()
{
    Settings p ;
    p.name = comNameBox->currentText();
    p.baudRate = static_cast<QSerialPort::BaudRate>(
                 baudRateBox->itemData( baudRateBox->currentIndex()).toInt());
    p.stringBaudRate = QString::number(p.baudRate);
    p.dataBits =static_cast<QSerialPort::DataBits>(
                dataBitsBox->itemData(dataBitsBox->currentIndex()).toInt());
    p.stringDataBits =  dataBitsBox->currentText();
    p.stopBits = static_cast<QSerialPort::StopBits>(
                stopBitsBox->itemData(stopBitsBox->currentIndex()).toInt());
    p.stringStopBits = stopBitsBox->currentText();
    p.parity = static_cast<QSerialPort::Parity>(
                parityBox->itemData(parityBox->currentIndex()).toInt());
    p.stringParity = parityBox->currentText();
   // p.flowControl =

    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite))
    {
         plainTextEdit->insertPlainText(tr("Connected to %1 : %2, %3, %4, %5")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits));
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

    }
    okpushButton->setEnabled(true);
 }

void BrowseDialog::readSerialData()
{
    QByteArray data = serial->readAll();
   // console->putData(data); insertPlainText(QString(data));
    plainTextEdit->insertPlainText(QString(data));
}

void BrowseDialog::closeSerialPort()
{
    if(serial->isOpen())
    {
        serial->close();
    }
    plainTextEdit->insertPlainText(tr("SerialPort is closed. \n"));
}

void BrowseDialog::toggleSerialPort()
{
    if(radioButton->isChecked())
    {
        startSerialPort();
    }
    else
    {
        closeSerialPort();
    }
}

void BrowseDialog::writeSerialData( )
{
    //char InsyncData[] = {0x21,0x20};,sizeof(binByteArray.toHex())
   // serial->write(binByteArray.toHex());
    progressBar->setRange(0,100);
    int length=0;
    length = sizeof(binByteArray)>>10;

    serial->write(binByteArray);
    //serial->write(InsyncData,sizeof(InsyncData));
    //plainTextEdit->insertPlainText(binByteArray.toHex());
    plainTextEdit->insertPlainText(tr("\n send OK \n"));
}

