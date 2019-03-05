#ifndef MYQLABEL_H
#define MYQLABEL_H

#include<QLabel>
#include<QMouseEvent>
class MyQLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyQLabel(QWidget *parent=0);
    ~MyQLabel(void);
protected:
    void mouseReleaseEvent(QMouseEvent * event);
signals:
    void clicked();//鼠标点击事件
};

#endif // MYQLABEL_H
