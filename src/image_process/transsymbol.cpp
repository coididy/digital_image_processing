#include "transsymbol.h"
#include "ui_transsymbol.h"

TransSymbol::TransSymbol(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransSymbol)
{
    ui->setupUi(this);
}

TransSymbol::~TransSymbol()
{
    delete ui;
}
void TransSymbol::symbolDisplay(QImage *image)
{
    QImage *newImage=new QImage(image->width(),image->height(),QImage::Format_ARGB32);
    //提供的字符图片是10*10像素的
    int wnum=newImage->width()/10;//宽方向上有wnum个10*10小块
    int hnum=newImage->height()/10;//高方向上有hnum个10*10小块
    for(int i=0;i<hnum;i++)//高
    {
        for(int j=0;j<wnum;j++)//宽
        {
            int oldsum=0;
            int oldaverage=0;
            for(int m=0;m<10;m++)
            {
                for(int n=0;n<10;n++)
                {
                    QColor color=QColor(image->pixel(10*j+m,10*i+n));
                    oldsum=oldsum+color.red();
                }

            }
            oldaverage=oldsum/100;//该小块儿的灰度均值
            //255÷18=14.16……,一共准备了15张ASCII符号的图片，将256个灰度分成15级
            QString imagename=":/images/resources/ASCII/"+QString::number(oldaverage/18,10) +".jpg";//找到与该小块匹配的字符路径
            QImageReader imgReader(imagename);
            QImage ASCIIimage;//局部变量，用于读取ASCII字符图片
            imgReader.read(&ASCIIimage);
            for(int m=0;m<10;m++)//将该小块字符化
            {
                for(int n=0;n<10;n++)
                {
                    QColor color=QColor(ASCIIimage.pixel(m,n));
                    newImage->setPixel(10*j+m,10*i+n,qRgb(color.red(),color.green(),color.blue()));
                }
            }
        }
    }
    ui->label->setPixmap(QPixmap::fromImage(*newImage));
}
