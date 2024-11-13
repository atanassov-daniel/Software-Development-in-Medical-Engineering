#pragma once

#include <QString>

class CTDataset
{
public:
    CTDataset();
    ~CTDataset();
    void load(QString imagePath);
    short* data();

private:
    short *m_pImageData;
};
