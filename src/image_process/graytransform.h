#ifndef GRAYTRANSFORM_H
#define GRAYTRANSFORM_H

#include <QWidget>
#include<QImage>
#include<QString>
#include<QVector>
#include<QPainter>
#include<QPixmap>
#include<QPaintEvent>
#include<QtCore/qmath.h>
#include<QColor>
#include<QtDebug>
#include<QLabel>
#include<QSlider>
#include<QtAlgorithms>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace cv;
namespace Ui {
class GrayTransform;
}

class GrayTransform : public QWidget
{
    Q_OBJECT

public:
    explicit GrayTransform(QWidget *parent = nullptr);
    void GrayTransDisplay(const QImage origin,int flag,int times,bool judge);//judge标明该图像是否为灰度图，如果是灰度图需要显示灰度直方图，否则不需要显示
    ~GrayTransform();
protected:
    void paintEvent(QPaintEvent *event);
private slots:
    void on_pushButton_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void addHorizontalSlider_valueChanged(int value);

private:
    Ui::GrayTransform *ui;
    QImage *image;
    int flag;
    int delta_x,delta_y;//当前ui->sliderValue->text()的值，若只有一个参数，则y有效
    int paramsNumbers;//判断几个参数有效，2表示两个参数有效
    bool is_gray;
    QVector <int>hist;
    int maxcount;//记录灰度值最多的像素的个数
    int graysum;//记录灰度值的和
    int midgray;//记录中值灰度的值
    double averageGray;//记录平均灰度值
    double standardDeviation;//记录灰度标准差
    double deviation;//方差的pixelnum倍
    QSlider *addHorizontalSlider;//第二个水平滑动条

    void getOptimalgrayValue(int gray[8]);//kirsch边缘检测中获取最佳灰度值
    void getGaussianTemplate(int size,double kernel[8][8],double sigma2);//获取size×size大小的高斯模板
    void getBilateralFiltering(int x,int y,int size,double kernel_red[8][8],double kernel_green[8][8],double kernel_blue[8][8],double sigma2);//获取size×size大小的双边滤波模板
    void getSpaceWidget(int size,double kernel[8][8],double sigma2);//获取距离权重
    Mat QImage2cvMat(const QImage &inImage, bool inCloneImageData=true);
    QImage cvMat2QImage( const cv::Mat &inMat );
    double fourValueMin(double a,double b,double c,double d);//求四个数中的最小值
    double fourValueMax(double a,double b,double c,double d);//求四个数中的最大值
    QPointF matrixMult(int nowx,int nowy,double left,double top,double angle,int x0,int y0);//矩阵运算求出变换前在原图像，原坐标系下的的点坐标
};

#endif // GRAYTRANSFORM_H
