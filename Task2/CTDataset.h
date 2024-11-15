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

private:
    short *m_pImageData;
};
