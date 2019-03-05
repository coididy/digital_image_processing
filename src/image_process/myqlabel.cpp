#include "myqlabel.h"

MyQLabel::MyQLabel(QWidget *parent):QLabel (parent)
{

}
MyQLabel::~MyQLabel(void)
{

}
void MyQLabel::mouseReleaseEvent(QMouseEvent * ev)
{//定义鼠标左键释放事件
    if(ev->button() == Qt::LeftButton)
    {
        Q_UNUSED(ev)
            emit clicked();
    }
}
