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

BrowseDialog::BrowseDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    baudRateBox->setInsertPolicy(QComboBox::NoInsert);
    connect(browsepushButton,SIGNAL(clicked(bool)),this,SLOT(browseFile()));
    connect(cancelpushButton,SIGNAL(clicked(bool)),this,SLOT(reject()));
    connect(okpushButton,SIGNAL(clicked(bool)),this,SLOT(accept()));

    fillPortsParameters();
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
           QByteArray str = file.readAll();
           plainTextEdit->insertPlainText(str.toHex());
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
    dataBitsBox->setCurrentIndex(0);

    parityBox->addItem(QStringLiteral("Even"),QSerialPort::EvenParity);
    parityBox->addItem(QStringLiteral("Odd"),QSerialPort::OddParity);
    parityBox->addItem(QStringLiteral("None"),QSerialPort::NoParity);
    parityBox->setCurrentIndex(2);

}
