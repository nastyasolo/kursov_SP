#ifndef WMIF_H
#define WMIF_H

#include <string>
#include <functional>
#include <Wbemidl.h>
#include <Windows.h>

class WmiHelper {
public:
    WmiHelper();
    ~WmiHelper();

    // Получение имени процессора
    std::wstring GetCpuName();

    // Получение процента загрузки процессора
    int GetCpuLoadPercentage(); 
    //double GetCpuLoadPercentage();


    // Получение информации о памяти
    void GetMemoryUsage(double& memoryUsage, double& totalMemory, double& usedMemory);

private:
    IWbemLocator* pLocator = nullptr;
    IWbemServices* pServices = nullptr;

    void ExecuteQuery(const std::wstring& query, std::function<void(IWbemClassObject*)> processor);
};

#endif // WMIF_H
