#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_QuitOnClose,true);//设置此主窗口关闭后，其他窗口全部关闭
    //this->setWindowState(Qt::WindowMaximized);   窗口初始化时，最大化显示
    image=new QImage;
    newImage=new QImage;
    is_gray=false;
    is_gauss=false;
    is_canny=false;
    image_isopen=false;

    this->setWindowTitle(tr("图像处理"));
    //打开最开始的图片时，显示的是红色分量直方图
    ui->label_4->setText(tr("平均像素值:"));
    ui->label_6->setText(tr("中值像素值:"));
    ui->gray_histogram->setWindowTitle("红色分量直方图");

    ui->dockWidget_3->setWindowTitle("待定");
    ui->dockWidget_4->setWindowTitle("待定");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::windowShow(QString filename)
{
    this->show();
    openImageFile(filename);
    image_isopen=true;
}

void MainWindow::on_openFileBtn_clicked()//点击界面上的打开按钮，
{
    QString filename=QFileDialog::getOpenFileName(this,tr("打开图片"),".","image Files(*.jpg *.png *.jpeg *.bmp)");
    if(filename.isEmpty())
        return;
    else
    {
        if(openImageFile(filename))
        {//初始化
            is_gray=false;
            is_canny=false;
            is_gauss=false;
            hough_space.clear();
            image_isopen=true;
        }
    }
}

void MainWindow::on_saveAsBtn_clicked()
{
    if(!image_isopen)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先打开一张图片！再进行该项操作！"));
        return;
    }
    QString filename1 = QFileDialog::getSaveFileName(this,tr("Save Image"),".",tr("所有文件 (*)")); //选择路径
    if(newImage->save(filename1))//第二个参数表示若没有指明保存的格式，会根据扩展名来自动辨别，第三个参数表示图片质量，默认为-1
    {
        QMessageBox::information(this,tr("提示！"),tr("保存成功！"));
    }
    else
    {
        QMessageBox::information(this,tr("提示！"),tr("保存失败！"));
    }
}

void MainWindow::on_transformBtn_clicked()//点击按钮后，显示灰度图，同时更新灰度直方图
{
    if(!image_isopen)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先打开一张图片！再进行该项操作！"));
        return;
    }
    if(is_gray==true) return;

    ui->label_4->setText(tr("平均灰度值:"));
    ui->label_6->setText(tr("中值灰度值:"));
    ui->gray_histogram->setWindowTitle("灰度直方图");

    newImage=greyScale(*newImage);//灰度图转换

    ui->label->setPixmap(QPixmap::fromImage(*newImage));//显示灰度图
    is_gray=true;
    this->update();//更新，显示灰度直方图
}

void MainWindow::on_sureBtn_clicked()//改变采样率和量化等级
{
    if(!image_isopen)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先打开一张图片！再进行该项操作！"));
        return;
    }

    QString string1=ui->comboBox1->currentText();
    QString string2=ui->comboBox2->currentText();

    int sampsize=0;//采样的大小，每sampsize个像素里面去掉一个像素点
    int quantificationsize=256;//量化等级

    bool ok=true;
    sampsize=string1.toInt(&ok,10);//采样系数，每sampsize个像素点，取一个有效点
    quantificationsize=string2.toInt(&ok,10);

    samp=new Samp;
    samp->setAttribute(Qt::WA_QuitOnClose,false);
    samp->changSamp(sampsize,quantificationsize,*newImage);
    samp->show();

}

bool MainWindow::openImageFile(QString filename)//打开图片
{
    if(!(image->load(filename))) //加载图像
    {
        QMessageBox::information(this, tr("打开图像失败"),tr("打开图像失败!"));
        return false;
    }
    QImageReader imgReader(filename);
    imgReader.read(image);
    *image=image->convertToFormat(QImage::Format_ARGB32);
    ui->label->resize(image->width(),image->height());
    ui->label->setPixmap(QPixmap::fromImage(*image));
    *newImage=image->copy();//深拷贝
    //newImage=image;   /*浅拷贝*/
    return true;
}

QImage * MainWindow::greyScale(QImage &origin)
{
    QImage *nowImage=new QImage(origin.width(),origin.height(),QImage::Format_ARGB32);
    //QVector<int>data(256,0);
    for(int y=0;y<nowImage->height();y++)
    {
        for(int x=0;x<nowImage->width();x++)
        {
            QColor color=origin.pixel(x,y);
            int gray=static_cast<int>(0.299*color.red()+0.587*color.green()+0.114*color.blue());
            nowImage->setPixel(x,y,qRgb(gray,gray,gray));
            //data[gray]++;
        }
    }
    return nowImage;
}

void MainWindow::paintEvent(QPaintEvent *event)//显示灰度直方图,绘制事件不需要显示调用
{
    Q_UNUSED(event);
    //获取图像的灰度分布数组
    if(!image_isopen) return;//未打开图片，直接返回
    QVector <int>hist(256);
    int pixelnum=newImage->width()*newImage->height();
    int maxcount=0;//记录灰度值最多的像素的个数
    int graysum=0;//记录灰度值的和
    int midgray=0;//记录中值灰度的值
    double averageGray=0;//记录平均灰度值
    double standardDeviation=0;//记录灰度标准差
    double deviation=0;//方差的pixelnum倍
    for(int y=0;y<newImage->height();y++)
    {
        QRgb *line=reinterpret_cast<QRgb *>(newImage->scanLine(y));//C++中强制类型转换的方法
        //QRgb *line=(QRgb *)midImage->scanLine(y);
        for(int x=0;x<newImage->width();x++)
        {
            hist[qRed(line[x])]++;
            graysum=graysum+qRed(line[x]);
            if(hist[qRed(line[x])]>maxcount) maxcount=hist[qRed(line[x])];
        }
    }
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
    standardDeviation=sqrt(deviation/pixelnum);//计算灰度标准差
    ui->averageGrayLabel->setText(QString::number( averageGray,10,2));//输出平均灰度值
    ui->midgrayLabel->setText(QString::number(midgray,10));//输出中值灰度值
    ui->pixelnumLabel->setText(QString::number(pixelnum,10));//输出像素总数
    ui->standardDeviationLabel->setText(QString::number(standardDeviation,10,2));//输出灰度标准差

    QPainter p(this);
    p.translate(ui->gray_histogram->pos().x()+ui->histogramWidget->pos().x(),ui->gray_histogram->pos().y()+ui->histogramWidget->pos().y());//坐标转换，将坐标原点转移到此处位置
    p.drawLine(0,0,0,ui->histogramWidget->height());//纵轴
    p.drawLine(0,ui->histogramWidget->height(),ui->histogramWidget->width(),ui->histogramWidget->height());//横轴
    for(int i=0;i<256;i++)
    {
        int x=i;
        int y=hist[i];
        y=(y*ui->histogramWidget->height())/maxcount;
        p.drawLine(x,ui->histogramWidget->height()-y,x,ui->histogramWidget->height());
        if(i % 32 == 0||i==255)
        {
            p.drawText(QPoint(i,ui->histogramWidget->height()+15),QString::number(i));
        }
    }

}

void MainWindow::on_bitbreakBtn_clicked()//点击了转位平面按钮
{
    if(!image_isopen)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先打开一张图片！再进行该项操作！"));
        return;
    }
    bitMapShow=new BitMapShow;
    bitMapShow->setAttribute(Qt::WA_QuitOnClose,false);
    bitMapShow->bitMapDisplay(newImage);
    bitMapShow->show();
}

void MainWindow::on_transformASCIIBtn_clicked()//点击了转字符图按钮
{
    if(!is_gray)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片转换为灰度图，再进行该项操作！"));
        return;//未转换成灰度图，直接返回
    }
    transSymbol=new TransSymbol;
    transSymbol->setAttribute(Qt::WA_QuitOnClose,false);
    transSymbol->symbolDisplay(newImage);
    transSymbol->show();
}

void MainWindow::on_recoverBtn_clicked()//恢复按钮，直接恢复到打开图片时的样子
{
    if(!image_isopen)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先打开一张图片！再进行该项操作！"));
        return;
    }
    ui->label_4->setText(tr("平均像素值:"));
    ui->label_6->setText(tr("中值像素值:"));
    ui->gray_histogram->setWindowTitle("红色分量直方图");

    ui->label->resize(image->width(),image->height());
    ui->label->setPixmap(QPixmap::fromImage(*image));
    this->is_gray=false;
    *newImage=image->copy();//深拷贝
    this->update();
}

void MainWindow::on_action1_triggered()//图像变亮
{
    showGrayTransform(0);
}

void MainWindow::on_action2_triggered()//图像变暗
{
    showGrayTransform(1);
}

void MainWindow::on_action3_triggered()//图像反相
{
    showGrayTransform(2);
}

void MainWindow::on_action4_triggered()//对数变换，显示细节
{
    showGrayTransform(3);
}

void MainWindow::on_action5_triggered()//伽马变换，图像校“正”
{
    showGrayTransform(4);
}

void MainWindow::on_action_triggered()//平移
{
    showGrayTransform(5);
}

void MainWindow::on_action_2_triggered()//旋转
{
    showGrayTransform(6);
}

void MainWindow::on_action_5_triggered()//最近邻域差值，缩放
{
    showGrayTransform(7);
}

void MainWindow::on_action_6_triggered()//双线性差值，缩放
{
    showGrayTransform(8);
}

void MainWindow::on_action_4_triggered()//水平镜像
{
    showGrayTransform(9);
}

void MainWindow::on_action_7_triggered()//垂直镜像
{
    showGrayTransform(10);
}

void MainWindow::on_action3_3_triggered()//3×3均值滤波
{
    showGrayTransform(11);
}

void MainWindow::on_action5_5_triggered()//5×5均值滤波
{
    showGrayTransform(12);
}

void MainWindow::on_action_10_triggered()//3×3窗口中值滤波
{
    showGrayTransform(13);
}

void MainWindow::on_actionKsize_33_triggered()//3×3高斯滤波
{
    showGrayTransform(14);
}

void MainWindow::on_actionKsize_5_5_triggered()//5×5高斯滤波
{
    showGrayTransform(15);
}

void MainWindow::on_actionKsize_7_7_triggered()//7×7高斯滤波
{
    showGrayTransform(16);
}

void MainWindow::on_actionKsize_3_3_triggered()//3×3双边滤波
{
    showGrayTransform(17);
}

void MainWindow::on_actionKsize_5_6_triggered()//5×5双边滤波
{
    showGrayTransform(18);
}

void MainWindow::on_actionKsize_7_8_triggered()//7×7双边滤波
{
    showGrayTransform(19);
}

void MainWindow::on_action_9_triggered()//四邻域拉普拉斯算子
{
    showGrayTransform(20);
}

void MainWindow::on_action_12_triggered()//八邻域拉普拉斯算子
{
    showGrayTransform(21);
}

void MainWindow::on_actionSobel_triggered()//Sobel算子锐化
{
    showGrayTransform(22);
}

void MainWindow::on_actionPriwitt_triggered()
{
    showGrayTransform(23);
}

void MainWindow::on_actionWallis_triggered()
{
    showGrayTransform(24);
}

void MainWindow::on_actionkirsch_triggered()
{
    if(!is_gray)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片转换为灰度图！再进行该项操作！"));
        return;
    }
    grayTransform[25]=new GrayTransform;
    grayTransform[25]->setAttribute(Qt::WA_QuitOnClose,false);
    grayTransform[25]->GrayTransDisplay(*newImage,25,1,is_gray);
    grayTransform[25]->show();
}

void MainWindow::showGrayTransform(int index)
{
    if(!image_isopen)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先打开一张图片！再进行该项操作！"));
        return;
    }
    grayTransform[index]=new GrayTransform;
    grayTransform[index]->setAttribute(Qt::WA_QuitOnClose,false);
    grayTransform[index]->GrayTransDisplay(*newImage,index,1,is_gray);
    grayTransform[index]->show();
}

void MainWindow::on_equalizationBtn_clicked()//直方图均衡化
{
    if(!image_isopen)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先打开一张图片！再进行该项操作！"));
        return;
    }
    QImage *nowImage=new QImage(newImage->width(),newImage->height(),QImage::Format_ARGB32);//指定nowImage是32位的
    int pixelnum=newImage->width()*newImage->height();
    QVector <int>histRed(256);
    QVector <int>histGreen(256);
    QVector <int>histBlue(256);
    for(int y=0;y<newImage->height();y++)
    {
        for(int x=0;x<newImage->width();x++)
        {
            QColor color=newImage->pixel(x,y);
            histRed[color.red()]++;//获取当前图片的像素分布数组
            histGreen[color.green()]++;
            histBlue[color.blue()]++;
        }
    }

    QVector <int>pixelLevelRed(256);//红色灰度级的分布，pixelLevel[255]=width*height
    QVector <int>pixelLevelGreen(256);//绿色灰度级的分布，
    QVector <int>pixelLevelBlue(256);//蓝色灰度级的分布，

    int tempRed=0;
    int tempGreen=0;
    int tempBlue=0;//临时变量，用于存储sum
    for(int i=0;i<256;i++)
    {
        tempRed=tempRed+histRed[i];
        pixelLevelRed[i]=tempRed;

        tempGreen=tempGreen+histGreen[i];
        pixelLevelGreen[i]=tempGreen;

        tempBlue=tempBlue+histBlue[i];
        pixelLevelBlue[i]=tempBlue;
    }
    for(int y=0;y<newImage->height();y++)
    {
        for(int x=0;x<newImage->width();x++)
        {
            QColor color=newImage->pixel(x,y);
            int newRed=static_cast<int>(((pixelLevelRed[color.red()]*1.0)/pixelnum)*255.0);
            int newGreen=static_cast<int>(((pixelLevelGreen[color.green()]*1.0)/pixelnum)*255.0);
            int newBlue=static_cast<int>(((pixelLevelBlue[color.blue()]*1.0)/pixelnum)*255.0);
            nowImage->setPixel(x,y,qRgb(newRed,newGreen,newBlue));//使用该函数需要明确图片的位。具体还没弄清楚
        }
    }
    ui->label->resize(nowImage->width(),nowImage->height());
    ui->label->setPixmap(QPixmap::fromImage(*nowImage));
    *newImage=nowImage->copy();
    this->update();
}

Mat MainWindow::QImage2cvMat(const QImage &inImage, bool inCloneImageData)
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

QImage MainWindow::cvMat2QImage( const cv::Mat &inMat )
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

void MainWindow::on_action_3_triggered()//高斯去噪
{
    if(!is_gray)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片转换成灰度图片!"));
        return;
    }
    mat=QImage2cvMat(*image);
    after_gray_cvMat=QImage2cvMat(*newImage);
    GaussianBlur(after_gray_cvMat,after_gauss_cvMat,Size(3,3),0,0);
    //imshow("高斯滤波去噪效果",after_gauss_cvMat);
    namedWindow("Gauss",0);
    imshow("Gauss",after_gauss_cvMat);
    is_gauss=true;

}

void MainWindow::on_action_8_triggered()//canny算子边缘提取
{
    if(!is_gauss)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片进行高斯滤波去噪!"));
        return;
    }
    Canny(after_gauss_cvMat,after_canny_cvMat,50,150,3);//输入图像，输出图像，阙值1（低于它的像素点被认为不是边缘），阙值2（高于它的像素点会被认为是边缘），sobel算子大小
    //经过canny边缘提取后的图像为二值图
    //imshow("canny边缘提取效果",after_canny_cvMat);
    namedWindow("Canny",0);
    imshow("Canny",after_canny_cvMat);
    is_canny=true;
}

void MainWindow::on_action_14_triggered()//进行霍夫空间检测
{
    if(!is_canny)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片进行边缘提取!"));
        return;
    }
    QImage nowImage=cvMat2QImage(after_canny_cvMat);
    int width=nowImage.width();
    int height=nowImage.height();
    const double pi=3.1415926;
    int max_rho=static_cast<int>(sqrt(width*width+height*height));
    //QVector<QVector<int> >hough_space(500,QVector<int>(2*max_rho));
    QVector<int>temp_vector(2*max_rho);
    for(int i=0;i<500;i++)//初始化
    {
        hough_space.push_back(temp_vector);
        for(int j=0;j<2*max_rho;j++)
            hough_space[i][j]=0;
    }
    //int max_hough=0;
    for(int j=0;j<height;j++)//得到霍夫参数空间
    {
        for(int i=0;i<width;i++)
        {
            int x=i,y=j;
            QColor color=nowImage.pixel(x,y);
            if(color.red()==0)continue;//此时图片为二值图，灰度值为0表示该处不是边缘点，直接略过
            for(int k=0;k<499;k++)
            {
                double theta=((k+1)*pi)/500.0;
                double rho=x*qCos(theta)+y*qSin(theta);
                int r=static_cast<int>(rho+max_rho);
                if(r>=2*max_rho||r<0) continue;
                hough_space[k][r]++;
                //if(hough_space[k][r]>max_hough) max_hough=hough_space[k][r];
            }
        }
    }
    QMessageBox::information(this,"提示！","霍夫空间检测完成！");
}

void MainWindow::on_action_13_triggered()//霍夫直线绘制
{
    if(hough_space.isEmpty())
    {
        QMessageBox::information(this,tr("提示！"),tr("请先进行霍夫空间检测!"));
        return;
    }
    int threshold=QInputDialog::getInt(this,"输入","请输入阙值",100,0,2000);
    QVector<QVector2D>effective_points;
    QVector2D temp;
    const double pi=3.1415926;
    int max_rho=hough_space[0].size();//此处的max_rho实际上是已经乘以2倍了的
    for(int i=0;i<500;i++)//根据阙值分离出有效的直线参数
    {
        for(int j=0;j<max_rho;j++)
        {
            if(hough_space[i][j]>=threshold)
            {
                int number5=hough_space[i][j];
                //判断是否为局部（八邻域）最大值
                if(i==0&&j==0)
                {
                    if(number5<hough_space[i][j+1]||number5<hough_space[i+1][j]||number5<hough_space[i+1][j+1])continue;
                }
                else if (i==0&&j==max_rho-1)
                {
                    if(number5<hough_space[i][j-1]||number5<hough_space[i+1][j]||number5<hough_space[i+1][j-1])continue;
                }
                else if (i==499&&j==0)
                {
                    if(number5<hough_space[i][j+1]||number5<hough_space[i-1][j]||number5<hough_space[i-1][j+1])continue;
                }
                else if (i==499&&j==max_rho-1)
                {
                    if(number5<hough_space[i][j-1]||number5<hough_space[i-1][j-1]||number5<hough_space[i-1][j])continue;
                }
                else if(i==0)
                {
                    if(number5<hough_space[i][j-1]||number5<hough_space[i][j+1]||number5<hough_space[i+1][j-1]||number5<hough_space[i+1][j]||number5<hough_space[i+1][j+1])continue;
                }
                else if(i==499)
                {
                    if(number5<hough_space[i][j-1]||number5<hough_space[i][j+1]||number5<hough_space[i-1][j-1]||number5<hough_space[i-1][j]||number5<hough_space[i-1][j+1])continue;
                }
                else if(j==0)
                {
                    if(number5<hough_space[i-1][j]||number5<hough_space[i+1][j]||number5<hough_space[i-1][j+1]||number5<hough_space[i][j+1]||number5<hough_space[i+1][j+1])continue;
                }
                else if(j==max_rho-1)
                {
                    if(number5<hough_space[i-1][j]||number5<hough_space[i+1][j]||number5<hough_space[i-1][j-1]||number5<hough_space[i][j-1]||number5<hough_space[i+1][j-1])continue;
                }
                else
                {
                    if(number5<hough_space[i][j-1]||number5<hough_space[i][j+1]
                            ||number5<hough_space[i+1][j-1]||number5<hough_space[i+1][j]||number5<hough_space[i+1][j+1]
                            ||number5<hough_space[i-1][j-1]||number5<hough_space[i-1][j]||number5<hough_space[i-1][j+1])
                        continue;
                }
                temp.setX(static_cast<float>(((i+1)*pi)/500));//theta
                temp.setY(static_cast<float>(j-0.5*max_rho));//rho
                effective_points.push_back(temp);
            }
        }
    }
    qDebug()<<effective_points.size();
    Mat temp_mat=mat.clone();//深拷贝
    for(int i=0;i<effective_points.size();i++)
    {
        double theta=static_cast<double>(effective_points[i].x());
        double rho=static_cast<double>(effective_points[i].y());
        cv::Point pt1,pt2;
        double a=qCos(theta),b=sin(theta);
        double x0=a*rho,y0=b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        line(temp_mat,pt1,pt2,Scalar(0,0,255),1,CV_AA);
    }
    namedWindow("Hough line detection",0);
    imshow("Hough line detection",temp_mat);
}

void MainWindow::on_action_15_triggered()//傅里叶变换
{
    if(!is_gray)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片转换成灰度图片!"));
        return;
    }
    mat=QImage2cvMat(*image);
    cvtColor(mat,mat,CV_RGB2GRAY);
    //OpenCV中的傅里叶变换，在处理图像的长宽为2|3|5的倍数时速度更快
    int m=getOptimalDFTSize(mat.rows);//图像高变换后的高度
    int n=getOptimalDFTSize(mat.cols);//图像宽变换后的宽度
    Mat padded;//图像进行扩充后的新图像
    copyMakeBorder(mat,padded,m-mat.rows,0,0,n-mat.cols,BORDER_CONSTANT,Scalar::all(0));//对原图进行扩充，扩展部分用黑色0填充
    //将扩充后的新图像与……图像放在一个Mat数组中
    Mat planes[]={Mat_<float>(padded),Mat::zeros(padded.size(),CV_32F)};//Mat_<float>是CV_32F的意思，zeros()返回零矩阵
    Mat complexImg;
    merge(planes,2,complexImg);//将planes中的2个图像组合成一个多通道的图像
    cv::dft(complexImg,complexImg);//将complexImg中的双通道图像进行傅里叶变换，通道1存的是实部，，通道2存的是虚部
    split(complexImg,planes);//merge函数的逆变换，转换成一个为实部的mat,一个为虚部的mat。
    //这一部分是为了计算dft变换后的幅值，傅立叶变换的幅度值范围大到不适合在屏幕上显示。高值在屏幕上显示为白点，而低值为黑点，高低值的变化无法有效分辨。为了在屏幕上凸显出高低变化的连续性，我们可以用对数尺度来替换线性尺度,以便于显示幅值,计算公式如下:
    //=> log(1 + sqrt(Re(DFT(I))^2 +Im(DFT(I))^2))
    cv::magnitude(planes[0],planes[1],planes[0]);
    Mat mag=planes[0];
    mag =mag+Scalar::all(1);
    cv::log(mag,mag);
    mag = mag(Rect(0, 0, mag.cols & -2, mag.rows & -2));
    Mat _magI = mag.clone();
    //这一步的目的仍然是为了显示,但是幅度值仍然超过可显示范围[0,1],我们使用 normalize() 函数将幅度归一化到可显示范围。
    normalize(_magI, _magI, 0, 1, CV_MINMAX);
    //重新分配象限，使（0,0）移动到图像中心，
    //傅里叶变换之前要对源图像乘以（-1）^(x+y)进行中心化。
    //这是是对傅里叶变换结果进行中心化
    int cx = mag.cols/2;
    int cy = mag.rows/2;
    Mat tmp;
    Mat q0(mag, Rect(0, 0, cx, cy));   //Top-Left - Create a ROI per quadrant
    Mat q1(mag, Rect(cx, 0, cx, cy));  //Top-Right
    Mat q2(mag, Rect(0, cy, cx, cy));  //Bottom-Left
    Mat q3(mag, Rect(cx, cy, cx, cy)); //Bottom-Right
    //swap quadrants(Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    // swap quadrant (Top-Rightwith Bottom-Left)
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
    normalize(mag,mag, 0, 1, CV_MINMAX);
    imshow("Input Image", mat);
    imshow("spectrum magnitude",mag);
    //傅里叶的逆变换
    Mat ifft;
    cv::idft(complexImg,ifft,DFT_REAL_OUTPUT);
    normalize(ifft,ifft,0,1,CV_MINMAX);
    namedWindow("inverse fft",0);
    imshow("inverse fft",ifft);
}

void MainWindow::on_action_17_triggered()//小波变换
{
    if(!is_gray)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片转换成灰度图片!"));
        return;
    }
    mat=QImage2cvMat(*image);
    cvtColor(mat,mat,CV_RGB2GRAY);
    Mat mat_wdt=WDT(mat,"haar",3);
    //Mat mat_iwdt=IWDT(mat,"haar",3);
    namedWindow("WDT",0);
    imshow("WDT",mat_wdt);
    //imshow("IWDT",mat_iwdt);
}

Mat MainWindow::WDT(const Mat &_src, const string _wname, const int _level)//小波变换正变换
{
    //int reValue = THID_ERR_NONE;
    Mat src = Mat_<float>(_src);
    Mat dst = Mat::zeros(src.rows, src.cols, src.type());
    int N = src.rows;
    int D = src.cols;

    /// 高通低通滤波器
    Mat lowFilter;
    Mat highFilter;
    wavelet(_wname, lowFilter, highFilter);

    /// 小波变换
    int t = 1;
    int row = N;
    int col = D;

    while (t <= _level)
    {
        ///先进行行小波变换
        for (int i = 0; i<row; i++)
        {
            /// 取出src中要处理的数据的一行
            Mat oneRow = Mat::zeros(1, col, src.type());
            for (int j = 0; j<col; j++)
            {
                oneRow.at<float>(0, j) = src.at<float>(i, j);
            }
            oneRow = waveletDecompose(oneRow, lowFilter, highFilter);
            /// 将src这一行置为oneRow中的数据
            for (int j = 0; j<col; j++)
            {
                dst.at<float>(i, j) = oneRow.at<float>(0, j);
            }
        }
        normalize(dst, dst, 0, 1, NORM_MINMAX);
        //imshow("dst1", dst);

#if 0
        //normalize( dst, dst, 0, 255, NORM_MINMAX );
        IplImage dstImg1 = IplImage(dst);
        cvSaveImage("dst.jpg", &dstImg1);

#endif
        /// 小波列变换
        for (int j = 0; j<col; j++)
        {
            /// 取出src数据的一行输入
            Mat oneCol = Mat::zeros(row, 1, src.type());
            for (int i = 0; i<row; i++)
            {
                oneCol.at<float>(i, 0) = dst.at<float>(i, j);
            }
            oneCol = (waveletDecompose(oneCol.t(), lowFilter, highFilter)).t();

            for (int i = 0; i<row; i++)
            {
                dst.at<float>(i, j) = oneCol.at<float>(i, 0);
            }
        }
        normalize( dst, dst, 0, 1, NORM_MINMAX );
        //imshow("dst2", dst);
#if 0

        IplImage dstImg2 = IplImage(dst);
        cvSaveImage("dst.jpg", &dstImg2);
#endif

        /// 更新
        row /= 2;
        col /= 2;
        t++;
        src = dst;
    }

    return dst;
}

Mat MainWindow::IWDT(const Mat &_src, const string _wname, const int _level)//小波变换逆变换
{
    //int reValue = THID_ERR_NONE;
    Mat src = Mat_<float>(_src);
    Mat dst = Mat::zeros(src.rows, src.cols, src.type());
    int N = src.rows;
    int D = src.cols;

    /// 高通低通滤波器
    Mat lowFilter;
    Mat highFilter;
    wavelet(_wname, lowFilter, highFilter);

    /// 小波变换
    int t = 1;
    int row = static_cast<int>(N / std::pow(2., _level - 1));
    int col = static_cast<int>(D / std::pow(2., _level - 1));

    while (row <= N && col <= D)
    {
        /// 小波列逆变换
        for (int j = 0; j<col; j++)
        {
            /// 取出src数据的一行输入
            Mat oneCol = Mat::zeros(row, 1, src.type());
            for (int i = 0; i<row; i++)
            {
                oneCol.at<float>(i, 0) = src.at<float>(i, j);
            }
            oneCol = (waveletReconstruct(oneCol.t(), lowFilter, highFilter)).t();

            for (int i = 0; i<row; i++)
            {
                dst.at<float>(i, j) = oneCol.at<float>(i, 0);
            }
        }
        //normalize( dst, dst, 0, 1, NORM_MINMAX );
        imshow("dst3",dst);

#if 0
        //normalize( dst, dst, 0, 255, NORM_MINMAX );
        IplImage dstImg2 = IplImage(dst);
        cvSaveImage("dst.jpg", &dstImg2);
#endif
        ///行小波逆变换
        for (int i = 0; i<row; i++)
        {
            /// 取出src中要处理的数据的一行
            Mat oneRow = Mat::zeros(1, col, src.type());
            for (int j = 0; j<col; j++)
            {
                oneRow.at<float>(0, j) = dst.at<float>(i, j);
            }
            oneRow = waveletReconstruct(oneRow, lowFilter, highFilter);
            /// 将src这一行置为oneRow中的数据
            for (int j = 0; j<col; j++)
            {
                dst.at<float>(i, j) = oneRow.at<float>(0, j);
            }
        }
        normalize( dst, dst, 0, 1, NORM_MINMAX );
        imshow("dst4",dst);
#if 0
        //normalize( dst, dst, 0, 255, NORM_MINMAX );
        IplImage dstImg1 = IplImage(dst);
        cvSaveImage("dst.jpg", &dstImg1);
#endif

        row *= 2;
        col *= 2;
        src = dst;
    }

    return dst;
}

void MainWindow::wavelet(const string _wname, Mat &_lowFilter, Mat &_highFilter)// 生成不同类型的小波，现在只有haar，sym2
{
    if (_wname == "haar" || _wname == "db1")
    {
        int N = 2;
        _lowFilter = Mat::zeros(1, N, CV_32F);
        _highFilter = Mat::zeros(1, N, CV_32F);

        _lowFilter.at<float>(0, 0) = 1 / sqrtf(N);
        _lowFilter.at<float>(0, 1) = 1 / sqrtf(N);

        _highFilter.at<float>(0, 0) = -1 / sqrtf(N);
        _highFilter.at<float>(0, 1) = 1 / sqrtf(N);
    }
    if (_wname == "sym2")
    {
        int N = 4;
        float h[] = { -0.483f, 0.836f, -0.224f, -0.129f };
        float l[] = { -0.129f, 0.224f, 0.837f, 0.483f };

        _lowFilter = Mat::zeros(1, N, CV_32F);
        _highFilter = Mat::zeros(1, N, CV_32F);

        for (int i = 0; i<N; i++)
        {
            _lowFilter.at<float>(0, i) = l[i];
            _highFilter.at<float>(0, i) = h[i];
        }

    }
}

Mat MainWindow::waveletDecompose(const Mat &_src, const Mat &_lowFilter, const Mat &_highFilter)// 小波分解
{
    assert(_src.rows == 1 && _lowFilter.rows == 1 && _highFilter.rows == 1);
    assert(_src.cols >= _lowFilter.cols && _src.cols >= _highFilter.cols);
    Mat src = Mat_<float>(_src);

    int D = src.cols;

    Mat lowFilter = Mat_<float>(_lowFilter);
    Mat highFilter = Mat_<float>(_highFilter);


    /// 频域滤波，或时域卷积；ifft( fft(x) * fft(filter)) = cov(x,filter)
    Mat dst1 = Mat::zeros(1, D, src.type());
    Mat dst2 = Mat::zeros(1, D, src.type());

    filter2D(src, dst1, -1, lowFilter);
    filter2D(src, dst2, -1, highFilter);


    /// 下采样
    Mat downDst1 = Mat::zeros(1, D / 2, src.type());
    Mat downDst2 = Mat::zeros(1, D / 2, src.type());

    cv::resize(dst1, downDst1, downDst1.size());
    cv::resize(dst2, downDst2, downDst2.size());


    /// 数据拼接
    for (int i = 0; i<D / 2; i++)
    {
        src.at<float>(0, i) = downDst1.at<float>(0, i);
        src.at<float>(0, i + D / 2) = downDst2.at<float>(0, i);
    }

    return src;
}

Mat MainWindow::waveletReconstruct(const Mat &_src, const Mat &_lowFilter, const Mat &_highFilter)//小波重建
{
    assert(_src.rows == 1 && _lowFilter.rows == 1 && _highFilter.rows == 1);
    assert(_src.cols >= _lowFilter.cols && _src.cols >= _highFilter.cols);
    Mat src = Mat_<float>(_src);

    int D = src.cols;

    Mat lowFilter = Mat_<float>(_lowFilter);
    Mat highFilter = Mat_<float>(_highFilter);

    /// 插值;
    Mat Up1 = Mat::zeros(1, D, src.type());
    Mat Up2 = Mat::zeros(1, D, src.type());

    /// 线性插值
    Mat roi1(src, Rect(0, 0, D / 2, 1));
    Mat roi2(src, Rect(D / 2, 0, D / 2, 1));
    cv::resize(roi1, Up1, Up1.size(), 0, 0, INTER_CUBIC);
    cv::resize(roi2, Up2, Up2.size(), 0, 0, INTER_CUBIC);

    /// 前一半低通，后一半高通
    Mat dst1 = Mat::zeros(1, D, src.type());
    Mat dst2 = Mat::zeros(1, D, src.type());
    filter2D(Up1, dst1, -1, lowFilter);
    filter2D(Up2, dst2, -1, highFilter);

    /// 结果相加
    dst1 = dst1 + dst2;
    return dst1;
}

void MainWindow::on_action_16_triggered()//离散余弦变换
{
    if(!is_gray)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片转换成灰度图片!"));
        return;
    }
    mat=QImage2cvMat(*image);
    cvtColor(mat,mat,CV_RGB2GRAY);
    Mat mat_dct;
    Mat mat_idct;
    cv::resize(mat, mat, Size(512, 512));
    mat.convertTo(mat_dct,CV_32F,1.0/255);
    mat.convertTo(mat_idct,CV_32F,1.0/255);
    cv::dct(mat_dct,mat_dct);
    //cv::idct(mat_idct,mat_idct);
    namedWindow("DCT",0);
    imshow("DCT",mat_dct);
    //imshow("IDCT",mat_idct);
}

void MainWindow::on_action_18_triggered()//边缘跟踪
{
    mat=QImage2cvMat(*image);
    Canny(mat,after_canny_cvMat,50,150,3);//输出图像为二值图
    QImage nowImage=cvMat2QImage(after_canny_cvMat);//二值Mat图转换为QImage的Format_Grayscale8格式
    nowImage=nowImage.convertToFormat(QImage::Format_ARGB32);//转换成RGB模式
    qDebug()<<nowImage.format();
    const QPoint directions[8] = { { 0, 1 }, {1,1}, { 1, 0 }, { 1, -1 }, { 0, -1 },  { -1, -1 }, { -1, 0 },{ -1, 1 } };
    cv::RNG rng;//随机数类
    QVector<QPoint>edge_t;
    QVector<QVector<QPoint> >edges;
    int index=0;//表示当前点周围的索引
    int counts=0;
    for(int j=1;j<nowImage.height()-1;j++)
    {
        for(int i=1;i<nowImage.width()-1;i++)
        {
            QPoint b_pt=QPoint(i,j);//起始点
            QPoint c_pt=QPoint(i,j);//当前点
            //如果当前点为边缘点
            QColor color=nowImage.pixel(c_pt.x(),c_pt.y());//获取当前点的像素值
            if(color.red()==255)//如果是边缘点
            {
                edge_t.clear();
                bool tra_flag=false;
                //将该边缘点入栈
                edge_t.push_back(c_pt);
                nowImage.setPixel(c_pt.x(),c_pt.y(),qRgb(0,0,0));//将该点像素设为0表示该点已经访问过了
                //开始对b_pt点进行跟踪
                while(!tra_flag)
                {
                    for(counts=0;counts<8;counts++)
                    {
                        //防止索引出界
                        if(index>=8)
                        {
                            index-=8;
                        }
                        if(index<0)
                        {
                            index+=8;
                        }
                        //当前点更新
                        c_pt=QPoint(b_pt.x()+directions[index].x(),b_pt.y()+directions[index].y());
                        //边界判断
                        if(c_pt.x()>0&&c_pt.x()<nowImage.width()-1&&c_pt.y()>0&&c_pt.y()<nowImage.height()-1)
                        {
                            QColor color1=nowImage.pixel(c_pt.x(),c_pt.y());
                            if(color1.red()==255)//如果该点是边缘
                            {
                                index-=2;//避免重复检测
                                edge_t.push_back(c_pt);
                                nowImage.setPixel(c_pt.x(),c_pt.y(),qRgb(0,0,0));
                                //此时以当前点为地点搜索
                                b_pt.setX(c_pt.x());
                                b_pt.setY(c_pt.y());
                                break;//结束对八邻域的遍历。规定边缘以一个像素为单位
                            }
                        }
                        index++;
                    }
                    if(counts==8)//遍历结束,到达该步表示在当前点四周未找到边缘点
                    {
                        tra_flag=true;//结束标志
                        index=0;
                        edges.push_back(edge_t);
                    }
                }
            }
        }
    }
    newImage->fill(qRgb(0,0,0));//用0填充图片
    for (int i = 0; i < edges.size(); i++)
    {
        QColor color2(rng.uniform(0, 255),rng.uniform(0, 255),rng.uniform(0, 255));
        //cout << edges[i].size() << endl;
        // 过滤掉较小的边缘
        if (edges[i].size() > 5)
        {
            for (int j = 0; j < edges[i].size(); j++)
            {
                newImage->setPixel(edges[i][j].x(),edges[i][j].y(),qRgb(color2.red(),color2.green(),color2.blue()));
            }
        }
    }
    ui->label->resize(newImage->width(),newImage->height());
    ui->label->setPixmap(QPixmap::fromImage(*newImage));
}

void MainWindow::on_action_21_triggered()//游程编码压缩图像
{//文件编码格式：quint8(flag压缩的方法)int(图像的宽)int(图像的高)quint8(当前灰度值连续的数量)quint8(灰度值）quint8(当前灰度值连续的数量)quint8(灰度值）……
    if(!is_gray)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片转换成灰度图片!"));
        return;
    }
    QString filename = QFileDialog::getSaveFileName(this,tr("选择要保存的压缩文件路径"),".",tr("所有文件 (*)")); //选择路径
    if(filename.isEmpty())
    {
        return;
    }
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream input(&file);
    quint8 flag=1;//1表示游程编码压缩,2表示霍夫曼编码压缩
    input<<flag<<int(newImage->width())<<int(newImage->height());
    quint8 numbers=0;//当前灰度值连续的数目
    quint8 gray=0;//记录灰度值
    for(int j=0;j<newImage->height();j++)
    {
        for(int i=0;i<newImage->width();i++)
        {
            QColor color=newImage->pixel(i,j);
            if(numbers==0)
            {
                gray=static_cast<quint8>(color.red());//r=g=b
                numbers++;
            }
            if(gray==static_cast<quint8>(color.red()))
            {
                if(numbers==255)//一次最多存储255个相同更多颜色
                {
                    input<<numbers<<gray;
                    numbers=0;
                }
                numbers++;
            }
            else
            {
                input<<numbers<<gray;
                numbers=0;
            }
        }
    }
    QMessageBox::information(this,tr("提示！"),tr("压缩成功!"));
    file.close();
    return;
}

void MainWindow::on_action_25_triggered()//解压缩
{
    QString filename=QFileDialog::getOpenFileName(this,tr("打开压缩文件"),".","所有文件 (*)");
    if(filename.isEmpty()) return;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this,tr("提示！"),tr("文件打开失败!"));
        return;
    }
    QDataStream output(&file);
    quint8 flag;
    int width,height;
    output>>flag>>width>>height;//图像压缩的方式、图像的宽、图像的高
    *newImage=QImage(width,height,QImage::Format_ARGB32);
    qDebug()<<width<<height;
    if(flag==1)//游程编码解压缩
    {
        quint8 numbers=0,gray=0;
        for(int j=0;j<height;j++)
        {
            for(int i=0;i<width;i++)
            {
                if(numbers==0)
                {
                    output>>numbers>>gray;
                }
                newImage->setPixel(i,j,qRgb(gray,gray,gray));
                numbers--;
            }
        }
    }
    else if(flag==2)//霍夫曼编码解压缩
    {
        hfm_code.clear();
        quint8 num=0;
        output>>num;//有效灰度信息的数目
        for(int i=0;i<256;i++)
        {
            hfm_code<<"";//初始化
        }
        for(int i=0;i<num;i++)//读取霍夫曼码
        {
            QBitArray bit;
            quint8 gray;
            output>>gray;//灰度值
            output>>bit;//该灰度值的编码
            for(int j=0;j<bit.size();j++)
            {
                if(!bit[j])hfm_code[gray].append("0");
                else hfm_code[gray].append("1");
            }
        }
        ListPtr hfm_tree=new List;
        for(int i=0;i<256;i++)//根据霍夫曼码构造霍夫曼树
        {
            if(hfm_code[i]=="")continue;
            int j=0;
            ListPtr p=hfm_tree;
            while(1)
            {
                if(hfm_code[i][j]=="0")
                {
                    if(p->left_son==nullptr)
                    {
                        p->left_son=new List;
                        p=p->left_son;
                        p->code=0;
                    }
                    else p=p->left_son;
                }
                else
                {
                    if(p->right_son==nullptr)
                    {
                        p->right_son=new List;
                        p=p->right_son;
                        p->code=1;
                    }
                    else p=p->right_son;
                }
                j++;
                if(j==hfm_code[i].length())
                {
                    p->gray=i;
                    break;
                }
            }
        }
        ListPtr p=hfm_tree;
        QBitArray Bit;
        output>>Bit;
        qDebug()<<Bit.size();
        int pixelAds=0;
        for(int i=0;i<Bit.size();i++)
        {
            if (Bit[i])//1
                p = p->right_son;
            else//0
                p=p->left_son;

            if (p->gray!=-1)//若该节点为有效信息点
            {
                int col = pixelAds % width;
                int row = pixelAds / width;
                int grayValue = p->gray;
                pixelAds++;
                newImage->setPixel(col,row, qRgb(grayValue, grayValue, grayValue));
                p=hfm_tree;
            }
        }
    }
    else
    {
        QMessageBox::information(this,tr("提示！"),tr("无法完成解压缩!"));
    }
    ui->label->resize(newImage->width(),newImage->height());
    ui->label->setPixmap(QPixmap::fromImage(*newImage));
    *image=newImage->copy();
    is_gray=false;
    is_canny=false;
    is_gauss=false;
    hough_space.clear();
    image_isopen=true;
    file.close();
    return;
}

void MainWindow::on_actionHuffman_triggered()//霍夫曼编码
{
    //文件编码格式：quint8(flag压缩的方法)int(图像的宽)int(图像的高)quint8(有效灰度的数量)bit(颜色编码信息）
   if(!is_gray)
    {
        QMessageBox::information(this,tr("提示！"),tr("请先将图片转换成灰度图片!"));
        return;
    }
    QString filename = QFileDialog::getSaveFileName(this,tr("选择要保存的压缩文件路径"),".",tr("所有文件 (*)")); //选择路径
    if(filename.isEmpty()) return;
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QDataStream input(&file);
    quint8 flag=2;//1表示游程编码压缩,2表示霍夫曼编码压缩
    input<<flag<<int(newImage->width())<<int(newImage->height());
    QVector<int>gray_data(256,0);
    //获取灰度值频数
    for(int j=0;j<newImage->height();j++)
    {
        for(int i=0;i<newImage->width();i++)
        {
            QColor color=newImage->pixel(i,j);
            gray_data[color.red()]++;
        }
    }
    ListPtr hfm_tree,p,pre,temp;//hfm_tree是带头结点的单链表   p当前节点
    hfm_tree=new List;
    quint8 num=0;//灰度颜色的个数
    for(int i=0;i<256;i++)//构建森林，排序
    {
        if(gray_data[i]==0)continue;
        num++;
        pre=hfm_tree;
        p=hfm_tree->next;
        while(p!=nullptr&&p->numbers<=gray_data[i])
        {
            pre=p;
            p=p->next;
        }
        temp=new List;
        temp->gray=i;
        temp->numbers=gray_data[i];
        temp->next=p;
        pre->next=temp;
    }
    for(int i=0;i<num-1;i++)
    {
        pre=hfm_tree->next;
        p=pre->next;
        hfm_tree->next=p->next;
        temp=new List;
        temp->numbers=p->numbers+pre->numbers;
        temp->left_son=pre;pre->next=nullptr;pre->code=0;//左0右1
        temp->right_son=p;p->next=nullptr;p->code=1;
        //将新的节点插入合适的位置
        pre=hfm_tree;
        p=hfm_tree->next;
        while(p!=nullptr&&p->numbers<=temp->numbers)
        {
            pre=p;
            p=p->next;
        }
        temp->next=p;
        pre->next=temp;
    }
    hfm_code.clear();
    for(int i=0;i<256;i++)hfm_code<<"";//初始化
    QString string;
    QVector<QBitArray>hfm_bit(256);
    PreOrder_recur(hfm_tree->next,string);//获取用字符串表示的霍夫曼码
    /*for(int i=0;i<256;i++)
    {
        qDebug()<<i<<"      "<<hfm_code[i];
    }*/
    input<<num;//有效灰度信息的数目,一个字节
    for(int i=0;i<256;i++)//将编码信息存储进位数组中，并输入到文件中
    {
        if(hfm_code[i]=="") continue;
        input<<quint8(i);//1个字节的灰度值
        hfm_bit[i].resize(hfm_code[i].length());
        for(int j=0;j<hfm_code[i].length();j++)
        {
            if(hfm_code[i][j]=="0") hfm_bit[i][j]=0;
            else hfm_bit[i][j]=1;
        }
        input<<hfm_bit[i];
    }
    int bit_length=0;//记录所有编码组合而成的位的总长度
    QBitArray Bit(bit_length);//超长位数组
    int record=0;//记录当前已经存储到了多少位
    for(int j=0;j<newImage->height();j++)//将图像的灰度信息转换成编码输入到图像中
    {
        for(int i=0;i<newImage->width();i++)
        {
            QColor color=newImage->pixel(i,j);
            quint8 gray=static_cast<quint8>(color.red());
            bit_length+=hfm_bit[gray].size();
            Bit.resize(bit_length);
            for(int k=0;k<hfm_bit[gray].size();k++)
            {
                Bit[record++]=hfm_bit[gray][k];
            }
            //input<<hfm_bit[color.red()];
        }
    }
    input<<Bit;
    qDebug()<<bit_length;
    qDebug()<<Bit.size();
    QMessageBox::information(this,tr("提示！"),tr("压缩成功!"));
    file.close();
}

void MainWindow::PreOrder_recur(List *p,QString string)  //先序遍历霍夫曼树输出数据
{
    if(p!=nullptr)
    {
        if(p->code!=-1) string.append(QString::number(p->code));
        PreOrder_recur(p->left_son,string);
        if(p->gray!=-1) hfm_code[p->gray]=string;
        PreOrder_recur(p->right_son,string);
        if(p->gray!=-1) hfm_code[p->gray]=string;
    }

}
