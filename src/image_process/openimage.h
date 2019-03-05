#ifndef OPENIMAGE_H
#define OPENIMAGE_H

#include <QMainWindow>
#include<QMouseEvent>
#include<QLabel>
#include"myqlabel.h"
#include<QtDebug>
#include<QString>
#include<QFileDialog>
#include<QPoint>
namespace Ui {
class OpenImage;
}

class OpenImage : public QMainWindow
{
    Q_OBJECT

public:
    explicit OpenImage(QWidget *parent = 0);
    ~OpenImage();

protected:
    //重载基类的鼠标按下事件处理函数
    virtual void mousePressEvent(QMouseEvent *event);
    //重载基类的鼠标释放事件处理函数
    virtual void mouseReleaseEvent(QMouseEvent *event);
    //重载基类的鼠标移动事件处理函数
    virtual void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::OpenImage *ui;
    MyQLabel *startLabel; //对话框背景图片
    QPoint move_point;   //鼠标按下位置
    QPoint move_size;
    bool mouse_press;    //鼠标是否按下的标识
    void startLabel_clicked();//MyQLabel被点击
signals:
    void openImageFile(QString filename);//打开图片文件的信号
};
#endif // OPENIMAGE_H
