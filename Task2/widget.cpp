#include "widget.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include "./ui_widget.h"
#include <qvalidator.h>

const int imWidth = 512;
const int imHeight = 512;
const int CT_schichten = 130;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->pushButton_load_3D, SIGNAL(clicked()), this, SLOT(Male3D()));
    connect(ui->pushButton_3D, SIGNAL(clicked()), this, SLOT(render3D()));
    connect(ui->slider_windowing_center,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(updatedWindowingCenter(int)));
    connect(ui->slider_windowing_width,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(updatedWindowingWidth(int)));
    connect(ui->lineEdit_center,
            SIGNAL(textChanged(QString)),
            this,
            SLOT(onLineEditCenterChanged(QString)));
    connect(ui->lineEdit_width,
            SIGNAL(textChanged(QString)),
            this,
            SLOT(onLineEditWidthChanged(QString)));

    connect(ui->slider_schicht, SIGNAL(valueChanged(int)), this, SLOT(updatedSchicht(int)));
    connect(ui->lineEdit_schicht,
            SIGNAL(textChanged(QString)),
            this,
            SLOT(onLineEditSchichtChanged(QString)));

    connect(ui->slider_schwellenwert,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(updatedSchwellenwert(int)));
    connect(ui->spinBox_schwellenwert,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(onSpinBoxSchwellenwertChanged(int)));

    this->imageDrawn = false;
    // idea behind adding this->imageDrawn: otherwise even if a picture hasn't been loaded yet, one could move the slider and the image in the label_image (which is just all pixels set to black) would then change color, plus I'm not sure what the access of m_pImageData[index] when it isn't set yet would be, is it a memory leak?

    // set the allowed input to the input boxes to the same range of values as the corresponding sliders
    QValidator *validator_center = new QIntValidator(ui->slider_windowing_center->minimum(),
                                                     ui->slider_windowing_center->maximum(),
                                                     this);
    ui->lineEdit_center->setValidator(validator_center);
    QValidator *validator_width = new QIntValidator(ui->slider_windowing_width->minimum(),
                                                    ui->slider_windowing_width->maximum(),
                                                    this);
    ui->lineEdit_width->setValidator(validator_width);
    QValidator *validator_schicht = new QIntValidator(ui->slider_schicht->minimum(),
                                                      ui->slider_schicht->maximum(),
                                                      this);
    ui->lineEdit_schicht->setValidator(validator_schicht);

    hideInputs();

    // Speicher reservieren
    m_ptiefenkarte = new short[imHeight * imWidth];
    m_pshadedBuffer = new short[imHeight * imWidth];

    this->is3dDrawn = false;
    // idea behind adding the variable is3dDrawn: so that if the 3D button was clicke and the 3D view was rendered, then I can connect things so that changing the Schwellenwert through the slider/input automatically changes the 3D view too
    this->isDepthBufferCreated = false;
}

Widget::~Widget()
{
    delete ui;
    // Speicher wieder freigeben
    delete[] m_ptiefenkarte;
    delete[] m_pshadedBuffer;
}

int Widget::windowing(int HU_value, int windowCenter, int windowWidth, int &greyValue)
{
    //Fensterung berechnen
    int window_min = windowCenter - windowWidth / 2;
    int window_max = windowCenter + windowWidth / 2;
    if (HU_value < window_min) {
        greyValue = 0;
    } else if (HU_value > window_max) {
        greyValue = 255;
    } else {
        greyValue = static_cast<int>(255.0 * (HU_value - (windowCenter - windowWidth / 2))
                                     / windowWidth);
    }

    return 0;
}

void Widget::updatedWindowingCenter(int value)
{
    if (!this->imageDrawn)
        return;
    ui->lineEdit_center->setText(QString::number(value));
    // ui->label_center->setText("Center: " + QString::number(value));
    updateSliceView();
}
void Widget::updatedWindowingWidth(int value)
{
    if (!this->imageDrawn)
        return;
    ui->lineEdit_width->setText(QString::number(value));
    updateSliceView();
}

void Widget::onLineEditCenterChanged(const QString &text)
{
    int value = text.toInt();
    ui->slider_windowing_center->setValue(value); // This will also trigger `updatedWindowingCenter`
}

void Widget::onLineEditWidthChanged(const QString &text)
{
    int value = text.toInt();
    ui->slider_windowing_width->setValue(value); // This will also trigger `updatedWindowingWidth`
}

void Widget::updateSliceView()
{
    QElapsedTimer timer;
    timer.start();

    // Erzeuge ein Objekt vom Typ Image
    QImage image(imWidth, imHeight, QImage::Format_RGB32);

    // Initialisiere das Bild mit schwarzem Hintergrund
    image.fill(qRgb(0, 0, 0));

    // Setze Inhalt des Arrays Pixel für Pixel in das Bild
    int schicht = 0;
    if (ui->frame_schicht->isVisible()) {
        schicht = ui->slider_schicht->value();
    }
    int schwellenwert = 0;
    if (ui->frame_schwellenwert->isVisible()) {
        schwellenwert = ui->slider_schwellenwert->value();
    }

    short *m_pImageData = dataset.data();

    int index;
    int greyValue;
    for (int y = 0; y < imHeight; ++y) {
        for (int x = 0; x < imWidth; ++x) {
            // Berechne den zugehörigen index des Speichers
            index = y * imWidth + x;

            /*
             * Wenn der HU-Wert den Schwellenwert überschreitet bzw. gleich ist
             * (siehe Folie 4 von 4 - Segmentierung 3D Rekonstruktion),
             * dann setze die Pixelfarbe einfach auf rot
            */
            if (m_pImageData[index + schicht * (imHeight * imWidth)] >= schwellenwert) {
                image.setPixel(x, y, qRgb(255, 0, 0));
            } else {
                // Grauwert an dem index aus imageData auslesen
                windowing(m_pImageData[index + schicht * (imHeight * imWidth)],
                          ui->slider_windowing_center->value(),
                          ui->slider_windowing_width->value(),
                          greyValue);
                // Grauwert als Pixel an der Position x, y im image setzen, wenn der HU-Wert nicht den Schwellenwert überschreitet
                image.setPixel(x, y, qRgb(greyValue, greyValue, greyValue));
            }
        }
    }

    // Bild auf Benutzeroberfläche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
    this->imageDrawn = true;

    qDebug() << timer.nsecsElapsed();
}

void Widget::hideInputs()
{
    ui->frame_center->setVisible(false);
    ui->frame_width->setVisible(false);
    ui->frame_schicht->setVisible(false);
    ui->frame_schwellenwert->setVisible(false);
    ui->frame_coordinates->setVisible(false);

    ui->pushButton_3D->setDisabled(true);
}

void Widget::showInputs()
{
    ui->frame_center->setVisible(true);
    ui->frame_width->setVisible(true);
    ui->frame_schicht->setVisible(true);
    ui->frame_schwellenwert->setVisible(true);
    ui->frame_coordinates->setVisible(true);

    ui->lineEdit_center->setText(QString::number(ui->slider_windowing_center->value()));
    ui->lineEdit_width->setText(QString::number(ui->slider_windowing_width->value()));
    ui->lineEdit_schicht->setText(QString::number(ui->slider_schicht->value()));
    ui->spinBox_schwellenwert->setValue(ui->slider_schwellenwert->value());

    ui->pushButton_3D->setDisabled(false);
}

void Widget::Male3D()
{
    // QFileDialog zum Auswählen von .raw Bilddateien öffnen und Datei auswählen
    QString imagePath = QFileDialog::getOpenFileName(this,
                                                     "Open Image",
                                                     "../../",
                                                     "Raw Image Files (*.raw)");
    //load dataset
    try {
        dataset.load(imagePath);
    } catch (QString e) {
        QMessageBox::critical(this, "ACHTUNG", e);
        return;
    }

    // Bild generieren und auf die GUI anzeigen
    updateSliceView();
    // Slider aktivieren und Default-Werte anzeigen
    showInputs();
}

void Widget::updatedSchicht(int value)
{
    ui->lineEdit_schicht->setText(QString::number(value));
    updateSliceView();
}

void Widget::onLineEditSchichtChanged(const QString &text)
{
    int value = text.toInt();
    ui->slider_schicht->setValue(value); // This will also trigger `updatedSchicht`
}

void Widget::updatedSchwellenwert(int value)
{
    ui->spinBox_schwellenwert->setValue(value);
    updateSliceView();
    if (this->is3dDrawn) {
        render3D();
    }
}
void Widget::onSpinBoxSchwellenwertChanged(int value)
{
    ui->slider_schwellenwert->setValue(value); // This will also trigger `updatedSchwellenwert`
    updateSliceView();
}

/*
die Parameter width, height und layers bestimmen jeweils die Breite, Höhe und Tiefe des CT-Datensatzes,
welcher durch den Zeiger inputData übergeben wird. threshold ist der Schwellenwert in HU.
depthBuffer ist ein (vorher anzulegender!) Speicherbereich, in dem die
Tiefenkarte der Größe width*height als Rückgabewert gespeichert wird.
*/
int Widget::calculateDepthBuffer(
    short *inputData, int width, int height, int layers, int threshold, short *depthBuffer)
{
    const int imageSize = (width * height);
    int index;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Berechne den zugehörigen index des Speichers
            index = y * width + x;

            /* Wir stellen uns vor, wir schauen aus einer bestimmten Richtung auf unseren 3D-Datensatz
             * und zählen, wie viele Voxel können wir nach vorne gehen, bis der erste Voxel kommt,
             * der den eingestellten Schwellenwert überschreitet. Wir schauen in positive Z-Richtung
             * (Schichtnummer) auf unseren Datensatz. Die Anzahl Schichten, die man durchlaufen musste,
             * ist nun an der Stelle (x,y) der zugehörige Tiefenwert.
            */
            bool isDepthSet = false;
            for (int layer = 0; layer < layers; layer++) {
                if (dataset.data()[index + layer * imageSize] >= threshold) {
                    depthBuffer[index] = layer;
                    isDepthSet = true;
                    break;
                }
            }
            /* if for some pixel (x, y) there is no layer found where the HU value is bigger
             * than the threshold, then set the depth to either (layers) or (layers - 1)     (source: lecture)
            */
            if (!isDepthSet)
                depthBuffer[index] = layers;
        }
    }

    return 0;
}

int Widget::renderDepthBuffer(const short *depthBuffer, int width, int height, short *shadedBufer)
{
    /*
     * Bei einer Schrittweite von 2 fällt der erste und letzte Pixel im Bild weg, das heißt,
     * das Bild wird nur im Bereich zwischen 1 und 510 gezeichnet.
    */
    /*const int s_x = 2;
    const int s_y = 2;*/
    int T_x, T_y;
    for (int y = 1; y < imHeight - 1; ++y) {
        for (int x = 1; x < imWidth - 1; ++x) {
            // Berechne für jeden Pixel den normierten Intensitätswert I_reflektiert, welcher direkt der anzuzeigende Grauwert ist
            T_x = depthBuffer[y * imWidth + x - 1] - depthBuffer[y * imWidth + x + 1];
            T_y = depthBuffer[(y - 1) * imWidth + x] - depthBuffer[(y + 1) * imWidth + x];
            // calculate I_reflektiert and store the value in the shadedBuffer
            /*shadedBufer[y * imWidth + x] = 255.0 * (s_x * s_y)
                                           / sqrt(pow((s_y * T_x), 2) + pow((s_x * T_y), 2)
                                                  + pow((s_x * s_y), 2));*/
            shadedBufer[y * imWidth + x] = 255.0 * 4
                                           / sqrt(pow((2 * T_x), 2) + pow((2 * T_y), 2) + 16);
        }
    }
    this->isDepthBufferCreated = true;

    return 0;
}

void Widget::render3D()
{
    int threshold = 0;
    if (ui->frame_schwellenwert->isVisible()) {
        threshold = ui->slider_schwellenwert->value();
    }

    calculateDepthBuffer(dataset.data(), imWidth, imHeight, CT_schichten, threshold, m_ptiefenkarte);
    renderDepthBuffer(m_ptiefenkarte, imWidth, imHeight, m_pshadedBuffer);

    // Erzeuge ein Objekt vom Typ Image
    QImage image(imWidth, imHeight, QImage::Format_RGB32);

    // Initialisiere das Bild mit schwarzem Hintergrund
    image.fill(qRgb(0, 0, 0));

    int iGrauwert;
    for (int y = 0; y < imHeight; ++y) {
        for (int x = 0; x < imWidth; ++x) {
            iGrauwert = m_pshadedBuffer[y * imWidth + x];
            image.setPixel(x, y, qRgb(iGrauwert, iGrauwert, iGrauwert));
        }
    }

    // Bild auf Benutzeroberfläche anzeigen
    ui->label_image3D->setPixmap(QPixmap::fromImage(image));
    is3dDrawn = true;
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    QPoint globalPos = event->pos();
    QPoint localPos = ui->label_image3D->mapFromParent(globalPos);
    if (ui->label_image3D->rect().contains(localPos)) {
        ui->value_x->setText(QString::number(localPos.x()));
        ui->value_y->setText(QString::number(localPos.y()));

        if (this->isDepthBufferCreated) {
            ui->value_z->setText(
                QString::number(this->m_ptiefenkarte[localPos.x() + localPos.y() * imWidth]));
        }
        return;
    }
    localPos = ui->label_image->mapFromParent(globalPos);
    if (ui->label_image->rect().contains(localPos)) {
        ui->value_x->setText(QString::number(localPos.x()));
        ui->value_y->setText(QString::number(localPos.y()));
        ui->value_z->setText(QString::number(ui->slider_schicht->value()));
    }
}
