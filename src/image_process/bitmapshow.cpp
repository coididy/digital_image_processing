#include "bitmapshow.h"
#include "ui_bitmapshow.h"

BitMapShow::BitMapShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BitMapShow)
{
    ui->setupUi(this);
    label[0]=ui->label_1;
    label[1]=ui->label_2;
    label[2]=ui->label_3;
    label[3]=ui->label_4;
    label[4]=ui->label_5;
    label[5]=ui->label_6;
    label[6]=ui->label_7;
    label[7]=ui->label_8;
}
void BitMapShow::bitMapDisplay(QImage *origin)
{
    //位分解，只针对256级灰度的图像进行，及对newImage处理
    QVector<int>bit(8);
    bit[0]=1;bit[1]=2;bit[2]=4;bit[3]=8;bit[4]=16;bit[5]=32;bit[6]=64;bit[7]=128;
    QImage image[8]={QImage(origin->width(),origin->height(),QImage::Format_ARGB32),
                     QImage(origin->width(),origin->height(),QImage::Format_ARGB32),
                     QImage(origin->width(),origin->height(),QImage::Format_ARGB32),
                     QImage(origin->width(),origin->height(),QImage::Format_ARGB32),
                     QImage(origin->width(),origin->height(),QImage::Format_ARGB32),
                     QImage(origin->width(),origin->height(),QImage::Format_ARGB32),
                     QImage(origin->width(),origin->height(),QImage::Format_ARGB32),
                     QImage(origin->width(),origin->height(),QImage::Format_ARGB32)};
    for(int y=0;y<origin->height();y++)
    {
          QRgb *line=reinterpret_cast<QRgb *>(origin->scanLine(y));//C++中强制类型转换的方法
          //QRgb *line=(QRgb *)origin->scanLine(y);
          for(int x=0;x<origin->width();x++)
          {
              for(int i=0;i<8;i++)
              {
                  int newRedValue,newGreenValue,newBlueValue;
                  if((qRed(line[x])&bit[i])==bit[i]) newRedValue=255;
                  else newRedValue=0;
                  if((qGreen(line[x])&bit[i])==bit[i]) newGreenValue=255;
                  else newGreenValue=0;
                  if((qBlue(line[x])&bit[i])==bit[i]) newBlueValue=255;
                  else newBlueValue=0;
                  image[i].setPixel(x,y,qRgb(newRedValue,newGreenValue,newBlueValue));
              }
          }
    }
    for(int i=0;i<8;i++)
    {
        image[i].scaled(label[i]->size(),Qt::KeepAspectRatio);
        label[i]->setScaledContents(true);
        label[i]->setPixmap(QPixmap::fromImage(image[i]));
    }

}
BitMapShow::~BitMapShow()
{
    delete ui;
}
