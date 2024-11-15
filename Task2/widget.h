#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "CTDataset.h"

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
    void updateSliceView();
    bool imageDrawn;
    void hideInputs();
    void showInputs();
    bool is3dDrawn;
    CTDataset dataset;

private slots:
    void updatedWindowingCenter(int value);
    void updatedWindowingWidth(int value);
    void onLineEditCenterChanged(const QString &text);
    void onLineEditWidthChanged(const QString &text);
    void Male3D();
    void updatedSchicht(int value);
    void onLineEditSchichtChanged(const QString &text);
    void updatedSchwellenwert(int value);
    void onSpinBoxSchwellenwertChanged(int value);
    void render3D();
    void mousePressEvent(QMouseEvent *event);
};

#endif // WIDGET_H
