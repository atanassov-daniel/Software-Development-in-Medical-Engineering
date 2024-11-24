#include <QString>
#include <QtTest>
#include "ctdataset.h"
#include <algorithm>

class MyLibTestClass : public QObject
{
    Q_OBJECT

public:
    MyLibTestClass() {};

private Q_SLOTS:
    void initTestCase();
    void windowingTest();
    void cleanupTestCase();
};

void MyLibTestClass::initTestCase()
{
    // if necessary do some initialization stuff here before the tests start
}

void MyLibTestClass::cleanupTestCase()
{
    // if necessary do some clean up stuff here after all tests have finished
}

void MyLibTestClass::windowingTest()
{
    // VALID case 1: testing clean zero for bottom HU boundary
    ReturnCode retCode;
    int greyValue = 0;
    retCode = CTDataset::windowing(-34, -34 + 50, 100, greyValue);
    QVERIFY2(retCode == ReturnCode::OK, "returns an error although input is valid");
    QVERIFY2(greyValue == 0, "windowing function lower bound");

    // VALID case 2: testing center of windowed domain
    greyValue = -1;
    retCode = CTDataset::windowing(50, 50, 100, greyValue);
    QVERIFY2(retCode == ReturnCode::OK, "returns an error although input is valid");
    QVERIFY2(greyValue == 128,
             qPrintable(
                 QString("windowing function medium value, was %1 instead of 128").arg(greyValue)));

    // INVALID case 1: HU input too low
    retCode = CTDataset::windowing(-4100, -1000, 2000, greyValue);
    QVERIFY2(retCode == ReturnCode::HU_OUT_OF_RANGE,
             "No error code returned although input HU value was <-1024");

    // ADD FURTHER Testcases here

    // INVALID case 1.2: HU input too high
    retCode = CTDataset::windowing(3072, 0, 0, greyValue);
    QVERIFY2(retCode == ReturnCode::HU_OUT_OF_RANGE,
             "No error code returned although input HU value was >3071");
    // INVALID case 2.1: windowCenter input too low
    retCode = CTDataset::windowing(0, -1025, 0, greyValue);
    QVERIFY2(retCode == ReturnCode::CENTER_OUT_OF_RANGE,
             "No error code returned although input window center value was <-1024");
    // INVALID case 2.2: windowCenter input too high
    retCode = CTDataset::windowing(0, 3072, 0, greyValue);
    QVERIFY2(retCode == ReturnCode::CENTER_OUT_OF_RANGE,
             "No error code returned although input window center value was >3071");
    // INVALID case 3.1: windowWidth input too small
    retCode = CTDataset::windowing(0, 0, 0, greyValue);
    QVERIFY2(retCode == ReturnCode::WIDTH_OUT_OF_RANGE,
             "No error code returned although input window width value was <1");
    // INVALID case 3.2: windowWidth input too small
    retCode = CTDataset::windowing(0, 0, 4096, greyValue);
    QVERIFY2(retCode == ReturnCode::WIDTH_OUT_OF_RANGE,
             "No error code returned although input window width value was >4095");

    // VALID case 3: check if the endpoints of the ranges of valid values are handled correctly (< vs <=)
    // HU value
    retCode = CTDataset::windowing(3071, 0, 1, greyValue);
    QVERIFY2(retCode == ReturnCode::OK,
             "Error code returned although input HU value was valid");

    retCode = CTDataset::windowing(-1024, 0, 1, greyValue);
    QVERIFY2(retCode == ReturnCode::OK,
             "Error code returned although input HU value was valid");
    // window center
    retCode = CTDataset::windowing(0, -1024, 1, greyValue);
    QVERIFY2(retCode == ReturnCode::OK,
             "Error code returned although input window center value was valid");

    retCode = CTDataset::windowing(0, 3071, 1, greyValue);
    QVERIFY2(retCode == ReturnCode::OK,
             "Error code returned although input window center value was valid");
    // window width
    retCode = CTDataset::windowing(0, 0, 1, greyValue);
    QVERIFY2(retCode == ReturnCode::OK,
             "Error code returned although input window width value was valid");

    retCode = CTDataset::windowing(0, 0, 4095, greyValue);
    QVERIFY2(retCode == ReturnCode::OK,
             "Error code returned although input window width value was valid");

    // VALID case 4: check if the greyValue == 0 when the HU value is smaller than the minimum value of the window
    retCode = CTDataset::windowing(-200, 0, 100, greyValue);
    QVERIFY2(retCode == ReturnCode::OK, "returns an error although input is valid");
    QVERIFY2(greyValue == 0, "windowing function works correctly for HU value < window minimum");
    // VALID case 5: check if the greyValue == 255 when the HU value is bigger than the maximum value of the window
    retCode = CTDataset::windowing(200, 0, 100, greyValue);
    QVERIFY2(retCode == ReturnCode::OK, "returns an error although input is valid");
    QVERIFY2(greyValue == 255, "windowing function works correctly for HU value > window maximum");
    /*
    int window_min = windowCenter - windowWidth / 2;
    int window_max = windowCenter + windowWidth / 2;*/
}

QTEST_APPLESS_MAIN(MyLibTestClass)

#include "tst_mylibunittest.moc"
