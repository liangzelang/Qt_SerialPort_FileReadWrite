#include "browsedialog.h"
#include <QApplication>
#include <QDialog>
//#include "ui_browsedialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
     BrowseDialog w;
     w.show();

    //Ui::BrowseDialog ui;
    //QDialog *dialog = new QDialog;
    //ui.setupUi(dialog);
    //dialog->show();

    return a.exec();
}
