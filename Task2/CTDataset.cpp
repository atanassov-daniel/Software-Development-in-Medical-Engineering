#include "CTDataset.h"
#include <QFile>
#include <QMessageBox>

const int imWidth = 512;
const int imHeight = 512;
const int CT_schichten = 130;

CTDataset::CTDataset()
{
    // Speicher reservieren
    m_pImageData = new short[CT_schichten * imHeight * imWidth];
    m_ptiefenkarte = new short[imHeight * imWidth];
    m_pshadedBuffer = new short[imHeight * imWidth];
    isDepthBufferCreated = false;
}

CTDataset::~CTDataset()
{
    // Speicher wieder freigeben
    delete[] m_pImageData;
    delete[] m_ptiefenkarte;
    delete[] m_pshadedBuffer;
}

short *CTDataset::data()
{
    return m_pImageData;
}

short *CTDataset::depthBuffer()
{
    return m_ptiefenkarte;
}

short *CTDataset::shadedBuffer()
{
    return m_pshadedBuffer;
}

bool CTDataset::existsDepthBuffer()
{
    return isDepthBufferCreated;
}

void CTDataset::load(QString imagePath)
{
    // Datei im Lesemodus öffnen
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);

    // check if the file could be opened
    if (!bFileOpen) {
        throw QString("Datei konnte nicht geöffnet werden");
    }

    // Bilddaten in Array einlesen
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char *) m_pImageData,
                                         CT_schichten * imHeight * imWidth * sizeof(short));

    if (iFileSize != iNumberBytesRead) {
        throw QString("Fehler beim Einlesen der Datei");
        // TODO if a 3D view has been loaded, then there is a try to load an invalid file, and then the schwellenwert is changed, it becomes a really weird thing to look at, because some of the bytes from the invalid file still get read into the array where the previous valid file data is stored, the easiest way to fix it would probably be to just flush the image labels empty and set the array for the image data to a nullptr, and then deactivate inputs
    }
    // Überprüfen, ob Anzahl eingelesener Bytes der erwarteten Anzahl entsprechen (512*512)
    if (iNumberBytesRead != CT_schichten * imHeight * imWidth * sizeof(short)) {
        throw QString("Anzahl eingelesener Bytes entspricht nicht der erwarteten Anzahl (%1*%2*%3)")
            .arg(QString::number(CT_schichten), QString::number(imHeight), QString::number(imWidth));
    }

    // Datei schließen
    dataFile.close();
}

ReturnCode CTDataset::windowing(int HU_value, int windowCenter, int windowWidth, int &greyValue)
{
    if (HU_value < -1024 || HU_value > 3071) {
        return ReturnCode::HU_OUT_OF_RANGE;
    }
    if (windowCenter < -1024 || windowCenter > 3071) {
        return ReturnCode::CENTER_OUT_OF_RANGE;
    }
    if (windowWidth < 1 || windowWidth > 4095) {
        return ReturnCode::WIDTH_OUT_OF_RANGE;
    }
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

    return ReturnCode::OK;
}

/*
die Parameter width, height und layers bestimmen jeweils die Breite, Höhe und Tiefe des CT-Datensatzes,
welcher durch den Zeiger inputData übergeben wird. threshold ist der Schwellenwert in HU.
depthBuffer ist ein (vorher anzulegender!) Speicherbereich, in dem die
Tiefenkarte der Größe width*height als Rückgabewert gespeichert wird.
*/
int CTDataset::calculateDepthBuffer(int width, int height, int layers, int threshold)
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
                    m_ptiefenkarte[index] = layer;
                    isDepthSet = true;
                    break;
                }
            }
            /* if for some pixel (x, y) there is no layer found where the HU value is bigger
             * than the threshold, then set the depth to either (layers) or (layers - 1)     (source: lecture)
            */
            if (!isDepthSet)
                m_ptiefenkarte[index] = layers;
        }
    }

    return 0;
}

int CTDataset::renderDepthBuffer(int width, int height, int layers, int threshold)
{
    calculateDepthBuffer(width, height, layers, threshold);
    /*
     * Bei einer Schrittweite von 2 fällt der erste und letzte Pixel im Bild weg, das heißt,
     * das Bild wird nur im Bereich zwischen 1 und 510 gezeichnet.
    */
    /*const int s_x = 2;
    const int s_y = 2;*/
    int T_x, T_y;
    for (int y = 1; y < width - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            // Berechne für jeden Pixel den normierten Intensitätswert I_reflektiert, welcher direkt der anzuzeigende Grauwert ist
            T_x = m_ptiefenkarte[y * width + x - 1] - m_ptiefenkarte[y * width + x + 1];
            T_y = m_ptiefenkarte[(y - 1) * width + x] - m_ptiefenkarte[(y + 1) * width + x];
            // calculate I_reflektiert and store the value in the shadedBuffer
            /*shadedBufer[y * imWidth + x] = 255.0 * (s_x * s_y)
                                           / sqrt(pow((s_y * T_x), 2) + pow((s_x * T_y), 2)
                                                  + pow((s_x * s_y), 2));*/
            m_pshadedBuffer[y * width + x] = 255.0 * 4
                                               / sqrt(pow((2 * T_x), 2) + pow((2 * T_y), 2) + 16);
        }
    }
    isDepthBufferCreated = true;

    return 0;
}
