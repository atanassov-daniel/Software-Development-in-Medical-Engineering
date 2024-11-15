#pragma once

#include <QString>

enum class ReturnCode { OK, HU_OUT_OF_RANGE, CENTER_OUT_OF_RANGE, WIDTH_OUT_OF_RANGE };

class CTDataset
{
public:
    CTDataset();
    ~CTDataset();
    void load(QString imagePath);
    short *data();
    static ReturnCode windowing(int HU_value, int windowCenter, int windowWidth, int &greyValue);
    int renderDepthBuffer(int width, int height, int layers, int threshold);
    bool existsDepthBuffer();
    short *depthBuffer();
    short *shadedBuffer();

private:
    short *m_pImageData;
    short *m_ptiefenkarte;
    short *m_pshadedBuffer;
    bool isDepthBufferCreated;
    int calculateDepthBuffer(int width, int height, int layers, int threshold);
};
