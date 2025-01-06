#ifndef DEVICE_CPP
#define DEVICE_CPP

#include "device.h"
#include "lib/bb_api.h"
#include "lib/sa_api.h"
#include "lib/rtlsdr.h"
QList<DeviceConnectionInfo> Device::GetDeviceList() const
{
    QList<DeviceConnectionInfo> deviceList;
    DeviceConnectionInfo info;

    int serialNumbers[8];
    int deviceCount;

    info.series = saSeries;
    saGetSerialNumberList(serialNumbers, &deviceCount);
    for(int i = 0; i < deviceCount; i++) {
        info.serialNumber = serialNumbers[i];
        deviceList.push_back(info);
    }
    info.series = bbSeries;
    bbGetSerialNumberList(serialNumbers, &deviceCount);
    for(int i = 0; i < deviceCount; i++) {
        info.serialNumber = serialNumbers[i];
        deviceList.push_back(info);
    }
    return deviceList;
}
QList<DeviceRtlInfo> Device::GetRtlList() const
{
    QList<DeviceRtlInfo> deviceList;
    DeviceRtlInfo info;
    QString serialNumbers;
    int deviceCount;
    info.series = rtlSeries;
    deviceCount = rtlsdr_get_device_count();
    for (int i = 0; i < deviceCount; i++) {
           const char *device_name = rtlsdr_get_device_name(i);
           char manufacturer[256], product[256], serial[256];
           if (rtlsdr_get_device_usb_strings(i, manufacturer, product, serial) == 0) {
               qDebug() << "  Manufacturer: " << manufacturer;
               qDebug() << "  Product: " << product;
               qDebug() << "  Serial: " << serial;
               info.serialNumber =  serial ;
               deviceList.push_back(info);
           }
       }
    return deviceList;
}

#endif // DEVICE_CPP
