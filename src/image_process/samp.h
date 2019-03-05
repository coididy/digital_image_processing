#ifndef SAMP_H
#define SAMP_H

#include <QWidget>
#include<QImage>
#include<QString>
#include<QPixmap>
#include<QDebug>
namespace Ui {
class Samp;
}

class Samp : public QWidget
{
    Q_OBJECT

public:
    explicit Samp(QWidget *parent = nullptr);
    void changSamp(int sampsize,int quantificationsize,const QImage &image);
    int getvalue(int level,int color);
    ~Samp();

private:
    Ui::Samp *ui;
    QImage*gauss_Smoothness(QImage *origin);
};

#endif // SAMP_H
