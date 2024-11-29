#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>


#include "WmiWorker.h"
#include "wmihelper.h"

#include <QThread>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>


namespace QtCharts {
    class QChart;
    class QLineSeries;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void requestSystemInfo();  // Добавляем сигнал

private slots:
    void updateSystemInfo();


    //лишнее
    void updateLabels(int cpuLoad, double memoryUsage, double totalMemory, double usedMemory);


private:
    Ui::MainWindowClass ui;

    QtCharts::QChart* cpuChart;
    QtCharts::QChart* memoryChart;
    QtCharts::QLineSeries* cpuSeries;
    QtCharts::QLineSeries* memorySeries;

    WmiHelper *wmiHelper = new WmiHelper() ;  // объект для работы с WMI

//вр1
    WmiWorker* worker = new WmiWorker(wmiHelper);
    QThread* workerThread = new QThread(this);

  //  WmiWorker* worker = new WmiWorker(wmiHelper);
};
