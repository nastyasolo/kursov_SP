#include "MainWindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    this->resize(800, 600);
    // Создание графиков
    cpuSeries = new QLineSeries();
    memorySeries = new QLineSeries();

    QChart* cpuChart = new QChart();
    cpuChart->addSeries(cpuSeries);
    cpuChart->setTitle("CPU Usage");
    cpuChart->createDefaultAxes();

    QChart* memoryChart = new QChart();
    memoryChart->addSeries(memorySeries);
    memoryChart->setTitle("Memory Usage");
    memoryChart->createDefaultAxes();

    QChartView* cpuChartView = new QChartView(cpuChart);
    QChartView* memoryChartView = new QChartView(memoryChart);

    ui.verticalLayout->addWidget(cpuChartView);
    ui.verticalLayout->addWidget(memoryChartView);
    
    // Создаем таймер для обновления данных
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSystemInfo);
    timer->start(1000);  // обновление каждую секунду

    // Связываем кнопку обновления с функцией обновления информации
    connect(ui.refreshButton, &QPushButton::clicked, this, &MainWindow::updateSystemInfo);
    QString processorName = QString::fromStdWString(wmiHelper->GetCpuName());
    ui.nameProcessorLabel->setText("Processor name: " + processorName);

    // Инициализируем начальные значения меток
    ui.cpuLoadLabel->setText("CPU Load: Calculating...");
    ui.memoryLabel->setText("Free Memory: Calculating...");

   
    //
    ////ва1
    //worker->moveToThread(workerThread);

    //connect(this, &MainWindow::requestSystemInfo, worker, &WmiWorker::fetchSystemInfo);
    //connect(worker, &WmiWorker::systemInfoUpdated, this, &MainWindow::updateLabels);

    //workerThread->start();

    //QTimer* timer = new QTimer(this);
    //connect(timer, &QTimer::timeout, this, &MainWindow::requestSystemInfo);
    //timer->start(2000);

}

MainWindow::~MainWindow()
{
    workerThread->quit();
    workerThread->wait();
    delete worker;
    delete workerThread;

    delete wmiHelper;
}
void MainWindow::updateSystemInfo()
{
    
   // std::wstring cpuName = wmiHelper->GetCpuName();
   // double load1 = 0.0, load2 = 0.0, load3 = 0.0;
   // int cpuLoad = wmiHelper->GetCpuLoadPercentage();
    double cpuLoad = wmiHelper->GetCpuLoadPercentage();


    //wmiHelper.GetCPULoad(cpuLoad);
    //wmiHelper.GetMemoryUsage(memoryUsage);

    double memoryUsage = 0.0, totalMemory = 0.0, usedMemory = 0.0;
    wmiHelper->GetMemoryUsage(memoryUsage, totalMemory, usedMemory);

  
    //ui.memoryLabel->setText("Memory Usage: " + static_cast<int>(memoryUsage) + " (" + usedMemory);
    ui.memoryLabel->setText("Memory Usage: " + QString::number(static_cast<int>(memoryUsage)) + " (" + QString::number(usedMemory) + "  GB из "+ QString::number(totalMemory) + " )" );

    ui.cpuLoadLabel->setText("CPU Usage: " + QString::number(static_cast<int>(cpuLoad)));

    // Обновление графиков
    cpuSeries->append(QDateTime::currentDateTime().toMSecsSinceEpoch(), cpuLoad);
    memorySeries->append(QDateTime::currentDateTime().toMSecsSinceEpoch(), memoryUsage);

    // Лимитируем количество точек на графике
    if (cpuSeries->count() > 100) cpuSeries->remove(0);
    if (memorySeries->count() > 100) memorySeries->remove(0);
    
}

//ва1????
void MainWindow::updateLabels(int cpuLoad, double memoryUsage, double totalMemory, double usedMemory) {
    ui.cpuLoadLabel->setText("CPU Usage: " + QString::number(cpuLoad) + "%");
    ui.memoryLabel->setText("Memory Usage: " + QString::number(static_cast<int>(memoryUsage)) +
        "% (" + QString::number(usedMemory, 'f', 2) +
        " GB из " + QString::number(totalMemory, 'f', 2) + " GB)");
}

