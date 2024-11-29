#include "WmiWorker.h"

WmiWorker::WmiWorker(WmiHelper* helper, QObject* parent)
    : QObject(parent), wmiHelper(helper) {}

void WmiWorker::fetchSystemInfo() {
    double memoryUsage = 0.0, totalMemory = 0.0, usedMemory = 0.0;
    int cpuLoad = wmiHelper->GetCpuLoadPercentage();
    wmiHelper->GetMemoryUsage(memoryUsage, totalMemory, usedMemory);

    emit systemInfoUpdated(cpuLoad, memoryUsage, totalMemory, usedMemory);
}
