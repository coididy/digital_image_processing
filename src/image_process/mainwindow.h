#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include<QMainWindow>
#include<QString>
#include<QDebug>
#include<QDockWidget>
#include<QLabel>
#include<QTextEdit>
#include<QWidget>
#include<QFileDialog>
#include<QInputDialog>
#include<QImage>
#include<QMessageBox>
#include<QImageReader>
#include<QPixmap>
#include<QScreen>
#include<QRgb>
#include<QFile>
#include<QDataStream>
#include<QColor>
#include<QDesktopWidget>
#include<QApplication>
#include<QVector>
#include<QPainter>
#include<QPaintEvent>
#include<QtCore/qmath.h>
#include<QByteArray>
#include<iostream>
#include<string>
#include<QBitArray>
#include<QLinkedList>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include"bitmapshow.h"
#include"transsymbol.h"
#include"samp.h"
#include"graytransform.h"
using namespace cv;
using namespace std;
namespace Ui {
class MainWindow;
}
typedef struct Tree
{
    int gray{-1};
    int numbers{0};
    int code{-1};//编码
    struct Tree *left_son{nullptr},*right_son{nullptr};
    struct Tree *next{nullptr};//单链表状态下
}List,*ListPtr;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void paintEvent(QPaintEvent *event);
private:
    Ui::MainWindow *ui;
    //显示采样率的窗口
    Samp *samp;
    //显示8副位平面的窗口
    BitMapShow *bitMapShow;
    //显示符号图像的窗口
    TransSymbol *transSymbol;
    //左窗口，显示图片
    //显示各种灰度变换的窗口
    void showGrayTransform(int index);
    GrayTransform *grayTransform[30];
    QLabel *imageLabel;
    //此时是否已经转换灰度图
    bool is_gray;
    //此时是否已经打开了一张图片
    bool image_isopen;
    //右窗口，停靠窗口，显示直方图
    QDockWidget *dock1;
    QWidget *w1;
    QDockWidget *dock2;
    QWidget *w2;
    QDockWidget *dock3;
    QWidget *w3;
    QImage *image;//打开的图像
    QImage *newImage;//转换后的图像
    //霍夫直线检测
    bool is_gauss;//已经进行了高斯去噪
    bool is_canny;//已经进行了边缘提取
    Mat mat;//原图
    Mat after_gray_cvMat;//进行灰度化后的图像
    Mat after_gauss_cvMat;//进行高斯滤波后的图像
    Mat after_canny_cvMat;//进行canny算子边缘提取后的图像
    QVector<QVector<int> >hough_space;//霍夫参数空间

    QList<QString>hfm_code;//霍夫码

    //小波变换
    Mat WDT(const Mat&_src,const string _wname,const int _level);//正变换
    Mat IWDT(const Mat &_src, const string _wname, const int _level);//逆变换
    void wavelet(const string _wname, Mat &_lowFilter, Mat &_highFilter);// 生成不同类型的小波，现在只有haar，sym2
    Mat waveletDecompose(const Mat &_src, const Mat &_lowFilter, const Mat &_highFilter);// 小波分解
    Mat waveletReconstruct(const Mat &_src, const Mat &_lowFilter, const Mat &_highFilter);//小波重建

    bool openImageFile(QString filename);
    QImage * greyScale(QImage &origin);
    void grayHistogram(QImage *origin);
    QImage cvMat2QImage(const Mat &inmat);//Mat转QImage
    Mat QImage2cvMat(const QImage &inImage, bool inCloneImageData = true);//QImage转Mat
    void  PreOrder_recur(List *p,QString string);  //先序遍历霍夫曼树输出数据

public slots:
    void windowShow(QString filename);
private slots:
    void on_openFileBtn_clicked();
    void on_saveAsBtn_clicked();
    void on_transformBtn_clicked();
    void on_sureBtn_clicked();
    void on_bitbreakBtn_clicked();
    void on_transformASCIIBtn_clicked();
    void on_recoverBtn_clicked();
    void on_action1_triggered();
    void on_action2_triggered();
    void on_action3_triggered();
    void on_action4_triggered();
    void on_action5_triggered();
    void on_equalizationBtn_clicked();
    void on_action_5_triggered();
    void on_action_6_triggered();
    void on_action_triggered();
    void on_action_2_triggered();
    void on_action_4_triggered();
    void on_action_7_triggered();
    void on_action3_3_triggered();
    void on_action5_5_triggered();
    void on_action_10_triggered();
    void on_actionKsize_33_triggered();
    void on_actionKsize_5_5_triggered();
    void on_actionKsize_7_7_triggered();
    void on_actionKsize_3_3_triggered();
    void on_actionKsize_5_6_triggered();
    void on_actionKsize_7_8_triggered();
    void on_action_9_triggered();
    void on_action_12_triggered();
    void on_actionSobel_triggered();
    void on_actionPriwitt_triggered();
    void on_actionWallis_triggered();
    void on_action_3_triggered();
    void on_action_8_triggered();
    void on_action_14_triggered();
    void on_action_13_triggered();
    void on_action_15_triggered();
    void on_action_17_triggered();
    void on_action_16_triggered();
    void on_actionkirsch_triggered();
    void on_action_18_triggered();
    void on_action_21_triggered();
    void on_action_25_triggered();
    void on_actionHuffman_triggered();
};

#endif // MAINWINDOW_H
