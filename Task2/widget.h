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
    short *m_pImageData;
    int windowing(int HU_value, int center, int width);
    void updateSliceView();
    bool imageDrawn;
    void hideInputs();
    void showInputs();

private slots:
    void MaleBild8Bit();
    void MaleBild12Bit();
    void updatedWindowingCenter(int value);
    void updatedWindowingWidth(int value);
    void onLineEditCenterChanged(const QString &text);
    void onLineEditWidthChanged(const QString &text);
};

#endif // WIDGET_H
