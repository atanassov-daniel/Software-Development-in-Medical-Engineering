#pragma once

#if defined(MYLIB_EXPORT)
#define MYLIB_EXPORT_IMPORT Q_DECL_EXPORT
#else
#define MYLIB_EXPORT_IMPORT Q_DECL_IMPORT
#endif

#include <QString>

enum class ReturnCode { OK, HU_OUT_OF_RANGE, CENTER_OUT_OF_RANGE, WIDTH_OUT_OF_RANGE };

/**
 * @brief The CTDataset class provides functionality for working with a CT image dataset.
 * @details A CT image dataset can be loaded and then subsequently manipulated, for example a HU value can be linearly windowed to a grey value with a given window center and width, a depth and shaded buffer can be created to provide the needed data to render a 3D view
*/
class MYLIB_EXPORT_IMPORT CTDataset
{
public:
    /** @brief Creates a CTDataset instance, with which CT datasets of the inputted dimensions can be manipulated */
    CTDataset(int imWidth, int imHeight, int CT_layers);
    ~CTDataset();
    /** @brief Loads a CT image dataset of the previously configured dimensions */
    void load(QString imagePath);
    /** @brief Returns a pointer to the loaded raw CT image data */
    short *data() const;
    /** @brief Returns a pointer to the depth buffer */
    short *depthBuffer() const;
    /** @brief Returns a pointer to the shaded buffer needed for the 3D rendering */
    short *shadedBuffer() const;
    /** @brief Checks if the depth buffer has already been created */
    bool existsDepthBuffer() const;
    /** @brief Calculates the linear windowing of the given HU value with the provided window center and width */
    static ReturnCode windowing(int HU_value, int windowCenter, int windowWidth, int &greyValue);
    /** @brief renders a 3D view of the CT dataset for the given threshold */
    int renderDepthBuffer(int threshold);

private:
    /** @brief The width of a single CT image. Once set, it can't be changed. */
    const int imWidth;
    /** @brief The height of a single CT image. Once set, it can't be changed. */
    const int imHeight;
    /** @brief The number of layers included in the CT dataset. Once set, it can't be changed. */
    const int CT_layers;
    /** @brief pointer to the array where all the CT dataset data is stored */
    short *m_pImageData;
    /** @brief pointer to the array where the depth buffer is stored */
    short *m_ptiefenkarte;
    /** @brief pointer to the array where the shaded buffer for the 3D rendering is stored */
    short *m_pshadedBuffer;
    /** @brief stores if the depth buffer has already been calculated */
    bool isDepthBufferCreated;
    /** @brief calculates the depth buffer needed in renderDepthBuffer() */
    int calculateDepthBuffer(int threshold);
};
