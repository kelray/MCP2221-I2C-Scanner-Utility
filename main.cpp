#include "mainwindow.h"
#include <QTextEdit>
#include <QPushButton>
#include <QDebug>
#include <QApplication>
#include "GetErrName.h"
#include "mcp2221_dll_um.h"

//MCP2221 Variables
#define I2cAddr7bit 1
#define I2cAddr8bit 0
void *handle;
wchar_t SerNum = 0x0000075428;
wchar_t LibVer[6];
wchar_t MfrDescriptor[30];
wchar_t ProdDescrip[30];
wchar_t NewProductDescrip[] = L"Automated Test Utility";
int ver = 0;
int error = 0;
int flag = 0;
unsigned int delay = 0;
unsigned int ReqCurrent;
unsigned int PID = 0xDD;
unsigned int VID = 0x4D8;
unsigned int NumOfDev = 0;
unsigned char PowerAttrib;

//GPIO configuration setup
unsigned char pinFunc[4] = {MCP2221_GPFUNC_IO, MCP2221_GPFUNC_IO, MCP2221_GPFUNC_IO, MCP2221_GPFUNC_IO};
unsigned char pinDir[4] = {MCP2221_GPDIR_OUTPUT, MCP2221_GPDIR_OUTPUT, MCP2221_GPDIR_OUTPUT, MCP2221_GPDIR_OUTPUT};
unsigned char OutValues[4] = {MCP2221_GPVAL_LOW, MCP2221_GPVAL_LOW, MCP2221_GPVAL_LOW, MCP2221_GPVAL_LOW};

//Text box - for info display
QString string;
QTextEdit *TextBox;
unsigned char Addr = 0x00;
unsigned char DummyByte = 0x00;
QString str;

void Mcp2221_config()
{
    ver = Mcp2221_GetLibraryVersion(LibVer);		//Get DLL version
    if(ver == 0)
    {
        //printf("Library (DLL) version: %ls\n", LibVer);
        string = "Library (DLL) version: "+QString::fromWCharArray(LibVer);
        TextBox->append(string);
    }
    else
    {
        error = Mcp2221_GetLastError();
        //printf("Version can't be found, version: %d, error: %d\n", ver, error);
        string = "Cannot get version, error: " + error;
        TextBox->append(string);
    }

    //Get number of connected devices with this VID & PID
    Mcp2221_GetConnectedDevices(VID, PID, &NumOfDev);
    if(NumOfDev == 0)
    {
        //printf("No MCP2221 devices connected\n");
        string = "No MCP2221 devices connected";
        TextBox->append(string);
        //exit(0);
    }
    else
    {
        //printf("Number of devices found: %d\n", NumOfDev);
        string = "Number of devices found: " + QString::number(NumOfDev);
        TextBox->append(string);
    }

    //Open device by index
    handle = Mcp2221_OpenByIndex(VID, PID, NumOfDev-1);
    if(error == NULL)
    {
        //printf("Connection successful\n");
        string = "Connection successful";
        TextBox->append(string);
    }
    else
    {
        error = Mcp2221_GetLastError();
        //printf("Error message is %s\n", error);
        string = "Error message is "+ QString::number(error) + " " + QString(Mcp2221_GetErrorName(error));
        TextBox->append(string);
        _sleep(10000);
    }

    //Get manufacturer descriptor
    flag = Mcp2221_GetManufacturerDescriptor(handle, MfrDescriptor);
    if(flag == 0)
    {
        //printf("Manufacturer descriptor: %ls\n", MfrDescriptor);
        string = "Manufacturer descriptor: " + QString::fromWCharArray(MfrDescriptor);
        TextBox->append(string);
    }
    else
    {
        //printf("Error getting descriptor: %d\n", flag);
        string = "Error getting descriptor: " + QString::number(flag);
        TextBox->append(string);
    }

    //Get product descriptor
    flag = Mcp2221_GetProductDescriptor(handle, ProdDescrip);
    if(flag == 0)
    {
        //printf("Product descriptor: %ls\n", ProdDescrip);
        string = "Product descriptor: " + QString::fromWCharArray(ProdDescrip);
        TextBox->append(string);
    }
    else
    {
        //printf("Error getting product descriptor: %d\n", flag);
        string = "Error getting product descriptor:" + QString::number(flag);
        TextBox->append(string);
    }

    //Get power attributes
    flag = Mcp2221_GetUsbPowerAttributes(handle, &PowerAttrib, &ReqCurrent);
    if(flag == 0)
    {
        //printf("Power Attributes, %x\nRequested current units = %d\nRequested current(mA) = %d\n", PowerAttrib, ReqCurrent, ReqCurrent*2);
        string = "Power Attributes " + QString::number(PowerAttrib) + "\nRequested current units = " + QString::number(ReqCurrent) + "\nRequested current(mA) = " + QString::number(ReqCurrent*2);
        TextBox->append(string);
    }
    else
    {
        //printf("Error getting power attributes: %d\n", flag);
        string = "Error getting power attributes:"+ QString::number(flag);
        TextBox->append(string);
    }

    //Set I2C bus
    flag = Mcp2221_SetSpeed(handle, 500000);    //set I2C speed to 400 KHz
    if(flag == 0)
    {
        //printf("I2C is configured\n");
        string = "I2C is configured";
        TextBox->append(string);
    }
    else
    {
        //printf("Error setting I2C bus: %d\n", flag);
        string = "Error setting I2C bus:"+ QString::number(flag);
        TextBox->append(string);
    }

    //Set I2C advanced parameters
    flag = Mcp2221_SetAdvancedCommParams(handle, 10, 100);  //10ms timeout, try 1000 times
    if(flag == 0)
    {
        //printf("I2C advanced settings set\n");
        string = "I2C advanced settings set";
        TextBox->append(string);
    }
    else
    {
        //printf("Error setting I2C advanced settings: %d\n", flag);
        string = "Error setting I2C advanced settings:"+ QString::number(flag);
        TextBox->append(string);
    }

    //Set GPIO
    flag = Mcp2221_SetGpioSettings(handle, RUNTIME_SETTINGS, pinFunc, pinDir, OutValues);
    if(flag != 0)
    {
        //printf("Error setting GPIO, error: %d\n", flag);
        string = "Error setting GPIO, error: "+ QString::number(flag);
        TextBox->append(string);
    }
}

void MainWindow::Mcp2221_ScanI2cBus()
{
    //Scan all addresses from 1 to 127
    for (Addr = 1; Addr <= 127; Addr++)
    {
        //issue start condition then address
        flag = Mcp2221_I2cWrite(handle, sizeof(DummyByte), Addr, I2cAddr7bit, &DummyByte);
        if (flag == 0)
        {
            string = "Device found: 0x" + QString("%1").arg((uint8_t)Addr, 0, 16); //QString::number(Addr).toLatin1().toHex();
            TextBox->append(string);
        }
        else
        {
            string = "Error: " + QString(Mcp2210_GetErrorName(flag));
            //Mcp2221_I2cCancelCurrentTransfer(handle);
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;    
    w.setFixedSize(500, 500);
    w.setWindowTitle("MCP2221 I2C Scanner");   //Window title

    TextBox = new QTextEdit(&w);
    TextBox->setGeometry(15, 15, 400, 400);
    TextBox->acceptRichText();
    TextBox->setAcceptRichText(true);

    QPushButton *ScanI2cBus = new QPushButton(&w);
    ScanI2cBus->setText("Scan");
    ScanI2cBus->setGeometry(15, 435, 100, 35);


    Mcp2221_config();
    w.show();
    QObject::connect(ScanI2cBus, SIGNAL(clicked()), &w, SLOT(Mcp2221_ScanI2cBus()));

    return a.exec();
}
