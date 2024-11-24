#include "CTDataset.h"
#include <QElapsedTimer>
#include <QFile>
#include <QMessageBox>
#include <thread> // std::thread

/**
 * @param imWidth the width of the images in the CT dataset
 * @param imHeight the height of the images in the CT dataset
 * @param CT_layers the number of CT layers that are included in the loaded RAW file with the CT image data
 */
CTDataset::CTDataset(int imWidth, int imHeight, int CT_layers)
    : imWidth{imWidth}
    , imHeight{imHeight}
    , CT_layers{CT_layers}
{
    // Allocate memory
    m_pImageData = new short[CT_layers * imHeight * imWidth];
    m_ptiefenkarte = new short[imHeight * imWidth];
    m_pshadedBuffer = new short[imHeight * imWidth];
    isDepthBufferCreated = false;
}

CTDataset::~CTDataset()
{
    // Deallocate memory
    delete[] m_pImageData;
    delete[] m_ptiefenkarte;
    delete[] m_pshadedBuffer;
}

short *CTDataset::data() const
{
    return m_pImageData;
}

short *CTDataset::depthBuffer() const
{
    return m_ptiefenkarte;
}

short *CTDataset::shadedBuffer() const
{
    return m_pshadedBuffer;
}
// TODO: maybe make sure that the pointers can only be used when there is actual valid data loaded, otherwise don't let the person that uses this CTDataset library access some random or even wrong memory values

bool CTDataset::existsDepthBuffer() const
{
    return isDepthBufferCreated;
}

/**
 * @throws QString with an error message in case there was a problem with opening/reading the file or the CT dataset isn't of the correct dimensions (for the previously configured image width, height, and number of CT layers)
 * @param imagePath the path to the RAW file with the CT dataset
*/
void CTDataset::load(QString imagePath)
{
    // Open file in read-only mode
    QFile dataFile(imagePath);
    bool bFileOpen = dataFile.open(QIODevice::ReadOnly);

    // check if the file could be opened
    if (!bFileOpen) {
        throw QString("Datei konnte nicht geöffnet werden");
    }

    // Read the image data into the array m_pImageData
    int iFileSize = dataFile.size();
    int iNumberBytesRead = dataFile.read((char *) m_pImageData,
                                         CT_layers * imHeight * imWidth * sizeof(short));

    if (iFileSize != iNumberBytesRead) {
        throw QString("Fehler beim Einlesen der Datei");
        // TODO if a 3D view has been loaded, then there is a try to load an invalid file, and then the schwellenwert is changed, it becomes a really weird thing to look at, because some of the bytes from the invalid file still get read into the array where the previous valid file data is stored, the easiest way to fix it would probably be to just flush the image labels empty and set the array for the image data to a nullptr, and then deactivate inputs
    }
    // Check if the number of bytes read corresponds to the expected number of bytes for the configured dimensions
    if (iNumberBytesRead != CT_layers * imHeight * imWidth * sizeof(short)) {
        throw QString("Anzahl eingelesener Bytes entspricht nicht der erwarteten Anzahl (%1*%2*%3)")
            .arg(QString::number(CT_layers), QString::number(imHeight), QString::number(imWidth));
    }

    // Close the file
    dataFile.close();
}

/**
 * @param HU_value the HU value for which the windowed grey value should be calculated. The value should be in the range [-1024, 3071]
 * @param windowCenter the center of the window, which should be in the range [-1024, 3071]
 * @param windowWidth the width of the window, which should be in the range [1, 4095]
 * @param greyValue the memory location where the calculated windowed grey value (in the range [0, 255]) should be stored
 * @return a **ReturnCode** with one of the following values: [OK, HU_OUT_OF_RANGE, CENTER_OUT_OF_RANGE, WIDTH_OUT_OF_RANGE]
*/
ReturnCode CTDataset::windowing(int HU_value, int windowCenter, int windowWidth, int &greyValue)
{
    // check if the input values are valid
    if (HU_value < -1024 || HU_value > 3071) {
        return ReturnCode::HU_OUT_OF_RANGE;
    }
    if (windowCenter < -1024 || windowCenter > 3071) {
        return ReturnCode::CENTER_OUT_OF_RANGE;
    }
    if (windowWidth < 1 || windowWidth > 4095) {
        return ReturnCode::WIDTH_OUT_OF_RANGE;
    }
    // Calculate the windowed grey value
    int window_min = windowCenter - windowWidth / 2;
    int window_max = windowCenter + windowWidth / 2;
    if (HU_value < window_min) {
        greyValue = 0;
    } else if (HU_value > window_max) {
        greyValue = 255;
    } else {
        greyValue = std::roundf(255.0 * (HU_value - (windowCenter - windowWidth / 2)) / windowWidth);
    }

    return ReturnCode::OK;
}

/**
 * @details calculates the depth buffer for the given threshold HU value. The depth buffer is needed to render a 3D view of the CT dataset in renderDepthBuffer(). The function uses multithreading to improve performance on multi-threaded systems.
 * @param threshold the threshold HU value for which the depth buffer gets calculated
 * @return an error code (TODO)
*/
int CTDataset::calculateDepthBuffer(int threshold)
{
    QElapsedTimer timer;
    timer.start();

    const int imageSize = (imWidth * imHeight);

    auto lambdaFun = [&](int threadIndex, int numThreads) {
        int index;
        //for (int x = (width / numThreads) * threadIndex; x < (width / numThreads) * (threadIndex+1); x++) {
        for (int y = (imHeight / numThreads) * threadIndex;
             y < (imHeight / numThreads) * (threadIndex + 1);
             ++y) {
            for (int x = 0; x < imWidth; ++x) {
                //for (int y = 0; y < height; ++y) {
                // Berechne den zugehörigen index des Speichers
                index = y * imWidth + x;

                bool isDepthSet = false;
                for (int layer = 0; layer < CT_layers; layer++) {
                    if (m_pImageData[index + layer * imageSize] >= threshold) {
                        m_ptiefenkarte[index] = layer;
                        isDepthSet = true;
                        break;
                    }
                }
                // if for some pixel (x, y) there is no layer found where the HU value is bigger than the threshold, then set the depth to either (layers) or (layers - 1)     (source: lecture)
                if (!isDepthSet)
                    m_ptiefenkarte[index] = CT_layers;
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
calculateDepthBuffer with threads runtime (average from 67 runs) = 118.47 ms
calculateDepthBuffer without threads runtime (average from 18 runs) = 485.5 ms
Conclusion: when testing on my computer, the solution with threads was around 4 times faster.
*/

/**
 * @details uses calculateDepthBuffer() to calculate the depth buffer for the given threshold. With the depth buffer, using a raytracing approach, a 3D view of the CT image dataset gets rendered
 * @param threshold the threshold HU value for which the 3D view gets rendered
 * @return an error code (TODO)
*/
int CTDataset::renderDepthBuffer(int threshold)
{
    calculateDepthBuffer(threshold);
    /*
     * Bei einer Schrittweite von 2 fällt der erste und letzte Pixel im Bild weg, das heißt,
     * das Bild wird nur im Bereich zwischen 1 und 510 gezeichnet.
    */
    /*const int s_x = 2;
    const int s_y = 2;*/
    int T_x, T_y;
    for (int y = 1; y < imWidth - 1; ++y) {
        for (int x = 1; x < imWidth - 1; ++x) {
            // Berechne für jeden Pixel den normierten Intensitätswert I_reflektiert, welcher direkt der anzuzeigende Grauwert ist
            T_x = m_ptiefenkarte[y * imWidth + x - 1] - m_ptiefenkarte[y * imWidth + x + 1];
            T_y = m_ptiefenkarte[(y - 1) * imWidth + x] - m_ptiefenkarte[(y + 1) * imWidth + x];
            // calculate I_reflektiert and store the value in the shadedBuffer
            /*shadedBufer[y * imWidth + x] = 255.0 * (s_x * s_y)
                                           / sqrt(pow((s_y * T_x), 2) + pow((s_x * T_y), 2)
                                                  + pow((s_x * s_y), 2));*/
            m_pshadedBuffer[y * imWidth + x] = 255.0 * 4
                                               / sqrt(pow((2 * T_x), 2) + pow((2 * T_y), 2) + 16);
        }
    }
    isDepthBufferCreated = true;

    return 0;
}
