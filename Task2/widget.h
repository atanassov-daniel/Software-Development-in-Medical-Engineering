#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    short* m_pImageData;
    int windowing(int HU_value, int center, int width);

private slots:
    void MaleBild8Bit();
    void MaleBild12Bit();
};

#endif // WIDGET_H
