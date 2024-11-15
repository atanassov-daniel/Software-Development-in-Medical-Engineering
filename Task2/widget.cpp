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

    this->is3dDrawn = false;
    // idea behind adding the variable is3dDrawn: so that if the 3D button was clicke and the 3D view was rendered, then I can connect things so that changing the Schwellenwert through the slider/input automatically changes the 3D view too
}

Widget::~Widget()
{
    delete ui;
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
                if (CTDataset::windowing(m_pImageData[index + schicht * (imHeight * imWidth)],
                                         ui->slider_windowing_center->value(),
                                         ui->slider_windowing_width->value(),
                                         greyValue)
                    == ReturnCode::OK) {
                    // Grauwert als Pixel an der Position x, y im image setzen, wenn der HU-Wert nicht den Schwellenwert überschreitet
                    image.setPixel(x, y, qRgb(greyValue, greyValue, greyValue));
                } else {
                    // TODO Error handling
                }
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

void Widget::render3D()
{
    int threshold = 0;
    if (ui->frame_schwellenwert->isVisible()) {
        threshold = ui->slider_schwellenwert->value();
    }

    dataset.renderDepthBuffer(imWidth, imHeight, CT_schichten, threshold);
    short *m_pshadedBuffer = dataset.shadedBuffer();

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

        if (dataset.existsDepthBuffer()) {
            ui->value_z->setText(
                QString::number(dataset.depthBuffer()[localPos.x() + localPos.y() * imWidth]));
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
