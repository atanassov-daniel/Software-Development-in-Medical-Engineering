#include "CTDataset.h"
#include <QElapsedTimer>
#include <QFile>
#include <QMessageBox>
#include <thread> // std::thread

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
die Parameter width, height und layers bestimmen jeweils die Breite, Höhe und Tiefe des CT-Datensatzes, threshold ist der Schwellenwert in HU.
*/
int CTDataset::calculateDepthBuffer(int width, int height, int layers, int threshold)
{
    QElapsedTimer timer;
    timer.start();

    const int imageSize = (width * height);

    auto lambdaFun = [&](int threadIndex, int numThreads) {
        int index;
        for (int y = (height / numThreads) * threadIndex;
             y < (height / numThreads) * (threadIndex + 1);
             ++y) {
            for (int x = 0; x < width; ++x) {
                // Berechne den zugehörigen index des Speichers
                index = y * width + x;

                bool isDepthSet = false;
                for (int layer = 0; layer < layers; layer++) {
                    if (m_pImageData[index + layer * imageSize] >= threshold) {
                        m_ptiefenkarte[index] = layer;
                        isDepthSet = true;
                        break;
                    }
                }
                // if for some pixel (x, y) there is no layer found where the HU value is bigger than the threshold, then set the depth to either (layers) or (layers - 1)     (source: lecture)
                if (!isDepthSet)
                    m_ptiefenkarte[index] = layers;
            }
        }
    };

    int numThreads = static_cast<int>(std::thread::hardware_concurrency() * 0.7);
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(lambdaFun, i, numThreads);
    }
    for (auto &th : threads) {
        th.join();
    }
    qDebug() << "calculateDepthBuffer with threads runtime: " << timer.nsecsElapsed();

    return 0;
}
/*
calculateDepthBuffer with threads runtime:
(130975300 + 115650500 + 127856900 + 143733700 + 144293100 + 140935300 + 147231900 + 150032700 + 145633000 + 142795000 + 142374400 + 153954800 + 185957000 + 215892600 + 196779500 + 209705600 + 212544100 + 206829000 + 203952100 + 210840700 + 120031000 + 109788700 + 20715300 + 7525300 + 4378500 + 6456100 + 111707700 + 106523100 + 106267100 + 4203000 + 6724800 + 21479700 + 8617100 + 21747600 + 90401100 + 103546300 + 116810400 + 102017200 + 120985900 + 114697200 + 110652300 + 107648400 + 107267400 + 109510600 + 109051200 + 105873100 + 107172900 + 104057500 + 107001000 + 113448900 + 115150700 + 104557300 + 115464000 + 111598000 + 122671800 + 115274700 + 121961000 + 138556600 + 141177400 + 143038500 + 136124800 + 143947200 + 143853600 + 142819400 + 144544000 + 139517100 + 143172800) / 67
= 118473171.642 ns = 118.47 ms

calculateDepthBuffer without threads runtime:
(486038000 + 354232300 + 374982300 + 483433000 + 359807500 + 367947800 + 480836900 + 355914800 + 391152800 + 491589500 + 360086700 + 410720200 + 495876000 + 497187700 + 747332000 + 750615200 + 726609400 + 604684000) / 18
= 485502561.111 ns = 485.5 ms

Conclusion: when testing on my computer the solution with threads was around 4 times faster
*/

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
