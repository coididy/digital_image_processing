#ifndef BITMAPSHOW_H
#define BITMAPSHOW_H

#include <QWidget>
#include<QImage>
#include<QLabel>
#include<QPixmap>
#include<QImage>
namespace Ui {
class BitMapShow;
}

class BitMapShow : public QWidget
{
    Q_OBJECT

public:
    explicit BitMapShow(QWidget *parent = nullptr);
    void bitMapDisplay(QImage *origin);
    ~BitMapShow();

private:
    Ui::BitMapShow *ui;
    QLabel *label[8];

};

#endif // BITMAPSHOW_H
