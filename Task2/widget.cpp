#include "widget.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "./ui_widget.h"
#include <iostream>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->pushButton_load_8bit, SIGNAL(clicked()), this, SLOT(MaleBild8Bit()));
    connect(ui->pushButton_load_12bit, SIGNAL(clicked()), this, SLOT(MaleBild12Bit()));

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
}

void Widget::MaleBild12Bit()
{
    // Erzeuge ein Objekt vom Typ Image
    QImage image(512, 512, QImage::Format_RGB32);

    // Initialisiere das Bild mit schwarzem Hintergrund
    image.fill(qRgb(0, 0, 0));

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

    // Setze Inhalt des Arrays Pixel für Pixel in das Bild
    for (int y = 0; y < 512; ++y) {
        for (int x = 0; x < 512; ++x) {
            // Berechne den zugehörigen index des Speichers
            int index = y * 512 + x;
            // Grauwert an dem index aus imageData auslesen
            //int iGrauwert = windowing(m_pImageData[index], 0, 800);
            int iGrauwert = windowing(m_pImageData[index], 600, 1200);
            // Grauwert als Pixel an der Position x, y im image setzen
            image.setPixel(x, y, qRgb(iGrauwert, iGrauwert, iGrauwert));
        }
    }

    // Bild auf Benutzeroberfläche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
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
