#include "graytransform.h"
#include "ui_graytransform.h"

GrayTransform::GrayTransform(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GrayTransform),hist(256)
{
    ui->setupUi(this);
    ui->gray_histogram->setWindowTitle("灰度直方图");
    this->setWindowTitle(tr("图像变换"));

    flag=-1;
    is_gray=false;
    paramsNumbers=1;//默认一个参数有效
    delta_x=delta_y=0;
    maxcount=0;//记录灰度值最多的像素的个数
    graysum=0;//记录灰度值的和
    midgray=0;//记录中值灰度的值
    averageGray=0;//记录平均灰度值
    standardDeviation=0;//记录灰度标准差
    deviation=0;//方差的pixelnum倍
}
GrayTransform::~GrayTransform()
{
    delete ui;
}
void GrayTransform::GrayTransDisplay(const QImage origin,int temp,int times,bool judge)
{
    is_gray=judge;
    if(times==1) image=new QImage(origin.copy());

    //QImage *newImage=new QImage(origin.copy());
    QImage *newImage=new QImage(origin.width(),origin.height(),QImage::Format_ARGB32);
    newImage->fill(qRgb(255,255,255));//以白色填充
    flag=temp;
    bool ok=true;
    /*每次调用该函数需要清空数据*/
    for(int i=0;i<256;i++) hist[i]=0;
    maxcount=0;//记录灰度值最多的像素的个数
    graysum=0;//记录灰度值的和
    midgray=0;//记录中值灰度的值
    averageGray=0;//记录平均灰度值
    standardDeviation=0;//记录灰度标准差
    deviation=0;//方差的pixelnum倍
    int pixelnum=image->width()*image->height();//该变量不能设置为成员变量，容易崩溃，除0操作
    if(flag==0||flag==1)//图像变亮或变暗
    {
        if(times==1)//第一次调用，对Slider进行初始化
        {
            ui->sliderValue->setText("50");
            ui->horizontalSlider->setValue(20);
            ui->horizontalSlider->setPageStep(5);
            ui->horizontalSlider->setMinimum(0);
            ui->horizontalSlider->setMaximum(255);
        }
        QString string;
        int changesize;
        if(flag==0)
        {
            string="图片整体变亮:newpixel = oldpixel + "+ui->sliderValue->text();
            changesize=ui->sliderValue->text().toInt(&ok,10);
        }
        else
        {
            string="图片整体变暗:newpixel = oldpixel - "+ui->sliderValue->text();
            changesize=-ui->sliderValue->text().toInt(&ok,10);
        }
        ui->lineEdit->setText(string);
        for(int y=0;y<image->height();y++)
        {
            for(int x=0;x<image->width();x++)
            {
                QColor color=image->pixel(x,y);
                int r,g,b;
                //像素变化函数
                r=color.red()+changesize;
                g=color.green()+changesize;
                b=color.blue()+changesize;
                r=qBound(0,r,255);
                g=qBound(0,g,255);
                b=qBound(0,b,255);
                newImage->setPixel(x,y,qRgb(r,g,b));//更新图像的像素

                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==2)//图像反相
    {
        //图像不需要进行第二次变化
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);

        ui->horizontalSlider->setValue(20);
        ui->horizontalSlider->setPageStep(5);
        ui->horizontalSlider->setMinimum(0);
        ui->horizontalSlider->setMaximum(255);
        QString string="图片反相，即原图像的负片:newpixel = 255 - oldpixel";
        ui->lineEdit->setText(string);
        for(int y=0;y<image->height();y++)
        {

            for(int x=0;x<image->width();x++)
            {
                QColor color=image->pixel(x,y);
                int r,g,b;
                //像素变化函数
                r=255-color.red();
                g=255-color.green();
                b=255-color.blue();

                newImage->setPixel(x,y,qRgb(r,g,b));//更新图像的像素

                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if (flag==3)//对数变换，显示细节
    {
        if(times==1)//第一次调用，对Slider进行初始化
        {
            ui->sliderValue->setText("9");
            ui->horizontalSlider->setValue(9);//初始值9
            ui->horizontalSlider->setPageStep(10);
            ui->horizontalSlider->setMinimum(0);//最小值0
            ui->horizontalSlider->setMaximum(1000);//最大值1000
            ui->horizontalSlider->setMaximumWidth(2000);//Slider最长2000
        }
        QString string="对数变换，显示细节:newpixel = 255×log"+QString::number(ui->sliderValue->text().toInt(&ok,10)+1)+" ( 1 + "+ui->sliderValue->text()+" * oldpixel)";
        ui->lineEdit->setText(string);
        for(int y=0;y<image->height();y++)
        {

            for(int x=0;x<image->width();x++)
            {
                QColor color=image->pixel(x,y);
                int r,g,b;
                //像素变化函数
                if(ui->sliderValue->text()=="0")
                {
                    r=color.red();
                    g=color.green();
                    b=color.blue();
                }
                else
                {
                    r=static_cast<int>(255*qLn(1.0+ui->sliderValue->text().toDouble()*(color.red()/255.0))/qLn(1.0+ui->sliderValue->text().toDouble())) ;
                    g=static_cast<int>(255*qLn(1.0+ui->sliderValue->text().toDouble()*(color.green()/255.0))/qLn(1.0+ui->sliderValue->text().toDouble())) ;
                    b=static_cast<int>(255*qLn(1.0+ui->sliderValue->text().toDouble()*(color.blue()/255.0))/qLn(1.0+ui->sliderValue->text().toDouble())) ;
                }

                newImage->setPixel(x,y,qRgb(r,g,b));//更新图像的灰度

                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==4)//伽马变换，图像矫正
    {
        if(times==1)//第一次调用，对Slider进行初始化
        {
            //QSlider无法设置float类型，因此将其扩充10倍
            ui->sliderValue->setText("1.0");
            ui->horizontalSlider->setValue(10);//初始值10
            ui->horizontalSlider->setPageStep(5);
            ui->horizontalSlider->setMinimum(0);//最小值0
            ui->horizontalSlider->setMaximum(100);//最大值100
            ui->horizontalSlider->setMaximumWidth(200);//Slider最长200
        }
        QString string="伽马变换，图像矫正:newpixel = 255 × oldpixel ^ "+QString::number(ui->sliderValue->text().toDouble(&ok)/10,10,2);
        ui->lineEdit->setText(string);
        for(int y=0;y<image->height();y++)
        {

            for(int x=0;x<image->width();x++)
            {
                QColor color=image->pixel(x,y);
                int r,g,b;
                //像素变化函数
                r=static_cast<int>(255*qPow(color.red()/255.0,ui->sliderValue->text().toDouble(&ok)/10)) ;
                g=static_cast<int>(255*qPow(color.green()/255.0,ui->sliderValue->text().toDouble(&ok)/10)) ;
                b=static_cast<int>(255*qPow(color.blue()/255.0,ui->sliderValue->text().toDouble(&ok)/10)) ;
                newImage->setPixel(x,y,qRgb(r,g,b));//更新图像的像素

                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==5)//平移变换
    {
        if(times==1)//第一次调用，对Slider进行初始化
        {
            paramsNumbers=2;//设置两个参数有效

            addHorizontalSlider=new QSlider(Qt::Horizontal);

            QSizePolicy policy=addHorizontalSlider->sizePolicy();
            policy.setHorizontalPolicy(QSizePolicy::Expanding);
            policy.setVerticalPolicy(QSizePolicy::Fixed);
            ui->sliderverticalLayout->addWidget(addHorizontalSlider);
            ui->sliderValue->setText("(0,0)");//初始值（0,0）表示不移动

            //x
            addHorizontalSlider->setValue(0);
            addHorizontalSlider->setPageStep(5);
            addHorizontalSlider->setMinimum(-newImage->width());
            addHorizontalSlider->setMaximum(newImage->width());
            addHorizontalSlider->setMaximumWidth(newImage->width());

            //y
            ui->horizontalSlider->setValue(0);//初始值0
            ui->horizontalSlider->setPageStep(5);
            ui->horizontalSlider->setMinimum(-newImage->height());
            ui->horizontalSlider->setMaximum(newImage->height());
            ui->horizontalSlider->setMaximumWidth(newImage->width());

            connect(addHorizontalSlider,&QSlider::valueChanged,this,&GrayTransform::addHorizontalSlider_valueChanged);
        }
        QString string="图像移动，以图片左上角为原点，移动 "+ ui->sliderValue->text();
        ui->lineEdit->setText(string);

        for(int now_y=0;now_y<newImage->height();now_y++)
        {

            for(int now_x=0;now_x<newImage->width();now_x++)
            {
                int old_x,old_y;//now_x,now_y坐标在原图像中的坐标
                old_x=now_x-delta_x;
                old_y=now_y-delta_y;
                if(old_x<0||old_y<0||old_x>=image->width()||old_y>=image->height()) continue;

                QColor color=image->pixel(old_x,old_y);
                int r,g,b;
                r=color.red();
                g=color.green();
                b=color.blue();

                hist[r]++;//对于灰度图，r=g=b

                newImage->setPixel(now_x,now_y,qRgb(r,g,b));//更新图像的像素
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==6)//旋转变换
    {
        if(times==1)//第一次调用，对Slider进行初始化
        {
            //QSlider无法设置float类型，因此将其扩充10倍
            ui->sliderValue->setText("90");
            ui->horizontalSlider->setValue(90);//初始值10
            ui->horizontalSlider->setPageStep(5);
            ui->horizontalSlider->setMinimum(0);//最小值0
            ui->horizontalSlider->setMaximum(360);//最大值100
            ui->horizontalSlider->setMaximumWidth(720);//Slider最长200
        }
        QString string="以图像中心为旋转中心，顺时针旋转 "+ui->sliderValue->text()+"°";
        ui->lineEdit->setText(string);
        int old_width=image->width();//原图像的宽
        int old_height=image->height();//原图像的高
        const double pi=3.1415926;
        double angle=(ui->sliderValue->text().toInt()*2*pi)/360;//旋转角度
        qDebug()<<angle;
        QPoint center(old_width/2,old_height/2);//旋转中心
        //图像坐标系：以图像左上角（0,0）为原点
        //原图像四个角的坐标变为以旋转中心为原点的的坐标系,x向右，y向上
        QPoint left_top(0-center.x(),-0+center.y());
        QPoint left_bottom(0-center.x(),-(old_height-1)+center.y());
        QPoint right_top((old_width-1)-center.x(),-0+center.y());
        QPoint right_bottom((old_width-1)-center.x(),-(old_height-1)+center.y());
        //以center为坐标原点，旋转后四个角的坐标
        //x=x0*cos+y0sin;y=-x0sin+y0cos
        QPointF trans_left_top(left_top.x()*qCos(angle)+left_top.y()*qSin(angle),-left_top.x()*qSin(angle)+left_top.y()*qCos(angle));
        QPointF trans_left_bottom(left_bottom.x()*qCos(angle)+left_bottom.y()*qSin(angle),-left_bottom.x()*qSin(angle)+left_bottom.y()*qCos(angle));
        QPointF trans_right_top(right_top.x()*qCos(angle)+right_top.y()*qSin(angle),-right_top.x()*qSin(angle)+right_top.y()*qCos(angle));
        QPointF trans_right_bottom(right_bottom.x()*qCos(angle)+right_bottom.y()*qSin(angle),-right_bottom.x()*qSin(angle)+right_bottom.y()*qCos(angle));
        //计算旋转后框住图像的大矩形的范围
        double left=fourValueMin(trans_left_bottom.x(),trans_left_top.x(),trans_right_bottom.x(),trans_right_top.x());
        double right=fourValueMax(trans_left_bottom.x(),trans_left_top.x(),trans_right_bottom.x(),trans_right_top.x());
        double top=fourValueMax(trans_left_bottom.y(),trans_left_top.y(),trans_right_bottom.y(),trans_right_top.y());
        double bottom=fourValueMin(trans_left_bottom.y(),trans_left_top.y(),trans_right_bottom.y(),trans_right_top.y());
        //旋转后矩形框的宽、高
        int new_width=static_cast<int>(qAbs(left-right)+0.5);
        int new_height=static_cast<int>(qAbs(top-bottom)+0.5);
        qDebug()<<new_height<<" "<<new_width;
        *newImage=QImage(new_width+1,new_height+1,QImage::Format_ARGB32);
        newImage->fill(qRgb(255,255,255));//以白色填充
        for(int now_y=0;now_y<newImage->height();now_y++)
        {
            for(int now_x=0;now_x<newImage->width();now_x++)
            {
                QPointF old_point=matrixMult(now_x,now_y,left,top,angle,center.x(),center.y());
                //如果越界,则不处理
                if(old_point.x()<0||old_point.y()<0||old_point.x()>=image->width()||old_point.y()>=image->height()) continue;
                //离点old_point最近四点，(lCol,uRow),(rCol,uRow),(lCol,dRow),(rCol,dRow)
                int uRow = static_cast<int>(old_point.y());
                int dRow = uRow + 1;
                int lCol = static_cast<int>(old_point.x());
                int rCol = lCol + 1;

                double r = old_point.y() - uRow;
                double c = old_point.x() - lCol;
                QColor result;
                if(old_point.x()>=image->width()-1&&old_point.y()>=image->height()-1)
                {//坐标在原图的右下角
                    result=image->pixel(image->width()-1,image->height()-1);
                }
                else if(old_point.y()>=image->height()-1)
                {//最后一行
                    QColor color1=image->pixel(lCol,image->height()-1);
                    QColor color2=image->pixel(rCol,image->height()-1);
                    result.setRed(static_cast<int>(color1.red()*c+color2.red()*(1-c)));
                    result.setGreen(static_cast<int>(color1.green()*c+color2.green()*(1-c)));
                    result.setBlue(static_cast<int>(color1.blue()*c+color2.blue()*(1-c)));
                }
                else if(old_point.x()>=image->width()-1)
                {//最后一列
                    QColor color1=image->pixel(image->width()-1,uRow);
                    QColor color2=image->pixel(image->width()-1,dRow);
                    result.setRed(static_cast<int>(color1.red()*r+color2.red()*(1-r)));
                    result.setGreen(static_cast<int>(color1.green()*r+color2.green()*(1-r)));
                    result.setBlue(static_cast<int>(color1.blue()*r+color2.blue()*(1-r)));
                }
                else
                {//其他普通位置
                    QColor lu,ru,ld,rd;
                    lu=image->pixel(lCol,uRow);
                    ru=image->pixel(rCol,uRow);
                    ld=image->pixel(lCol,dRow);
                    rd=image->pixel(rCol,dRow);
                    QColor color1,color2;
                    color1.setRed(static_cast<int>(lu.red()*c+ru.red()*(1-c)));
                    color1.setGreen(static_cast<int>(lu.green()*c+ru.green()*(1-c)));
                    color1.setBlue(static_cast<int>(lu.blue()*c+ru.blue()*(1-c)));

                    color2.setRed(static_cast<int>(ld.red()*c+rd.red()*(1-c)));
                    color2.setGreen(static_cast<int>(ld.green()*c+rd.green()*(1-c)));
                    color2.setBlue(static_cast<int>(ld.blue()*c+rd.blue()*(1-c)));

                    result.setRed(static_cast<int>(color1.red()*r+color2.red()*(1-r)));
                    result.setGreen(static_cast<int>(color1.green()*r+color2.green()*(1-r)));
                    result.setBlue(static_cast<int>(color1.blue()*r+color2.blue()*(1-r)));
                }
                hist[result.red()]++;//对于灰度图，r=g=b

                newImage->setPixel(now_x,now_y,qRgb(result.red(),result.green(),result.blue()));//更新图像的像素
                graysum=graysum+result.red();
                if(hist[result.red()]>maxcount) maxcount=hist[result.red()];
            }
        }
    }
    else if(flag==7)//最邻近差值，缩放
    {
        double hx=1.0,vy=1.0;//水平与垂直缩放系数
        if(times==1)//第一次调用，对Slider进行初始化
        {
            paramsNumbers=2;//设置两个参数有效

            addHorizontalSlider=new QSlider(Qt::Horizontal);

            QSizePolicy policy=addHorizontalSlider->sizePolicy();
            policy.setHorizontalPolicy(QSizePolicy::Expanding);
            policy.setVerticalPolicy(QSizePolicy::Fixed);
            ui->sliderverticalLayout->addWidget(addHorizontalSlider);
            ui->sliderValue->setText("(100,100)");//初始值（0,0）表示水平和垂直方向不缩放
            delta_x=delta_y=100;
            //x
            addHorizontalSlider->setValue(100);//初始值100÷100=1，表示不缩放
            addHorizontalSlider->setPageStep(10);
            addHorizontalSlider->setMinimum(25);//最小缩放系数，25÷100=0.25倍
            addHorizontalSlider->setMaximum(300);//最大缩放系数，300÷100=3倍
            addHorizontalSlider->setMaximumWidth(600);

            //y
            ui->horizontalSlider->setValue(100);//初始值100
            ui->horizontalSlider->setPageStep(10);
            ui->horizontalSlider->setMinimum(25);
            ui->horizontalSlider->setMaximum(300);
            ui->horizontalSlider->setMaximumWidth(600);

            connect(addHorizontalSlider,&QSlider::valueChanged,this,&GrayTransform::addHorizontalSlider_valueChanged);
        }
        QString string="图像缩放，水平缩放系数："+QString::number(delta_x/100.0)+" ,垂直缩放系数："+QString::number(delta_y/100.0);
        ui->lineEdit->setText(string);

        hx=delta_x/100.0;//水平缩放系数
        vy=delta_y/100.0;//垂直缩放系数

        *newImage=newImage->scaled(static_cast<int>(newImage->width()*hx+0.5),static_cast<int>(newImage->height()*vy+0.5));

        for(int now_y=0;now_y<newImage->height();now_y++)
        {
            int old_y=static_cast<int>(now_y/vy+0.5);//now_y坐标在原图像中的坐标
            if(old_y>=image->height()) old_y=image->height()-1;
            for(int now_x=0;now_x<newImage->width();now_x++)
            {
                int old_x=static_cast<int>(now_x/hx+0.5);//now_x坐标在原图像中的坐标

                QColor color=image->pixel(old_x,old_y);

                int r,g,b;
                r=color.red();
                g=color.green();
                b=color.blue();

                hist[r]++;//对于灰度图，r=g=b

                newImage->setPixel(now_x,now_y,qRgb(r,g,b));//更新图像的像素
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==8)//双线性差值，缩放
    {
        double hx=1.0,vy=1.0;//水平与垂直缩放系数
        if(times==1)//第一次调用，对Slider进行初始化
        {
            paramsNumbers=2;//设置两个参数有效

            addHorizontalSlider=new QSlider(Qt::Horizontal);

            QSizePolicy policy=addHorizontalSlider->sizePolicy();
            policy.setHorizontalPolicy(QSizePolicy::Expanding);
            policy.setVerticalPolicy(QSizePolicy::Fixed);
            ui->sliderverticalLayout->addWidget(addHorizontalSlider);
            ui->sliderValue->setText("(100,100)");//初始值（0,0）表示水平和垂直方向不缩放
            delta_x=delta_y=100;
            //x
            addHorizontalSlider->setValue(100);//初始值100÷100=1，表示不缩放
            addHorizontalSlider->setPageStep(10);
            addHorizontalSlider->setMinimum(25);//最小缩放系数，25÷100=0.25倍
            addHorizontalSlider->setMaximum(300);//最大缩放系数，300÷100=3倍
            addHorizontalSlider->setMaximumWidth(600);

            //y
            ui->horizontalSlider->setValue(100);//初始值100
            ui->horizontalSlider->setPageStep(10);
            ui->horizontalSlider->setMinimum(25);
            ui->horizontalSlider->setMaximum(300);
            ui->horizontalSlider->setMaximumWidth(600);

            connect(addHorizontalSlider,&QSlider::valueChanged,this,&GrayTransform::addHorizontalSlider_valueChanged);
        }
        QString string="图像缩放，水平缩放系数："+QString::number(delta_x/100.0)+" ,垂直缩放系数："+QString::number(delta_y/100.0);
        ui->lineEdit->setText(string);

        hx=delta_x/100.0;//水平缩放系数
        vy=delta_y/100.0;//垂直缩放系数

        *newImage=newImage->scaled(static_cast<int>(newImage->width()*hx+0.5),static_cast<int>(newImage->height()*vy+0.5));

        for(int now_y=0;now_y<newImage->height();now_y++)
        {
            for(int now_x=0;now_x<newImage->width();now_x++)
            {
                double col = now_x / hx;
                double row = now_y / vy;
                //离点（col,row）最近四点，(lCol,uRow),(rCol,uRow),(lCol,dRow),(rCol,dRow)
                int uRow = static_cast<int>(row);
                int dRow = uRow + 1;
                int lCol = static_cast<int>(col);
                int rCol = lCol + 1;

                double r = row - uRow;
                double c = col - lCol;
                QColor result;
                if(col>=image->width()-1&&row>=image->height()-1)
                {//坐标在原图的右下角
                    result=image->pixel(image->width()-1,image->height()-1);
                }
                else if(row>=image->height()-1)
                {//最后一行
                    QColor color1=image->pixel(lCol,image->height()-1);
                    QColor color2=image->pixel(rCol,image->height()-1);
                    result.setRed(static_cast<int>(color1.red()*c+color2.red()*(1-c)));
                    result.setGreen(static_cast<int>(color1.green()*c+color2.green()*(1-c)));
                    result.setBlue(static_cast<int>(color1.blue()*c+color2.blue()*(1-c)));
                }
                else if(col>=image->width()-1)
                {//最后一列
                    QColor color1=image->pixel(image->width()-1,uRow);
                    QColor color2=image->pixel(image->width()-1,dRow);
                    result.setRed(static_cast<int>(color1.red()*r+color2.red()*(1-r)));
                    result.setGreen(static_cast<int>(color1.green()*r+color2.green()*(1-r)));
                    result.setBlue(static_cast<int>(color1.blue()*r+color2.blue()*(1-r)));
                }
                else
                {//其他普通位置
                    QColor lu,ru,ld,rd;
                    lu=image->pixel(lCol,uRow);
                    ru=image->pixel(rCol,uRow);
                    ld=image->pixel(lCol,dRow);
                    rd=image->pixel(rCol,dRow);
                    QColor color1,color2;
                    color1.setRed(static_cast<int>(lu.red()*c+ru.red()*(1-c)));
                    color1.setGreen(static_cast<int>(lu.green()*c+ru.green()*(1-c)));
                    color1.setBlue(static_cast<int>(lu.blue()*c+ru.blue()*(1-c)));

                    color2.setRed(static_cast<int>(ld.red()*c+rd.red()*(1-c)));
                    color2.setGreen(static_cast<int>(ld.green()*c+rd.green()*(1-c)));
                    color2.setBlue(static_cast<int>(ld.blue()*c+rd.blue()*(1-c)));

                    result.setRed(static_cast<int>(color1.red()*r+color2.red()*(1-r)));
                    result.setGreen(static_cast<int>(color1.green()*r+color2.green()*(1-r)));
                    result.setBlue(static_cast<int>(color1.blue()*r+color2.blue()*(1-r)));
                }
                hist[result.red()]++;//对于灰度图，r=g=b

                newImage->setPixel(now_x,now_y,qRgb(result.red(),result.green(),result.blue()));//更新图像的像素
                graysum=graysum+result.red();
                if(hist[result.red()]>maxcount) maxcount=hist[result.red()];
            }
        }

    }
    else if(flag==9)//水平镜像
    {
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        QString string="水平镜像";
        ui->lineEdit->setText(string);
        for(int now_y=0;now_y<newImage->height();now_y++)
        {

            for(int now_x=0;now_x<newImage->width();now_x++)
            {
                int old_x,old_y;//now_x,now_y坐标在原图像中的坐标
                old_x=image->width()-now_x-1;
                old_y=now_y;

                QColor color=image->pixel(old_x,old_y);
                int r,g,b;
                r=color.red();
                g=color.green();
                b=color.blue();

                hist[r]++;//对于灰度图，r=g=b

                newImage->setPixel(now_x,now_y,qRgb(r,g,b));//更新图像的像素
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==10)//垂直镜像
    {
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        QString string="垂直镜像";
        ui->lineEdit->setText(string);
        for(int now_y=0;now_y<newImage->height();now_y++)
        {

            for(int now_x=0;now_x<newImage->width();now_x++)
            {
                int old_x,old_y;//now_x,now_y坐标在原图像中的坐标
                old_x=now_x;
                old_y=image->height()-now_y-1;

                QColor color=image->pixel(old_x,old_y);
                int r,g,b;
                r=color.red();
                g=color.green();
                b=color.blue();
                hist[r]++;//对于灰度图，r=g=b

                newImage->setPixel(now_x,now_y,qRgb(r,g,b));//更新图像的像素
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==11||flag==12)//3×3均值滤波或5×5均值滤波
    {
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        *newImage=QImage(origin.copy());
        int kernelSize;
        int sumKernel;
        if(flag==11) //3×3全为1的矩阵
        {
            kernelSize=3;
            sumKernel=9;
            QString string="3×3邻域均值滤波";
            ui->lineEdit->setText(string);
        }
        else //5×5全为1的矩阵
        {
            kernelSize=5;
            sumKernel=25;
            QString string="5×5邻域均值滤波";
            ui->lineEdit->setText(string);
        }
        int r,g,b;
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)//采用边界不处理的原则
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                r=0;
                g=0;
                b=0;
                for(int i=-kernelSize/2;i<=kernelSize/2;i++)
                {
                    for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                    {
                        QColor color = image->pixel(x+i, y+j);//原图的像素rgb
                        r+=color.red()*1;
                        g+=color.green()*1;
                        b+=color.blue()*1;
                    }
                }
                r=qBound(0,r/sumKernel,255);
                g=qBound(0,g/sumKernel,255);
                b=qBound(0,b/sumKernel,255);
                newImage->setPixel(x,y,qRgb(r,g,b));

                //未计算边界
                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==13)//3×3窗口中值滤波
    {
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        *newImage=QImage(origin.copy());
        int kernelSize;
        kernelSize=5;
        QString string="3×3邻域中值滤波";
        ui->lineEdit->setText(string);
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)//采用边界不处理的原则
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                QVector<int>r_arr;
                QVector<int>g_arr;
                QVector<int>b_arr;
                for(int i=-kernelSize/2;i<=kernelSize/2;i++)
                {
                    for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                    {
                        QColor color = image->pixel(x+i, y+j);//原图的像素rgb
                        r_arr.push_back(color.red());
                        g_arr.push_back(color.green());
                        b_arr.push_back(color.blue());
                    }
                }
                std::sort(r_arr.begin(),r_arr.end());
                std::sort(g_arr.begin(),g_arr.end());
                std::sort(b_arr.begin(),b_arr.end());
                int num=r_arr.size();
                newImage->setPixel(x,y,qRgb(r_arr[num/2],g_arr[num/2],b_arr[num/2]));
                //未计算边界
                hist[r_arr[4]]++;//对于灰度图，r=g=b
                graysum=graysum+r_arr[4];
                if(hist[r_arr[4]]>maxcount) maxcount=hist[r_arr[4]];
            }
        }
    }
    else if(flag==14||flag==15||flag==16)//3×3、5×5、5×5高斯滤波
    {
        if(times==1)//第一次调用，对Slider进行初始化
        {
            //QSlider无法设置float类型，因此将其扩充10倍
            ui->sliderValue->setText("10");
            ui->horizontalSlider->setValue(10);//初始值8
            ui->horizontalSlider->setPageStep(2);
            ui->horizontalSlider->setMinimum(0);//最小值0
            ui->horizontalSlider->setMaximum(100);//最大值20,也就是2
            ui->horizontalSlider->setMaximumWidth(400);//Slider最长80
        }
        double sigma2=(delta_y/10.0)*(delta_y/10.0);
        int kernelSize;
        QString string;
        if(flag==14)
        {
            string="3×3高斯滤波器，sigma(x)^2=sigma(y)^2 = "+QString::number(sigma2);
            kernelSize=3;
        }
        else if(flag==15)
        {
            string="5×5高斯滤波器，sigma(x)^2=sigma(y)^2 = "+QString::number(sigma2);
            kernelSize=5;
        }
        else {
            string="7×7高斯滤波器，sigma(x)^2=sigma(y)^2 = "+QString::number(sigma2);
            kernelSize=7;
        }
        ui->lineEdit->setText(string);
        double kernel[8][8]={{0.0}};
        getGaussianTemplate(kernelSize,kernel,sigma2);
        double r,g,b;
        QColor color;
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                r=0.0;
                g=0.0;
                b=0.0;
                for(int i=-kernelSize/2;i<=kernelSize/2;i++)
                {
                    for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                    {
                        color = image->pixel(x+i, y+j);//原图的像素rgb
                        r+=color.red()*kernel[kernelSize/2+i][kernelSize/2+j];
                        g+=color.green()*kernel[kernelSize/2+i][kernelSize/2+j];
                        b+=color.blue()*kernel[kernelSize/2+i][kernelSize/2+j];
                    }
                }
                r=qBound(0,static_cast<int>(r),255);
                g=qBound(0,static_cast<int>(g),255);
                b=qBound(0,static_cast<int>(b),255);
                newImage->setPixel(x,y,qRgb(static_cast<int>(r),static_cast<int>(g),static_cast<int>(b)));

                //未计算边界
                hist[static_cast<int>(r)]++;//对于灰度图，r=g=b
                graysum=graysum+static_cast<int>(r);
                if(hist[static_cast<int>(r)]>maxcount) maxcount=hist[static_cast<int>(r)];
            }
        }
    }
    else if(flag==17||flag==18||flag==19)//3×3、5×5、5×5双边滤波
    {
        if(times==1)//第一次调用，对Slider进行初始化
        {
            //QSlider无法设置float类型，因此将其扩充10倍
            ui->sliderValue->setText("10");
            ui->horizontalSlider->setValue(10);//初始值8
            ui->horizontalSlider->setPageStep(2);
            ui->horizontalSlider->setMinimum(0);//最小值0
            ui->horizontalSlider->setMaximum(100);//最大值100,也就是10
            ui->horizontalSlider->setMaximumWidth(400);//Slider最长80
        }
        double sigma2=(delta_y/10.0)*(delta_y/10.0);
        int kernelSize;
        QString string;
        if(flag==17)
        {
            string="3×3双边滤波器，sigma(s)^2=sigma(r)^2 = "+QString::number(sigma2);
            kernelSize=3;
        }
        else if(flag==18)
        {
            string="5×5双边滤波器，sigma(s)^2=sigma(r)^2 = "+QString::number(sigma2);
            kernelSize=5;
        }
        else {
            string="7×7双边滤波器，sigma(s)^2=sigma(r)^2 = "+QString::number(sigma2);
            kernelSize=7;
        }
        ui->lineEdit->setText(string);
        double kernel_red[8][8]={{0.0}};
        double kernel_green[8][8]={{0.0}};
        double kernel_blue[8][8]={{0.0}};
        double r,g,b;
        QColor color;
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                r=0.0;
                g=0.0;
                b=0.0;
                getBilateralFiltering(x,y,kernelSize,kernel_red,kernel_green,kernel_blue,sigma2);
                for(int i=-kernelSize/2;i<=kernelSize/2;i++)
                {
                    for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                    {
                        color = image->pixel(x+i, y+j);//原图的像素rgb
                        r+=color.red()*kernel_red[kernelSize/2+i][kernelSize/2+j];
                        g+=color.green()*kernel_green[kernelSize/2+i][kernelSize/2+j];
                        b+=color.blue()*kernel_blue[kernelSize/2+i][kernelSize/2+j];
                    }
                }
                r=qBound(0,static_cast<int>(r),255);
                g=qBound(0,static_cast<int>(g),255);
                b=qBound(0,static_cast<int>(b),255);
                newImage->setPixel(x,y,qRgb(static_cast<int>(r),static_cast<int>(g),static_cast<int>(b)));

                //未计算边界
                hist[static_cast<int>(r)]++;//对于灰度图，r=g=b
                graysum=graysum+static_cast<int>(r);
                if(hist[static_cast<int>(r)]>maxcount) maxcount=hist[static_cast<int>(r)];
            }
        }
    }
    else if(flag==20||flag==21)//四邻域、八邻域拉普拉斯锐化
    {
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        *newImage=QImage(origin.copy());
        int kernelSize=3;
        QString string="四邻域拉普拉斯算子锐化";
        int kernel[3][3]=
        {//四邻域  如果为中间值为5，则生成的图像有灰度信息
            {0,-1,0},
            {-1,4,-1},
            {0,-1,0},
        };
        if(flag==21)
        {//八邻域  如果为中间值为9，则生成的图像有灰度信息
            kernel[0][0]=-1;kernel[0][2]=-1;
            kernel[1][0]=-1;kernel[1][1]=8;kernel[1][2]=-1;
            kernel[2][0]=-1;kernel[2][2]=-1;
            string="八邻域拉普拉斯算子锐化";
        }
        ui->lineEdit->setText(string);
        int r,g,b;
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)//采用边界不处理的原则
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                r=0;
                g=0;
                b=0;
                for(int i=-kernelSize/2;i<=kernelSize/2;i++)
                {
                    for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                    {
                        QColor color = image->pixel(x+i, y+j);//原图的像素rgb
                        r+=color.red()*kernel[kernelSize/2+i][kernelSize/2+j];
                        g+=color.green()*kernel[kernelSize/2+i][kernelSize/2+j];
                        b+=color.blue()*kernel[kernelSize/2+i][kernelSize/2+j];
                    }
                }
                r=qBound(0,r,255);
                g=qBound(0,g,255);
                b=qBound(0,b,255);
                newImage->setPixel(x,y,qRgb(r,g,b));
                //未计算边界
                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==22)//Sobel算子梯度锐化
    {
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        *newImage=QImage(origin.copy());
        int kernelSize=3;
        QString string="Sobel算子梯度锐化";
        int kernel_1[3][3]=
        {//垂直边缘
            {1,0,-1},
            {2,0,-2},
            {1,0,-1},
        };
        int kernel_2[3][3]=
        {//水平边缘
            {-1,-2,-1},
            {0,0,0},
            {1,2,1},
        };
        ui->lineEdit->setText(string);
        int r1,g1,b1;//垂直边缘
        int r2,g2,b2;//水平边缘
        int r,g,b;//垂直+水平
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)//采用边界不处理的原则
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                r=r1=r2=0;
                g=g1=g2=0;
                b=b1=b2=0;
                for(int i=-kernelSize/2;i<=kernelSize/2;i++)
                {
                    for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                    {
                        QColor color = image->pixel(x+i, y+j);//原图的像素rgb
                        r1+=color.red()*kernel_1[kernelSize/2+i][kernelSize/2+j];
                        g1+=color.green()*kernel_1[kernelSize/2+i][kernelSize/2+j];
                        b1+=color.blue()*kernel_1[kernelSize/2+i][kernelSize/2+j];

                        r2+=color.red()*kernel_2[kernelSize/2+i][kernelSize/2+j];
                        g2+=color.green()*kernel_2[kernelSize/2+i][kernelSize/2+j];
                        b2+=color.blue()*kernel_2[kernelSize/2+i][kernelSize/2+j];
                    }
                }
                r=qBound(0,qAbs(r1)+qAbs(r2),255);//求绝对值再相加
                g=qBound(0,qAbs(g1)+qAbs(g2),255);
                b=qBound(0,qAbs(b1)+qAbs(b2),255);
                newImage->setPixel(x,y,qRgb(r,g,b));
                //未计算边界
                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==23)//Priwitt算子锐化
    {
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        *newImage=QImage(origin.copy());
        int kernelSize=3;
        QString string="Priwitt算子锐化";
        int kernel_1[3][3]=
        {//垂直边缘
            {1,-1,-1},
            {0,0,0},
            {1,1,1},
        };
        int kernel_2[3][3]=
        {//水平边缘
            {-1,0,1},
            {-1,0,1},
            {-1,0,1},
        };
        ui->lineEdit->setText(string);
        int r1,g1,b1;//垂直边缘
        int r2,g2,b2;//水平边缘
        int r,g,b;//垂直+水平
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)//采用边界不处理的原则
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                r=r1=r2=0;
                g=g1=g2=0;
                b=b1=b2=0;
                for(int i=-kernelSize/2;i<=kernelSize/2;i++)
                {
                    for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                    {
                        QColor color = image->pixel(x+i, y+j);//原图的像素rgb
                        r1+=color.red()*kernel_1[kernelSize/2+i][kernelSize/2+j];
                        g1+=color.green()*kernel_1[kernelSize/2+i][kernelSize/2+j];
                        b1+=color.blue()*kernel_1[kernelSize/2+i][kernelSize/2+j];

                        r2+=color.red()*kernel_2[kernelSize/2+i][kernelSize/2+j];
                        g2+=color.green()*kernel_2[kernelSize/2+i][kernelSize/2+j];
                        b2+=color.blue()*kernel_2[kernelSize/2+i][kernelSize/2+j];
                    }
                }
                r=qBound(0,qAbs(r1)+qAbs(r2),255);//求绝对值再相加
                g=qBound(0,qAbs(g1)+qAbs(g2),255);
                b=qBound(0,qAbs(b1)+qAbs(b2),255);
                newImage->setPixel(x,y,qRgb(r,g,b));
                //未计算边界
                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==24)//wallis锐化
    {
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        *newImage=QImage(origin.copy());
        QString string="wallis锐化";
        ui->lineEdit->setText(string);
        int r,g,b;//垂直+水平
        int kernelSize=3;
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)//采用边界不处理的原则
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                r=0;
                g=0;
                b=0;
                QColor color1 = image->pixel(x, y);//原图的像素rgb
                QColor color2=image->pixel(x,y-1);
                QColor color3=image->pixel(x,y+1);
                QColor color4=image->pixel(x-1,y);
                QColor color5=image->pixel(x+1,y);
                //46太大，锐化后的图片过亮，此处使用40
                r=static_cast<int>(40*(5*qLn(color1.red()+1)-(qLn(color4.red()+1)+qLn(color5.red()+1)+qLn(color2.red()+1)+qLn(color3.red()+1)))) ;
                g=static_cast<int>(40*(5*qLn(color1.green()+1)-(qLn(color4.green()+1)+qLn(color5.green()+1)+qLn(color2.green()+1)+qLn(color3.green()+1)))) ;
                b=static_cast<int>(40*(5*qLn(color1.blue()+1)-(qLn(color4.blue()+1)+qLn(color5.blue()+1)+qLn(color2.blue()+1)+qLn(color3.blue()+1)))) ;

                r=qBound(0,r,255);//求绝对值再相加
                g=qBound(0,g,255);
                b=qBound(0,b,255);
                newImage->setPixel(x,y,qRgb(r,g,b));
                //未计算边界
                hist[r]++;//对于灰度图，r=g=b
                graysum=graysum+r;
                if(hist[r]>maxcount) maxcount=hist[r];
            }
        }
    }
    else if(flag==25)//kirsch边缘检测
    {
        //所处理的图片为灰度图
        ui->sliderValue->setVisible(false);
        ui->horizontalSlider->setVisible(false);
        ui->pushButton->setVisible(false);
        *newImage=QImage(origin.copy());
        QString string="kirsch边缘检测";
        ui->lineEdit->setText(string);
        int a[8][3][3]={{{+5,+5,+5},{-3,0,-3},{-3,-3,-3},},
                       {{-3,+5,+5},{-3,0,+5},{-3,-3,-3}},
                       {{-3,-3,+5},{-3,0,+5},{-3,-3,+5}},
                       {{-3,-3,-3},{-3,0,+5},{-3,+5,+5}},
                       {{-3,-3,-3},{+5,0,-3},{+5,+5,-3}},
                       {{+5,-3,-3},{+5,0,-3},{+5,-3,-3}},
                       {{+5,+5,-3},{+5,0,-3},{-3,-3,-3}},
                       {{-3,-3,-3},{-3,0,-3},{+5,+5,+5}}};

        int kernelSize=3;
        for(int x=kernelSize/2;x<newImage->width()-(kernelSize/2);x++)//采用边界不处理的原则
        {
            for(int y=kernelSize/2;y<newImage->height()-(kernelSize/2);y++)
            {
                int gray[8]={0};
                for(int i=-kernelSize/2;i<=kernelSize/2;i++)
                {
                    for(int j=-kernelSize/2;j<=kernelSize/2;j++)
                    {
                        QColor color = image->pixel(x+i, y+j);//原图的像素rgb
                        for(int k=0;k<8;k++)
                        {
                            gray[k]+=color.red()*a[k][kernelSize/2+i][kernelSize/2+j];
                        }
                    }
                }
                getOptimalgrayValue(gray);//将数组gray中的最大值赋予gray[0]
                gray[0]=qBound(0,gray[0],255);//求绝对值再相加
                newImage->setPixel(x,y,qRgb(gray[0],gray[0],gray[0]));
                //未计算边界
                hist[gray[0]]++;//对于灰度图，r=g=b
                graysum=graysum+gray[0];
                if(hist[gray[0]]>maxcount) maxcount=hist[gray[0]];
            }
        }
    }
    //处理并显示变换后的图像与直方图数据
    averageGray=static_cast<double>(graysum*1.0/pixelnum);//计算平均灰度
    for(int i=0,temp=0;i<256;i++)
    {
        if(temp<=pixelnum/2)
        {
            temp=temp+hist[i];
            midgray=i;
        }
        deviation=deviation+hist[i]*((averageGray-i)*(averageGray-i));
    }
    standardDeviation=sqrt(deviation/pixelnum);//计算标准差
    ui->averageGrayLabel->setText(QString::number(averageGray,10,2));//输出平均值
    ui->midgrayLabel->setText(QString::number(midgray,10));//输出中值
    ui->pixelnumLabel->setText(QString::number(pixelnum,10));//输出像素总数
    ui->standardDeviationLabel->setText(QString::number(standardDeviation,10,2));//输出标准差

    ui->label->resize(newImage->width(),newImage->height());
    ui->label->setPixmap(QPixmap::fromImage(*newImage));
}
void GrayTransform::paintEvent(QPaintEvent *event)//显示直方图,绘制事件不需要显示调用
{
    Q_UNUSED(event);
    if(!is_gray)
    {
        ui->label_4->setText(tr("平均像素值:"));
        ui->label_6->setText(tr("中值像素值:"));
        ui->gray_histogram->setWindowTitle("红色分量直方图");
    }
    QPainter p(this);
    p.translate(ui->gray_histogram->pos().x()+ui->histogramWidget->pos().x(),ui->gray_histogram->pos().y()+ui->histogramWidget->pos().y());//坐标转换，将坐标原点转移到此处位置
    p.drawLine(0,0,0,ui->histogramWidget->height());//纵轴
    p.drawLine(0,ui->histogramWidget->height(),ui->histogramWidget->width(),ui->histogramWidget->height());//横轴
    for(int i=0;i<256;i++)
    {
        int x=i;
        int y=hist[i];
        if(maxcount==0)return;//防止因为maxcount=0导致程序崩溃
        y=(y*ui->histogramWidget->height())/maxcount;
        p.drawLine(x,ui->histogramWidget->height()-y,x,ui->histogramWidget->height());
        if(i % 32 == 0||i==255)
        {
             p.drawText(QPoint(i,ui->histogramWidget->height()+15),QString::number(i));
        }
    }
}

void GrayTransform::on_pushButton_clicked()
{
    GrayTransDisplay(*image,flag,2,is_gray);//图像值传递,2表示不是第一次调用，不需要进行初始化
    this->update();//更新绘制事件
}

void GrayTransform::on_horizontalSlider_valueChanged(int value)
{
    delta_y=value;
    if(paramsNumbers==2)//两个参数有效
    {
        ui->sliderValue->setText("("+QString::number(delta_x,10)+","+QString::number(delta_y,10)+")");
    }
    else
    {
        ui->sliderValue->setText(QString::number(delta_y,10));
    }
}

void GrayTransform::addHorizontalSlider_valueChanged(int value)
{
    delta_x=value;
    ui->sliderValue->setText("("+QString::number(delta_x,10)+","+QString::number(delta_y,10)+")");
}

void GrayTransform::getGaussianTemplate(int size, double kernel[8][8],double sigma2)//获取size×size大小的高斯模板
{
    //坐标（x,y）是相对核的中心的位置，x正方向向右，y正方向向上
    int k=size/2;//模板中心位置
    int x2,y2;
    double sum=0.0;
    for(int x=0;x<size;x++)
    {
        x2=(x-k)*(x-k);
        for(int y=0;y<size;y++)
        {
            y2=(y-k)*(y-k);
            sum+=kernel[x][y]=exp(-(x2+y2) / (2 * sigma2));
        }
    }
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            kernel[i][j]/=sum;
        }
    }
}

void GrayTransform::getBilateralFiltering(int x,int y,int size, double kernel_red[8][8],double kernel_green[8][8],double kernel_blue[8][8], double sigma2)//获取size×size大小的双边滤波模板
{
    //坐标（x,y）是相对核的中心的位置，x正方向向右，y正方向向上
    int k=size/2;//模板中心位置
    double sum_red=0.0;
    double sum_green=0.0;
    double sum_blue=0.0;
    QColor center=image->pixel(x,y);//模板中心的像素值
    getSpaceWidget(size,kernel_red,sigma2);//获取距离权重
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            kernel_green[i][j]=kernel_blue[i][j]=kernel_red[i][j];
        }
    }
    for(int j=0;j<size;j++)
    {
        for(int i=0;i<size;i++)
        {
            QColor color=image->pixel(x+(i-k),y+(j-k));
            kernel_red[j][i]*=exp(-((center.red()-color.red())*(center.red()-color.red())) / (2 * sigma2));
            sum_red+=kernel_red[j][i];
            kernel_green[j][i]*=exp(-((center.green()-color.green())*(center.green()-color.green())) / (2 * sigma2));
            sum_green+=kernel_green[j][i];
            kernel_blue[j][i]*=exp(-((center.blue()-color.blue())*(center.blue()-color.blue())) / (2 * sigma2));
            sum_blue+=kernel_blue[j][i];
        }
    }
    for(int i=0;i<size;i++)
    {
        for(int j=0;j<size;j++)
        {
            kernel_red[i][j]/=sum_red;
            kernel_green[i][j]/=sum_green;
            kernel_blue[i][j]/=sum_blue;
        }
    }
}

void GrayTransform::getSpaceWidget(int size, double kernel[8][8], double sigma2)//获取距离权重
{
    //坐标（x,y）是相对核的中心的位置，x正方向向右，y正方向向上
    int k=size/2;//模板中心位置
    int x2,y2;
    for(int x=0;x<size;x++)
    {
        x2=(x-k)*(x-k);
        for(int y=0;y<size;y++)
        {
            y2=(y-k)*(y-k);
            kernel[x][y]=exp(-(x2+y2) / (2 * sigma2));
        }
    }
}

void GrayTransform::getOptimalgrayValue(int gray[8])//取得kirsch边缘检测八个模板中的最大值，赋值给gray[0]
{
    for(int i=1;i<8;i++)
    {
        if(gray[0]<gray[i]) gray[0]=gray[i];
    }
}

Mat GrayTransform::QImage2cvMat(const QImage &inImage, bool inCloneImageData)
{
      switch ( inImage.format() )
      {
         // 8-bit, 4 channel
         case QImage::Format_ARGB32:
         case QImage::Format_ARGB32_Premultiplied:
         {
            cv::Mat  mat( inImage.height(), inImage.width(),
                          CV_8UC4,
                          const_cast<uchar*>(inImage.bits()),
                          static_cast<size_t>(inImage.bytesPerLine())
                          );

            return (inCloneImageData ? mat.clone() : mat);
         }
         // 8-bit, 3 channel
         case QImage::Format_RGB32:
         {
            if ( !inCloneImageData )
            {
               qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
            }

            cv::Mat  mat( inImage.height(), inImage.width(),
                          CV_8UC4,
                          const_cast<uchar*>(inImage.bits()),
                          static_cast<size_t>(inImage.bytesPerLine())
                          );

            cv::Mat  matNoAlpha;

            cv::cvtColor( mat, matNoAlpha, cv::COLOR_BGRA2BGR );   // drop the all-white alpha channel

            return matNoAlpha;
         }
         // 8-bit, 3 channel
         case QImage::Format_RGB888:
         {
            if ( !inCloneImageData )
            {
               qWarning() << "QImageToCvMat() - Conversion requires cloning so we don't modify the original QImage data";
            }

            QImage   swapped = inImage.rgbSwapped();

            return cv::Mat( swapped.height(), swapped.width(),
                            CV_8UC3,
                            const_cast<uchar*>(swapped.bits()),
                            static_cast<size_t>(swapped.bytesPerLine())
                            ).clone();
         }

         // 8-bit, 1 channel
         case QImage::Format_Indexed8:
         {
            cv::Mat  mat( inImage.height(), inImage.width(),
                          CV_8UC1,
                          const_cast<uchar*>(inImage.bits()),
                          static_cast<size_t>(inImage.bytesPerLine())
                          );

            return (inCloneImageData ? mat.clone() : mat);
         }
         default:
            qWarning() << "QImage2CvMat() - QImage format not handled in switch:" << inImage.format();
            break;
      }
      return cv::Mat();
}

QImage GrayTransform::cvMat2QImage( const cv::Mat &inMat )
{
      switch ( inMat.type() )
      {
         // 8-bit, 4 channel
         case CV_8UC4:
         {
            QImage image( inMat.data,inMat.cols, inMat.rows,static_cast<int>(inMat.step),QImage::Format_ARGB32 );
            return image;
         }
         // 8-bit, 3 channel
         case CV_8UC3:
         {
            QImage image( inMat.data,inMat.cols, inMat.rows,static_cast<int>(inMat.step),QImage::Format_RGB888 );
            return image.rgbSwapped();
         }
         // 8-bit, 1 channel
         case CV_8UC1:
         {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
            QImage image( inMat.data,inMat.cols, inMat.rows,static_cast<int>(inMat.step),QImage::Format_Grayscale8 );
#else
            static QVector<QRgb>  sColorTable;

            // only create our color table the first time
            if ( sColorTable.isEmpty() )
            {
               sColorTable.resize( 256 );

               for ( int i = 0; i < 256; ++i )
               {
                  sColorTable[i] = qRgb( i, i, i );
               }
            }

            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_Indexed8 );

            image.setColorTable( sColorTable );
#endif
            return image;
         }
         default:
            qWarning() << "cvMat2QImage() - cv::Mat image type not handled in switch:" << inMat.type();
            break;
      }
      return QImage();
}

double GrayTransform::fourValueMin(double a, double b, double c, double d)
{
    return qMin(qMin(a,b),qMin(c,d));
}

double GrayTransform::fourValueMax(double a, double b, double c, double d)
{
    return  qMax(qMax(a,b),qMax(c,d));
}

QPointF GrayTransform::matrixMult(int nowx, int nowy, double left, double top, double angle, int x0, int y0)
{
    /*设在原未旋转的图像中的坐标为(x,y),设在新的矩形框中的坐标为(x’,y’)，原图坐标系下中心点的坐标(x0,y0)，旋转角a，矩形框左上角在以原图像中心为原点的坐标系中的坐标为(left,top)，left的值为负
     [x,y,1]=[x',y',1]*[1,  0,0]*[ cosa,sina, 0]*[ 1, 0, 0]
                        0, -1,0   -sina,cosa, 0    0,-1, 0
                     left,top,1       0,   0, 1   x0,y0, 1
*/
    double x1=nowx+left;
    double y1=-nowy+top;
    double x2=x1*qCos(angle)-y1*qSin(angle);
    double y2=x1*qSin(angle)+y1*qCos(angle);
    double x=x2+x0;
    double y=-y2+y0;
    return QPointF(x,y);
}
