#ifndef TRANSSYMBOL_H
#define TRANSSYMBOL_H

#include <QWidget>
#include<QImage>
#include<QColor>
#include<QImageReader>
#include<QPixmap>
#include<QDebug>
namespace Ui {
class TransSymbol;
}

class TransSymbol : public QWidget
{
    Q_OBJECT

public:
    explicit TransSymbol(QWidget *parent = nullptr);
    void symbolDisplay(QImage *image);
    ~TransSymbol();

private:
    Ui::TransSymbol *ui;
};

#endif // TRANSSYMBOL_H
