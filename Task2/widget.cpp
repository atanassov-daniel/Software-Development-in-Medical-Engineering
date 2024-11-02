#include "widget.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
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
    connect(ui->pushButton_load_8bit, SIGNAL(clicked()), this, SLOT(MaleBild8Bit()));
    connect(ui->pushButton_load_12bit, SIGNAL(clicked()), this, SLOT(MaleBild12Bit()));
    connect(ui->pushButton_load_3D, SIGNAL(clicked()), this, SLOT(Male3D()));
    connect(ui->pushButton_tiefenkarte, SIGNAL(clicked()), this, SLOT(MaleTiefenkarte()));
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
    m_pImageData = new short[CT_schichten * imHeight * imWidth];
    m_ptiefenkarte = new short[imHeight * imWidth];
}

Widget::~Widget()
{
    delete ui;
    // Speicher wieder freigeben
    delete[] m_pImageData;
    delete[] m_ptiefenkarte;
}

void Widget::MaleBild8Bit()
{
    // Erzeuge ein Objekt vom Typ Image
    QImage image(imWidth, imHeight, QImage::Format_RGB32);

    // Initialisiere das Bild mit schwarzem Hintergrund
    image.fill(qRgb(0, 0, 0));

    // Speicher (Array) für Bilddaten auf dem Stack reservieren/bereitstellen
    char imageData[imHeight * imWidth];

    // QFileDialog zum Auswählen von .raw Bilddateien öffnen und Datei auswählen
    QString imagePath = QFileDialog::getOpenFileName(this,
                                                     "Open Image",
                                                     "../../",
                                                     "Raw Image Files (*.raw)");
    // Datei im Lesemodus öffnen
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);

    // check if the file could be opened
    if (!bFileOpen) {
        QMessageBox::critical(this, "ACHTUNG", "Datei konnte nicht geöffnet werden");
        return;
    }

    // Bilddaten in Array einlesen
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read(imageData, imHeight * imWidth);

    if (iFileSize != iNumberBytesRead) {
        QMessageBox::critical(this, "ACHTUNG", "Fehler beim Einlesen der Datei");
        return;
    }
    // Überprüfen, ob Anzahl eingelesener Bytes der erwarteten Anzahl entsprechen (512*512)
    if (iNumberBytesRead != imHeight * imWidth) {
        QMessageBox::critical(
            this,
            "ACHTUNG",
            "Anzahl eingelesener Bytes entspricht nicht der erwarteten Anzahl (512*512)");
        return;
    }

    // Datei schließen
    dataFile.close();

    // Setze Inhalt des Arrays Pixel für Pixel in das Bild
    int index, iGrauwert;
    for (int y = 0; y < imHeight; ++y) {
        for (int x = 0; x < imWidth; ++x) {
            // Berechne den zugehörigen index des Speichers
            index = y * imWidth + x;
            // Grauwert an dem index aus imageData auslesen
            iGrauwert = imageData[index];
            // Grauwert als Pixel an der Position x, y im image setzen
            image.setPixel(x, y, qRgb(iGrauwert, iGrauwert, iGrauwert));
        }
    }

    // Bild auf Benutzeroberfläche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));

    // Deactivate the sliders and hide their value for the 8bit image
    hideInputs();
}

void Widget::MaleBild12Bit()
{
    // QFileDialog zum Auswählen von .raw Bilddateien öffnen und Datei auswählen
    QString imagePath = QFileDialog::getOpenFileName(this,
                                                     "Open Image",
                                                     "../../",
                                                     "Raw Image Files (*.raw)");
    // Datei im Lesemodus öffnen
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);

    // check if the file could be opened
    if (!bFileOpen) {
        QMessageBox::critical(this, "ACHTUNG", "Datei konnte nicht geöffnet werden");
        return;
    }

    // Bilddaten in Array einlesen
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char *) m_pImageData, imHeight * imWidth * sizeof(short));

    if (iFileSize != iNumberBytesRead) {
        QMessageBox::critical(this, "ACHTUNG", "Fehler beim Einlesen der Datei");
        return;
    }
    // Überprüfen, ob Anzahl eingelesener Bytes der erwarteten Anzahl entsprechen (512*512)
    if (iNumberBytesRead != imHeight * imWidth * sizeof(short)) {
        QMessageBox::critical(
            this,
            "ACHTUNG",
            "Anzahl eingelesener Bytes entspricht nicht der erwarteten Anzahl (512*512)");
        return;
    }

    // Datei schließen
    dataFile.close();

    // Bild generieren und auf die GUI anzeigen
    updateSliceView();
    // Slider aktivieren und Default-Werte anzeigen
    showInputs();
    // TODO Here the choose schicht slider gets shown too, even though it isn't needed
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
    if (ui->slider_schicht->isVisible()) {
        schicht = ui->slider_schicht->value();
    }
    int schwellenwert = 0;
    if (ui->slider_schwellenwert->isVisible()) {
        schwellenwert = ui->slider_schwellenwert->value();
    }

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
    ui->slider_windowing_center->setVisible(false);
    ui->slider_windowing_width->setVisible(false);
    ui->lineEdit_center->setVisible(false);
    ui->lineEdit_width->setVisible(false);
    ui->label_center->setVisible(false);
    ui->label_width->setVisible(false);

    ui->lineEdit_schicht->setVisible(false);
    ui->slider_schicht->setVisible(false);
    ui->label_schicht->setVisible(false);

    ui->label_schwellenwert->setVisible(false);
    ui->slider_schwellenwert->setVisible(false);
    ui->spinBox_schwellenwert->setVisible(false);

    ui->pushButton_tiefenkarte->setDisabled(true);
}

void Widget::showInputs()
{
    ui->slider_windowing_center->setVisible(true);
    ui->slider_windowing_width->setVisible(true);
    ui->lineEdit_center->setVisible(true);
    ui->lineEdit_width->setVisible(true);
    ui->label_center->setVisible(true);
    ui->label_width->setVisible(true);

    ui->lineEdit_center->setText(QString::number(ui->slider_windowing_center->value()));
    ui->lineEdit_width->setText(QString::number(ui->slider_windowing_width->value()));

    ui->lineEdit_schicht->setVisible(true);
    ui->slider_schicht->setVisible(true);
    ui->label_schicht->setVisible(true);
    ui->lineEdit_schicht->setText(QString::number(ui->slider_schicht->value()));

    ui->label_schwellenwert->setVisible(true);
    ui->slider_schwellenwert->setVisible(true);
    ui->spinBox_schwellenwert->setVisible(true);
    ui->spinBox_schwellenwert->setValue(ui->slider_schwellenwert->value());

    ui->pushButton_tiefenkarte->setDisabled(false);
}

void Widget::Male3D()
{
    // QFileDialog zum Auswählen von .raw Bilddateien öffnen und Datei auswählen
    QString imagePath = QFileDialog::getOpenFileName(this,
                                                     "Open Image",
                                                     "../../",
                                                     "Raw Image Files (*.raw)");
    // Datei im Lesemodus öffnen
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);

    // check if the file could be opened
    if (!bFileOpen) {
        QMessageBox::critical(this, "ACHTUNG", "Datei konnte nicht geöffnet werden");
        return;
    }

    // Bilddaten in Array einlesen
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char *) m_pImageData,
                                         CT_schichten * imHeight * imWidth * sizeof(short));

    if (iFileSize != iNumberBytesRead) {
        QMessageBox::critical(this, "ACHTUNG", "Fehler beim Einlesen der Datei");
        return;
    }
    // Überprüfen, ob Anzahl eingelesener Bytes der erwarteten Anzahl entsprechen (512*512)
    if (iNumberBytesRead != CT_schichten * imHeight * imWidth * sizeof(short)) {
        QMessageBox::critical(
            this,
            "ACHTUNG",
            "Anzahl eingelesener Bytes entspricht nicht der erwarteten Anzahl (130*512*512)");
        return;
    }

    // Datei schließen
    dataFile.close();

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
                if (m_pImageData[index + layer * imageSize] >= threshold) {
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

void Widget::MaleTiefenkarte()
{
    if (this->imageDrawn == false)
        return;
    int schwellenwert = 0;
    if (ui->slider_schwellenwert->isVisible()) {
        schwellenwert = ui->slider_schwellenwert->value();
    }
    // Erzeuge ein Objekt vom Typ Image
    QImage image(imWidth, imHeight, QImage::Format_RGB32);

    // Initialisiere das Bild mit schwarzem Hintergrund
    image.fill(qRgb(0, 0, 0));

    calculateDepthBuffer(m_pImageData,
                         imWidth,
                         imHeight,
                         CT_schichten,
                         schwellenwert,
                         m_ptiefenkarte);

    int iGrauwert;
    for (int y = 0; y < imHeight; ++y) {
        for (int x = 0; x < imWidth; ++x) {
            // Tiefenwert an dem index aus imageData auslesen
            iGrauwert = m_ptiefenkarte[y * imWidth + x];
            /* Um zu überprüfen, ob die Berechnung der Tiefenkarte funktioniert hat, könnt ihr die
             * Tiefenkarte nach der Berechnung (in der der Slot-Funktion) in einem zusätzlichen Label
             * darstellen, indem der Tiefenwert direkt als Grauwert angezeigt wird.
            */
            image.setPixel(x, y, qRgb(iGrauwert, iGrauwert, iGrauwert));
        }
    }

    // Bild auf Benutzeroberfläche anzeigen
    ui->label_image3D->setPixmap(QPixmap::fromImage(image));
}
