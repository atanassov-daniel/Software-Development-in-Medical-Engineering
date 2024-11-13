#include "CTDataset.h"
#include <QFile>
#include <QMessageBox>

const int imWidth = 512;
const int imHeight = 512;
const int CT_schichten = 130;

CTDataset::CTDataset()
{
    m_pImageData = new short[CT_schichten * imHeight * imWidth];
}

CTDataset::~CTDataset()
{
    delete[] m_pImageData;
}

short *CTDataset::data()
{
    return m_pImageData;
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
