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
    int windowing(int HU_value, int windowCenter, int windowWidth, int &greyValue);
    void updateSliceView();
    bool imageDrawn;
    void hideInputs();
    void showInputs();
    short *m_ptiefenkarte;
    int calculateDepthBuffer(
        short *inputData, int width, int height, int layers, int threshold, short *depthBuffer);
    int renderDepthBuffer(const short *depthBuffer, int width, int height, short *shadedBufer);
    short *m_pshadedBuffer;
    bool is3dDrawn;
    bool isDepthBufferCreated;

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
