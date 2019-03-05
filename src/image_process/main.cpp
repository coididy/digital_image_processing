#include "mainwindow.h"
#include <QApplication>
#include"openimage.h"
#include<QFile>
#include<QDebug>
#include<QString>
#include<QBitArray>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QString qss;
    QFile qssFile(":/myqss/resources/MyQss/myqss.qss");
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen())
    {
        qss = QLatin1String(qssFile.readAll());
        qApp->setStyleSheet(qss);
        qssFile.close();
    }
    QObject::connect(&a,&QApplication::lastWindowClosed,&a,&QApplication::quit);//主窗口关闭后，其他窗口全部关闭
    //w.show();
    OpenImage openImage;
    openImage.show();
    QObject::connect(&openImage,&OpenImage::openImageFile,&w,&MainWindow::windowShow);
    return a.exec();
}
