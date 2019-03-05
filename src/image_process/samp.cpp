#include "samp.h"
#include "ui_samp.h"

Samp::Samp(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Samp)
{
    ui->setupUi(this);
    ui->label->setScaledContents(true);
}

Samp::~Samp()
{
    delete ui;
}

void Samp::changSamp(int sampsize,int quantificationsize,const QImage &image)
{
    QImage myImage=image.copy();
    this->setWindowTitle(tr("采样系数")+QString::number(sampsize,10)+tr(",量化等级")+QString::number(quantificationsize,10));
    QImage *nowimage=new QImage(myImage.width()/sampsize+1,myImage.height()/sampsize+1,QImage::Format_ARGB32);
    if(sampsize!=1) myImage=*gauss_Smoothness(&myImage);
    int m=0,n=0;
    for(int i=0;i<myImage.height();i++)
    {
        if(i%sampsize!=0) continue;
        for(int j=0;j<myImage.width();j++)
        {
            if(j%sampsize!=0) continue;
            QColor color=myImage.pixel(j,i);
            int r,g,b;
            r=color.red();
            g=color.green();
            b=color.blue();
            if(quantificationsize!=256)
            {
                //灰度等级变化
                r=getvalue(quantificationsize,r);
                g=getvalue(quantificationsize,g);
                b=getvalue(quantificationsize,b);
            }
            nowimage->setPixel(n,m,qRgb(r,g,b));
            n++;
        }
        n=0;
        m++;
    }

    //显示新图像
    nowimage->scaled(ui->label->size(),Qt::KeepAspectRatio);
    ui->label->setFixedSize(myImage.width(),myImage.height());

    ui->label->setPixmap(QPixmap::fromImage(*nowimage));
}
int Samp::getvalue(int level,int color)
{
    int block_num=level-1;
    int block_size=256/block_num;
    for(int i=1;i<=block_num;i++)
    {
        if(color>block_size*i) continue;
        int mid_value=block_size*i/2;
        int left=block_size*(i-1);
        int right=block_size*i-1;
        if(color<mid_value) return  left;
        else return  right;
    }
    return color;
}
QImage* Samp::gauss_Smoothness(QImage *origin)//高斯平滑
{
    //QImage *gaussImage=new QImage(origin->width(),origin->height(),QImage::Format_ARGB32);
    QImage *gaussImage=new QImage(*origin);
    int kernel[5][5]={/*高斯滤波矩阵*/
        {1,4,6,4,1},
        {4,16,24,16,4},
        {6,24,36,24,6},
        {4,16,24,16,4},
        {1,4,6,4,1},
    };
    int kernelSize=5;
    int sumKernel=256;
    int r,g,b;
    QColor color;
    for(int x=kernelSize/2;x<gaussImage->width()-(kernelSize/2);x++)
    {
        for(int y=kernelSize/2;y<gaussImage->height()-(kernelSize/2);y++)
        {
            r=0;
            g=0;
            b=0;
            for(int i=-kernelSize/2;i<=kernelSize/2;i++)
            {
                for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                {
                    color = QColor(origin->pixel(x+i, y+j));//原图的像素rgb
                    r+=color.red()*kernel[kernelSize/2+i][kernelSize/2+j];
                    g+=color.green()*kernel[kernelSize/2+i][kernelSize/2+j];
                    b+=color.blue()*kernel[kernelSize/2+i][kernelSize/2+j];
                }
            }
            r=qBound(0,r/sumKernel,255);
            g=qBound(0,g/sumKernel,255);
            b=qBound(0,b/sumKernel,255);
            gaussImage->setPixel(x,y,qRgb(r,g,b));
        }
    }
    return gaussImage;
}
