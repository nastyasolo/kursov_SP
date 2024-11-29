#include "WmiHelper.h"
#include <iostream>
#include <comdef.h>
#include <pdh.h>
#include <pdhmsg.h>

#pragma comment(lib, "wbemuuid.lib")

WmiHelper::WmiHelper() {
    HRESULT hres;

    // Инициализация COM
    hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hres)) {
        throw std::runtime_error("COM initialization failed");
    }

    // Вызов CoInitializeSecurity
    hres = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_PKT, // Более высокий уровень безопасности
        RPC_C_IMP_LEVEL_IMPERSONATE, // Или RPC_C_IMP_LEVEL_IMPERSONATE
        NULL, EOAC_NONE, NULL
    );

    if (FAILED(hres)) {
        CoUninitialize();
        throw std::runtime_error("Security initialization failed");
    }



    if (FAILED(hres)) {
        CoUninitialize(); // Освобождаем COM
        throw std::runtime_error("Security initialization failed");
    }

    // Создаём IWbemLocator
    hres = CoCreateInstance(
        CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator
    );
    if (FAILED(hres)) {
        CoUninitialize();
        throw std::runtime_error("Failed to create IWbemLocator object");
    }

    // Подключение к WMI
    hres = pLocator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pServices
    );
    if (FAILED(hres)) {
        pLocator->Release();
        CoUninitialize();
        throw std::runtime_error("Failed to connect to WMI server");
    }

    // Установка параметров безопасности
    hres = CoSetProxyBlanket(
        pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE
    );
    if (FAILED(hres)) {
        pServices->Release();
        pLocator->Release();
        CoUninitialize();
        throw std::runtime_error("Failed to set proxy blanket");
    }

    std::wcout << L"WmiHelper successfully initialized." << std::endl;
}



WmiHelper::~WmiHelper() {
    if (pServices) pServices->Release();
    if (pLocator) pLocator->Release();
    CoUninitialize();
}

std::wstring WmiHelper::GetCpuName() {
    std::wstring cpuName;
    ExecuteQuery(L"SELECT Name FROM Win32_Processor", [&cpuName](IWbemClassObject* pClassObject) {
        VARIANT vtProp;
    if (SUCCEEDED(pClassObject->Get(L"Name", 0, &vtProp, 0, 0))) {
        cpuName = vtProp.bstrVal;
        VariantClear(&vtProp);
    }
        });
    return cpuName;
}

int WmiHelper::GetCpuLoadPercentage() {
    int loadPercentage = -1; // Используем -1, чтобы отличать "ошибку" от реального значения 0
    ExecuteQuery(L"SELECT LoadPercentage FROM Win32_Processor",
        [&loadPercentage](IWbemClassObject* pClassObject) {
            VARIANT vtProp;
    if (SUCCEEDED(pClassObject->Get(L"LoadPercentage", 0, &vtProp, 0, 0))) {
        if (vtProp.vt == VT_UI4 || vtProp.vt == VT_I4) { // Проверяем тип значения
            loadPercentage = static_cast<int>(vtProp.uintVal);
        }
        else {
            std::wcerr << L"Unexpected variant type: " << vtProp.vt << std::endl;
        }
        VariantClear(&vtProp);
    }
    else {
        std::wcerr << L"Failed to get LoadPercentage property." << std::endl;
    }
        });
    std::wcout << L"CPU Load Percentage: " << loadPercentage << std::endl;
    return loadPercentage;
}
//double WmiHelper::GetCpuLoadPercentage() {
//    double totalLoad = -1.0;
//    int coreCount = 0;
//
//    ExecuteQuery(L"SELECT LoadPercentage FROM Win32_Processor",
//        [&totalLoad, &coreCount](IWbemClassObject* pClassObject) {
//            VARIANT vtProp;
//    if (SUCCEEDED(pClassObject->Get(L"LoadPercentage", 0, &vtProp, 0, 0))) {
//        if (vtProp.vt == VT_UI4 || vtProp.vt == VT_I4) {
//            totalLoad += static_cast<double>(vtProp.uintVal);
//            coreCount++;
//        }
//        VariantClear(&vtProp);
//    }
//        });
//
//    return (coreCount > 0) ? totalLoad / coreCount : 0.0;
//}



void WmiHelper::GetMemoryUsage(double& memoryUsage, double& totalMemory, double& usedMemory) {
    ExecuteQuery(L"SELECT TotalVisibleMemorySize, FreePhysicalMemory FROM Win32_OperatingSystem",
        [&memoryUsage, &totalMemory, &usedMemory](IWbemClassObject* pClassObject) {
            VARIANT totalMemProp, freeMemProp;
    if (SUCCEEDED(pClassObject->Get(L"TotalVisibleMemorySize", 0, &totalMemProp, 0, 0)) &&
        SUCCEEDED(pClassObject->Get(L"FreePhysicalMemory", 0, &freeMemProp, 0, 0))) {
        if (totalMemProp.vt == VT_BSTR && freeMemProp.vt == VT_BSTR) {
            unsigned long long totalMemoryRaw = std::stoull(totalMemProp.bstrVal);
            unsigned long long freeMemoryRaw = std::stoull(freeMemProp.bstrVal);

            usedMemory = totalMemoryRaw - freeMemoryRaw;
            totalMemory = static_cast<double>(totalMemoryRaw) / 1024 / 1024;
            memoryUsage = (static_cast<double>(usedMemory) / totalMemoryRaw) * 100;
        }
        VariantClear(&totalMemProp);
        VariantClear(&freeMemProp);
    }
        });
}


void WmiHelper::ExecuteQuery(const std::wstring& query, std::function<void(IWbemClassObject*)> processor) {
    if (!pServices) {
        std::wcerr << L"pServices is not initialized. Ensure WmiHelper constructor completed successfully." << std::endl;
        return;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    HRESULT hres = pServices->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator
    );

    if (FAILED(hres) || !pEnumerator) {
        std::wcerr << L"Query failed: " << query << L". Error: " << std::hex << hres << std::endl;
        return;
    }

    IWbemClassObject* pClassObject = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
        if (FAILED(hr) || uReturn == 0) break;

        if (pClassObject) {
            processor(pClassObject);
            pClassObject->Release();
        }
    }

    if (pEnumerator) pEnumerator->Release();
}

