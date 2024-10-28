#include "widget.h"
//#include <QDebug>
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->pushButton_Pixel, SIGNAL(clicked()), this, SLOT(MalePixel()));
    connect(ui->pushButton_RoteHorizLinie, SIGNAL(clicked()), this, SLOT(MaleRoteHorizLinie()));
    connect(ui->pushButton_GrueneVertikLinie,
            SIGNAL(clicked()),
            this,
            SLOT(MaleGrueneVertikLinie()));
    connect(ui->pushButton_Kreuz, SIGNAL(clicked()), this, SLOT(MaleKreuz()));
    connect(ui->pushButton_SchraegeLinie, SIGNAL(clicked()), this, SLOT(MaleSchraegeLinie()));
    connect(ui->pushButton_AllesRot, SIGNAL(clicked()), this, SLOT(MaleAllesRot()));
    connect(ui->pushButton_Quadrat, SIGNAL(clicked()), this, SLOT(MaleQuadrat()));
    connect(ui->pushButton_Farbverlauf, SIGNAL(clicked()), this, SLOT(MaleFarbverlauf()));
    connect(ui->pushButton_Kreuz, SIGNAL(clicked()), this, SLOT(MaleKreis()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::MalePixel()
{
    // Erzeuge ein Objekt vom Typ QImage
    QImage image(512, 512, QImage::Format_RGB32);

    // Zuerst initialisieren wir das Bild mit einer Farbe (hier schwarz)
    image.fill(qRgb(0, 0, 0));

    // Schreibe einen roten Pixel
    // Beachte: Das Qt Koordinatensystem hat den Pixel (0, 0) in der linken oberen Ecke des Bildes
    image.setPixel(50, 100, qRgb(255, 0, 0));

    // QImage is designed and optimized for I/O, and for direct pixel access and manipulation, while QPixmap is designed and optimized for showing images on screen.
    // Bild auf Benutzeroberflaeche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
}

void Widget::MaleRoteHorizLinie() // Draws a red horizontal line
{
    int height = 512, width = 512;
    // Erzeuge ein Objekt vom Typ QImage
    QImage image(width, height, QImage::Format_RGB32);

    // Zuerst initialisieren wir das Bild mit einer Farbe (hier schwarz)
    image.fill(qRgb(0, 0, 0));

    // Male eine rote horizontale Linie in etwa der Mitte des Bildes
    int y = height / 2;
    for (int x = 0; x < width; x++) {
        image.setPixel(x, y, qRgb(255, 0, 0));
    }

    // Bild auf Benutzeroberflaeche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
}
void Widget::MaleGrueneVertikLinie() // Draws a green vertical line
{
    int height = 512, width = 512;
    // Erzeuge ein Objekt vom Typ QImage
    QImage image(width, height, QImage::Format_RGB32);

    // Zuerst initialisieren wir das Bild mit einer Farbe (hier schwarz)
    image.fill(qRgb(0, 0, 0));

    // Male eine grüne vertikale Linie in etwa der Mitte des Bildes
    int x = width / 2;
    for (int y = 0; y < width; y++) {
        image.setPixel(x, y, qRgb(0, 255, 0));
    }

    // Bild auf Benutzeroberflaeche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
}
void Widget::MaleKreuz() // ein Kreuz der Strichlänge 5 (und Strichstärke 2) malen
{}
void Widget::MaleSchraegeLinie() // Draws a blue diagonal line with a slope of y = 2*x
{
    int height = 512, width = 512;
    // Erzeuge ein Objekt vom Typ QImage
    QImage image(width, height, QImage::Format_RGB32);

    // Zuerst initialisieren wir das Bild mit einer Farbe (hier schwarz)
    image.fill(qRgb(0, 0, 0));

    // x only goes from 0 until width/2, because after that y=2*x would be out of bounds of the pixmap image size
    /*
     * This was my first try for doing the task, but because the Qt coordinate
     * system has its start point (0, 0) in the upper left corner of the screen,
     * the output didn't look as it would in a cartesian coordinate system,
     * like for example the graph drawn by GeoGebra for the function y = 2*x
        for (int x = 0; x < width / 2; x++) {
            image.setPixel(x, 2 * x, qRgb(0, 0, 255));
        }
    */

    // To account for Qt's start coordinate in the upper left corner, the y coordinate needs to be inverted
    for (int x = 0; x < width / 2; ++x) {
        int y = 2 * x;
        int invertedY = height - 1 - y; // Invert y to make the origin at the bottom-left
        if (invertedY >= 0) {
            image.setPixel(x, invertedY, qRgb(0, 0, 255));
        }
    }

    // Bild auf Benutzeroberflaeche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
}
void Widget::MaleAllesRot() // Colors the entire image area red
{
    int height = 512, width = 512;
    // Erzeuge ein Objekt vom Typ QImage
    QImage image(width, height, QImage::Format_RGB32);

    // Initialisiere das Bild direkt mit der Farbe rot
    image.fill(qRgb(255, 0, 0));

    // Bild auf Benutzeroberflaeche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
}
void Widget::MaleQuadrat() // draws a square with length 50 in one color, but with a different border color
{
    int height = 512, width = 512;
    // Erzeuge ein Objekt vom Typ QImage
    QImage image(width, height, QImage::Format_RGB32);

    // Zuerst initialisieren wir das Bild mit einer Farbe (hier schwarz)
    image.fill(qRgb(0, 0, 0));

    // I decided I want to draw the square at the middle of the image, ie. the middle point of the square should be the middle of the image
    int xMiddlePixel = width / 2 - 1;
    int yMiddlePixel = height / 2 - 1; // -1 because the first pixel has coordinate 0
    int sideLength = 50;

    for (int x = xMiddlePixel - sideLength / 2; x < xMiddlePixel + sideLength / 2; x++) {
        for (int y = yMiddlePixel - sideLength / 2; y < yMiddlePixel + sideLength / 2; y++) {
            // qDebug() << "x: " << x << " y: " << y;
            if (x == xMiddlePixel - sideLength / 2 || x == xMiddlePixel + sideLength / 2 - 1
                || y == yMiddlePixel - sideLength / 2 || y == yMiddlePixel + sideLength / 2 - 1) {
                image.setPixel(x, y, qRgb(0, 255, 0));
            } else {
                image.setPixel(x, y, qRgb(255, 0, 255));
            }
        }
    }

    // Bild auf Benutzeroberflaeche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
}
void Widget::MaleFarbverlauf() // change the color value depending on the pixel position, generating a color gradient
{
    int height = 512, width = 512;
    // Erzeuge ein Objekt vom Typ QImage
    QImage image(width, height, QImage::Format_RGB32);

    // Zuerst initialisieren wir das Bild mit einer Farbe (hier schwarz)
    image.fill(qRgb(0, 0, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Adjust the RGB values based on x and y positions for a smoother gradient
            int red = static_cast<int>((255.0 * x) / width);
            int green = static_cast<int>((255.0 * y) / height);
            int blue = static_cast<int>((255.0 * (x + y)) / (width + height));
            image.setPixel(x, y, qRgb(red, green, blue));
        }
    }

    /*
    // a relatively nice looking thing, just not exactly what the task was:
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            image.setPixel(x, y, qRgb(x % 256, y % 256, (y - x) % 256));
        }
    }
    * Because the RGB values are in the range [0, 255], when x or y gets to 256, the values start from 0 again,
    * that's why in every quadrant of the image the same gradient repeats. If the modulo operation wasn't used,
    * the same effect would happen, because in an 8-bit-per--channel image 8 bit storage is used for the colors,
    * so the values jsut overflow back to 0 after reaching more than 255
    *
    * Since both the x and y coordinates are "reset" to values between 0 and 255 every 256 pixels, each quadrant of the 512x512 image (256x256
    * pixels) ends up showing the same gradient. Here's how it looks broken down into quadrants:
        Top-left: x and y range from 0 to 255, showing the initial gradient pattern.
        Top-right: x is in the range 256–511, but x % 256 resets to 0–255, repeating the pattern.
        Bottom-left: y is in the range 256–511, but y % 256 resets, repeating the pattern vertically.
        Bottom-right: Both x and y are reset by % 256, so the pattern repeats horizontally and vertically.
    */

    // Bild auf Benutzeroberflaeche anzeigen
    ui->label_image->setPixmap(QPixmap::fromImage(image));
}
void Widget::MaleKreis() // paint a circular area a different color than the rest of the image
{}
