#include "widget.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "./ui_widget.h"
#include <qvalidator.h>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->pushButton_load_8bit, SIGNAL(clicked()), this, SLOT(MaleBild8Bit()));
    connect(ui->pushButton_load_12bit, SIGNAL(clicked()), this, SLOT(MaleBild12Bit()));
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

    hideInputs();

    // Speicher der Größe 512*512 reservieren
    m_pImageData = new short[512 * 512];
}

Widget::~Widget()
{
    delete ui;
    // Speicher wieder freigeben
    delete[] m_pImageData;
}

void Widget::MaleBild8Bit()
{
    // Erzeuge ein Objekt vom Typ Image
    QImage image(512, 512, QImage::Format_RGB32);

    // Initialisiere das Bild mit schwarzem Hintergrund
    image.fill(qRgb(0, 0, 0));

    // Speicher (Array) für Bilddaten auf dem Stack reservieren/bereitstellen
    char imageData[512 * 512];

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
    int iNumberBytesRead = dataFile.read(imageData, 512 * 512);

    if (iFileSize != iNumberBytesRead) {
        QMessageBox::critical(this, "ACHTUNG", "Fehler beim Einlesen der Datei");
        return;
    }
    // Überprüfen, ob Anzahl eingelesener Bytes der erwarteten Anzahl entsprechen (512*512)
    if (iNumberBytesRead != 512 * 512) {
        QMessageBox::critical(
            this,
            "ACHTUNG",
            "Anzahl eingelesener Bytes entspricht nicht der erwarteten Anzahl (512*512)");
        return;
    }

    // Datei schließen
    dataFile.close();

    // Setze Inhalt des Arrays Pixel für Pixel in das Bild
    for (int y = 0; y < 512; ++y) {
        for (int x = 0; x < 512; ++x) {
            // Berechne den zugehörigen index des Speichers
            int index = y * 512 + x;
            // Grauwert an dem index aus imageData auslesen
            int iGrauwert = imageData[index];
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
    int iNumberBytesRead = dataFile.read((char *) m_pImageData, 512 * 512 * sizeof(short));

    if (iFileSize != iNumberBytesRead) {
        QMessageBox::critical(this, "ACHTUNG", "Fehler beim Einlesen der Datei");
        return;
    }
    // Überprüfen, ob Anzahl eingelesener Bytes der erwarteten Anzahl entsprechen (512*512)
    if (iNumberBytesRead != 512 * 512 * sizeof(short)) {
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
}

int Widget::windowing(int HU_value, int center, int width)
{
    int iGrauwert;

    //Fensterung berechnen
    int window_min = center - width / 2;
    int window_max = center + width / 2;
    if (HU_value < window_min) {
        iGrauwert = 0;
    } else if (HU_value > window_max) {
        iGrauwert = 255;
    } else {
        iGrauwert = static_cast<int>(255.0 * (HU_value - (center - width / 2)) / width);
    }

    return iGrauwert;
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
    // Erzeuge ein Objekt vom Typ Image
    QImage image(512, 512, QImage::Format_RGB32);

    // Initialisiere das Bild mit schwarzem Hintergrund
    image.fill(qRgb(0, 0, 0));

    // Setze Inhalt des Arrays Pixel für Pixel in das Bild
    for (int y = 0; y < 512; ++y) {
        for (int x = 0; x < 512; ++x) {
            // Berechne den zugehörigen index des Speichers
            int index = y * 512 + x;
            // Grauwert an dem index aus imageData auslesen
            int iGrauwert = windowing(m_pImageData[index],
                                      ui->slider_windowing_center->value(),
                                      ui->slider_windowing_width->value());
            //int iGrauwert = windowing(m_pImageData[index], 600, 1200);
            //int iGrauwert = windowing(m_pImageData[index], 0, 800);
            // Grauwert als Pixel an der Position x, y im image setzen
            image.setPixel(x, y, qRgb(iGrauwert, iGrauwert, iGrauwert));
        }
    }

    // Bild auf Benutzeroberfläche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
    this->imageDrawn = true;
}

void Widget::hideInputs()
{
    ui->slider_windowing_center->setVisible(false);
    ui->slider_windowing_width->setVisible(false);
    ui->lineEdit_center->setVisible(false);
    ui->lineEdit_width->setVisible(false);
    ui->label_center->setVisible(false);
    ui->label_width->setVisible(false);
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
}
