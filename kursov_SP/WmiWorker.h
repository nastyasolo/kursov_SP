
#ifndef WMIWORKER_H
#define WMIWORKER_H

#include <QObject>
#include "WmiHelper.h"

class WmiWorker : public QObject {
    Q_OBJECT

public:
    explicit WmiWorker(WmiHelper* helper, QObject* parent = nullptr);

public slots:
    void fetchSystemInfo();

signals:
    void systemInfoUpdated(int cpuLoad, double memoryUsage, double totalMemory, double usedMemory);

private:
    WmiHelper* wmiHelper;
};

#endif // WMIWORKER_H
