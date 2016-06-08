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
#include <QThread>
#include <QProcess>
//#include <QTestEventList>

BrowseDialog::BrowseDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    serial = new QSerialPort(this);
    connect(radioButton,SIGNAL(toggled(bool)),this,SLOT(toggleSerialPort()));
    connect(browsepushButton,SIGNAL(clicked(bool)),this,SLOT(browseFile()));
    connect(cancelpushButton,SIGNAL(clicked(bool)),this,SLOT(reject()));
   // connect(okpushButton,SIGNAL(clicked(bool)),this,SLOT(accept()));
    connect(uploadpushButton,SIGNAL(clicked(bool)),this,SLOT(uploadBinFile()));
    connect(okpushButton,SIGNAL(clicked(bool)),this,SLOT(writeSerialData()));
    connect(refreshpushButton,SIGNAL(clicked(bool)),this,SLOT(refreshSerialPorts()));
    connect(clearpushButton,SIGNAL(clicked()),this,SLOT(clearBufferCache()));
    connect(serial,SIGNAL(readyRead()),this,SLOT(readSerialData()));
    connect(this,SIGNAL(gotSerialData()),this,SLOT(uploadBinFile()));
    connect(startpushButton,SIGNAL(clicked(bool)),this,SLOT(startApplication()));
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
     //  QDataStream in(&file);
    //   datalen = in.readRawData(dataBuf,10);
    //   serial->write(dataBuf,10);
       while(!file.atEnd())
       {
           //QByteArray buf = file.readLine(512);
           binByteArray.append(file.readLine(512));
       }
       dataLen = file.size();
       plainTextEdit->insertPlainText(QString("%1").arg((dataLen)));
       plainTextEdit->appendPlainText("\n");
       //plainTextEdit->insertPlainText(binByteArray.toHex());
        plainTextEdit->insertPlainText(binByteArray.toHex());
       file.close();
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
    //quint16 serialReceiveDataLength = serial->readBufferSize();
    //plainTextEdit->insertPlainText(QString("%2").arg(serialReceiveDataLength));
    QByteArray serialReceiveData = serial->readAll();

    char* serialBuf = serialReceiveData.data();
    plainTextEdit->insertPlainText(QString(serialReceiveData));
    echoFlag=0;
    if((serialBuf[0]==0x12)&&(serialBuf[1]==0x10))
    {
        echoFlag=1;
        plainTextEdit->insertPlainText("send data OK \n");
        emit gotSerialData();
       // QMessageBox::critical(this, tr("Error"), tr("Got the Echo !!!"));
    }
    else
        echoFlag=0;
    //plainTextEdit->insertPlainText(QString("%1").arg(serialBuf[0]));
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
    plainTextEdit->insertPlainText(QString(length));

    serial->write(binByteArray);
    //serial->write(InsyncData,sizeof(InsyncData));
    //plainTextEdit->insertPlainText(binByteArray.toHex());
    plainTextEdit->insertPlainText(tr("\n send OK \n"));
}
void BrowseDialog::startApplication()
{
    char rebootData[2]={0x30,0x20};
    serial->write(rebootData,sizeof(rebootData));
}
void BrowseDialog::uploadBinFile()
{
    char writedByte;
    char  temp=0;
    char localWriteDataLength=0;

    char eocData[1]={0x20};
    char insyncData[2]={0x21,0x20};
    char eraseData[2]={0x23,0x20};
    char multiProgData[2]={0x27,writeLength};

    char Reboot_ID1[41]={0xfe,0x21,0x72,0xff,0x00,0x4c,0x00,0x00,0x80,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf6,0x00,0x01,0x00,0x00,0x48,0xf0};
    char Reboot_ID0[41]={0xfe,0x21,0x45,0xff,0x00,0x4c,0x00,0x00,0x80,0x3f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf6,0x00,0x00,0x00,0x00,0xd7,0xac};



    if(serial->isOpen())
    {

        switch(progressNumber)
        {
        case 5:
            serial->write(insyncData,sizeof(insyncData));
            serial->write(Reboot_ID0,sizeof(Reboot_ID0));
            serial->write(Reboot_ID1,sizeof(Reboot_ID1));
            //sleep(1500);
            QThread::msleep(1500);
            serial->readAll();       //clear the receive buffer.
             serial->write(insyncData,sizeof(insyncData));
             progressNumber++;
        case 0:
            serial->write(insyncData,sizeof(insyncData));
            progressNumber++;
            break;

        case 1:
            serial->write(eraseData,sizeof(eraseData));
            progressNumber++;
            //QMessageBox::critical(this, tr("erase "), tr("why me !!!"));
            break;

        case 2:
            localWriteDataLength= writeLength;   //252
            temp = writeDataTimes;          //0  1
            multiProgData[0] = 0x27;
            multiProgData[1] = writeLength;
            if((dataLen>(writeDataTimes*writeLength))&&(dataLen<((writeDataTimes+1)*writeLength)))
            {
                //writeLength = dataLen-writeLength*writeDataTimes;
                multiProgData[0] = 0x27;
                multiProgData[1] = dataLen-writeLength*writeDataTimes ;
                serial->write(multiProgData,sizeof(multiProgData));
                writedByte=serial->write(&(binByteArray.data()[writeDataTimes*writeLength]),(dataLen-writeLength*writeDataTimes));
                serial->write(eocData,sizeof(eocData));
                QMessageBox::critical(this,tr("finish"),tr("finish transmission"));
                progressNumber++;
                writeDataTimes=0;
                writeLength = 252;
                break;
            }
            serial->write(multiProgData,sizeof(multiProgData));
            writedByte=serial->write(&(binByteArray.data()[writeDataTimes*writeLength]),writeLength);
            serial->write(eocData,sizeof(eocData));
            writeDataTimes++;
            if(dataLen==(writeDataTimes*writeLength))
            {
                   QMessageBox::critical(this, tr("erase "), tr("why me !!!"));
                    progressNumber++;
                    writeDataTimes=0;
                    writeLength = 252;
            }
           break;
       default:
           //uploadpushButton       disable the uploadpushButton
            break;
        }
    }
    else
    {
       // startSerialPort();
       // QMessageBo
        plainTextEdit->insertPlainText(tr("Please open serial port before Upload!!!"));
        QMessageBox::critical(this, tr("Error"), tr("Please open serial port before Upload!!!"));
    }

}

void BrowseDialog::waitForEcho()
{

}
