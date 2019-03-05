#include "openimage.h"
#include "ui_openimage.h"
OpenImage::OpenImage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OpenImage)
{
    move_size=QPoint(0,0);
    //设置界面背景透明
        QPalette palette = this->palette();
        palette.setBrush(QPalette::Window, QBrush(Qt::NoButton));  //透明画刷
        setPalette(palette);            //设置当前对话框的调色板
        setAutoFillBackground(true);    //自动填充背景颜色
        setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);  //屏幕边框消失，窗体位于最前方
        setAttribute(Qt::WA_TranslucentBackground);  //窗体透明

        //背景标签
        startLabel = new MyQLabel(this);
        QPixmap pmpBg(":/images/resources/images/start.png");
        startLabel->setFixedSize(pmpBg.width(), pmpBg.height());  //设置背景标签的大小和背景图标大小一致
        this->setFixedSize(pmpBg.width(), pmpBg.height());
        //设置背景标签样式表
        //startLabel->setStyleSheet();  //qss文件中设置
        startLabel->move(0, 0);
        startLabel->setVisible(true);
        startLabel->setCursor(QCursor(Qt::PointingHandCursor));
        connect(startLabel,&MyQLabel::clicked,this,&OpenImage::startLabel_clicked);
}

OpenImage::~OpenImage()
{
    delete ui;
}
//重载基类的鼠标按下事件处理函数
void OpenImage:: mousePressEvent(QMouseEvent *event)
{
    //只能是鼠标左键移动
    if(event->button() == Qt::LeftButton)
    {
        mouse_press = true;
    }
    /*
     * 窗口移动距离
     * globalPose(), 以桌面左上角为原点，绝对坐标
     * pos(), 窗体左上角（不含边框）的坐标，只要鼠标在窗体内，且窗体不动，坐标不变
     */
    move_point = event->globalPos() - pos();
}
//重载基类的鼠标释放事件处理函数
void OpenImage:: mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    mouse_press = false;
}

//重载基类的鼠标移动事件处理函数
void OpenImage:: mouseMoveEvent(QMouseEvent *event)
{
    //移动窗口
    if(mouse_press)
    {
        /*
         *  event->globalPos() - move_point
         * =event->globalPos() - (event->globalPos0() - pos0())
         * =pos() + (event->globalPos() - event->globalPos0())
         */
        move_size=event->globalPos() - move_point;
        move(move_size);
    }
}
void OpenImage::startLabel_clicked()
{
    if(move_size!=QPoint(0,0))
    {
        move_size=QPoint(0,0);
        return;
    }
    QString filename=QFileDialog::getOpenFileName(this,tr("打开图片"),".","image Files(*.jpg *.png *.jpeg)");
    if(filename.isEmpty())
            return;
    else
    {
        emit openImageFile(filename);
        this->close();
    }
}
