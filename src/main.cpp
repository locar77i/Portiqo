// Stl headers
#include <iostream>
#include <map>
#include <memory>



// Qt headers
#include <QtWidgets/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtGui/QIcon>
#include <QtGui/QPalette>

// cURL headers
#include <curl/curl.h>

// headers
#include "Types.h"
#include "Manager.h"

// hmi headers
#include "hmi/MainWindow.h"
#include "hmi/MyQtUtils.h"

// exchange headers
#include "exchange/ProxiesManager.h"

// register the types
/*
Q_DECLARE_METATYPE(exchange::DummyOrder)
Q_DECLARE_METATYPE(exchange::DummyTransfer)
Q_DECLARE_METATYPE(exchange::PortfolioData)
Q_DECLARE_METATYPE(exchange::PortfolioUpdate)
*/



int main(int argc, char *argv[])
{
    std::cout << "Starting the application" << std::endl;
    // Creathe the application and the main window
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/Compressed-File-32x32.png"));
    // Load the translation
    QTranslator translator;
    hmi::loadTranslation(app, translator, "es"); // Load Spanish translation

    // Creathe the main window
    hmi::MainWindow window;

    // Create the File Manager
    exchange::Manager manager;

    std::cout << "Connecting signals and slots" << std::endl;
    // MainWindow signals to Manager slots
    QObject::connect(&window, &hmi::MainWindow::createPortfolio, &manager, &exchange::Manager::createPortfolio, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::openPortfolio, &manager, &exchange::Manager::openPortfolio, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::savePortfolio, &manager, &exchange::Manager::savePortfolio, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::savePortfolioAs, &manager, &exchange::Manager::savePortfolioAs, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::closePortfolio, &manager, &exchange::Manager::closePortfolio, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::lockPortfolio, &manager, &exchange::Manager::lockPortfolio, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::orderConfirmed, &manager, &exchange::Manager::order, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::orderSimulated, &manager, &exchange::Manager::simulateOrder, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::transferConfirmed, &manager, &exchange::Manager::transfer, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::transferSimulated, &manager, &exchange::Manager::simulateTransfer, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::removeLastPortfolioOperation, &manager, &exchange::Manager::removeLastPortfolioOperation, Qt::QueuedConnection);
    // Manager signals to MainWindow slots
    QObject::connect(&manager, &exchange::Manager::portfolioCreated, &window, &hmi::MainWindow::portfolioCreated, Qt::QueuedConnection);
    QObject::connect(&manager, &exchange::Manager::portfolioOperationAdded, &window, &hmi::MainWindow::portfolioOperationAdded, Qt::QueuedConnection);
    QObject::connect(&manager, &exchange::Manager::portfolioOperationRemoved, &window, &hmi::MainWindow::portfolioOperationRemoved, Qt::QueuedConnection);
    QObject::connect(&manager, &exchange::Manager::portfolioSaved, &window, &hmi::MainWindow::portfolioSaved, Qt::QueuedConnection);
    QObject::connect(&manager, &exchange::Manager::portfolioSavedAs, &window, &hmi::MainWindow::portfolioSavedAs, Qt::QueuedConnection);
    QObject::connect(&manager, &exchange::Manager::warning, &window, &hmi::MainWindow::showWarning, Qt::QueuedConnection);
    QObject::connect(&manager, &exchange::Manager::info, &window, &hmi::MainWindow::showInfo, Qt::QueuedConnection);
    QObject::connect(&manager, &exchange::Manager::error, &window, &hmi::MainWindow::showError, Qt::QueuedConnection);
    QObject::connect(&manager, &exchange::Manager::trace, &window, &hmi::MainWindow::showTrace, Qt::QueuedConnection);

    manager.init();
    QThread managerThread;
    manager.moveToThread(&managerThread);
    std::cout << "Launching the manager" << std::endl;
    managerThread.start();

    // Initialize the Curl library
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Create the System Status proxy and launch it in a separate thread
    exchange::SystemStatusWorker krakenSystemStatus(exchange::Service::Kraken, 60000); // Period is 60s
    QObject::connect(&window, &hmi::MainWindow::ping, &krakenSystemStatus, &exchange::SystemStatusWorker::pong, Qt::QueuedConnection);
    QObject::connect(&krakenSystemStatus, &exchange::SystemStatusWorker::systemStatusUpdated, &window, &hmi::MainWindow::updateSystemStatus, Qt::QueuedConnection);
    QObject::connect(&krakenSystemStatus, &exchange::SystemStatusWorker::statisticsUpdated, &window, &hmi::MainWindow::updateSystemStatusStats, Qt::QueuedConnection);
    QObject::connect(&krakenSystemStatus, &exchange::SystemStatusWorker::warning, &window, &hmi::MainWindow::showWarning, Qt::QueuedConnection);
    QObject::connect(&krakenSystemStatus, &exchange::SystemStatusWorker::info, &window, &hmi::MainWindow::showInfo, Qt::QueuedConnection);
    QObject::connect(&krakenSystemStatus, &exchange::SystemStatusWorker::error, &window, &hmi::MainWindow::showError, Qt::QueuedConnection);
    QObject::connect(&krakenSystemStatus, &exchange::SystemStatusWorker::trace, &window, &hmi::MainWindow::showTrace, Qt::QueuedConnection);
    krakenSystemStatus.init();
    exchange::TimerThread krakenSystemStatusThread(krakenSystemStatus.getPeriod());
    QObject::connect(krakenSystemStatusThread.timer(), &QTimer::timeout, &krakenSystemStatus, &exchange::SystemStatusWorker::exec);
    krakenSystemStatus.moveToThread(&krakenSystemStatusThread);


    // Kraken: create the Tickers Information proxy and launch it in a separate thread
    exchange::TickersInformationWorker krakenTickersInformation(exchange::Service::Kraken, 60000); // Period in ms
    QObject::connect(&window, &hmi::MainWindow::ping, &krakenTickersInformation, &exchange::TickersInformationWorker::pong, Qt::QueuedConnection);
    QObject::connect(&krakenTickersInformation, &exchange::TickersInformationWorker::tickersInformationUpdated, &manager, &exchange::Manager::updateTickersInformation, Qt::QueuedConnection);
    QObject::connect(&krakenTickersInformation, &exchange::TickersInformationWorker::tickersInformationUpdated, &window, &hmi::MainWindow::updateTickersInformation, Qt::QueuedConnection);
    QObject::connect(&krakenTickersInformation, &exchange::TickersInformationWorker::statisticsUpdated, &window, &hmi::MainWindow::updateTickersInformationStats, Qt::QueuedConnection);
    QObject::connect(&krakenTickersInformation, &exchange::TickersInformationWorker::warning, &window, &hmi::MainWindow::showWarning, Qt::QueuedConnection);
    QObject::connect(&krakenTickersInformation, &exchange::TickersInformationWorker::info, &window, &hmi::MainWindow::showInfo, Qt::QueuedConnection);
    QObject::connect(&krakenTickersInformation, &exchange::TickersInformationWorker::error, &window, &hmi::MainWindow::showError, Qt::QueuedConnection);
    QObject::connect(&krakenTickersInformation, &exchange::TickersInformationWorker::trace, &window, &hmi::MainWindow::showTrace, Qt::QueuedConnection);
    krakenTickersInformation.init();
    exchange::TimerThread krakenTickersInformationThread(krakenTickersInformation.getPeriod());
    QObject::connect(krakenTickersInformationThread.timer(), &QTimer::timeout, &krakenTickersInformation, &exchange::TickersInformationWorker::exec);
    krakenTickersInformation.moveToThread(&krakenTickersInformationThread);

    // Binance: create the Tickers Information proxy and launch it in a separate thread
    exchange::TickersInformationWorker binanceTickersInformation(exchange::Service::Binance, 60000); // Period in ms
    QObject::connect(&window, &hmi::MainWindow::ping, &binanceTickersInformation, &exchange::TickersInformationWorker::pong, Qt::QueuedConnection);
    QObject::connect(&binanceTickersInformation, &exchange::TickersInformationWorker::tickersInformationUpdated, &manager, &exchange::Manager::updateTickersInformation, Qt::QueuedConnection);
    QObject::connect(&binanceTickersInformation, &exchange::TickersInformationWorker::tickersInformationUpdated, &window, &hmi::MainWindow::updateTickersInformation, Qt::QueuedConnection);
    QObject::connect(&binanceTickersInformation, &exchange::TickersInformationWorker::statisticsUpdated, &window, &hmi::MainWindow::updateTickersInformationStats, Qt::QueuedConnection);
    QObject::connect(&binanceTickersInformation, &exchange::TickersInformationWorker::warning, &window, &hmi::MainWindow::showWarning, Qt::QueuedConnection);
    QObject::connect(&binanceTickersInformation, &exchange::TickersInformationWorker::info, &window, &hmi::MainWindow::showInfo, Qt::QueuedConnection);
    QObject::connect(&binanceTickersInformation, &exchange::TickersInformationWorker::error, &window, &hmi::MainWindow::showError, Qt::QueuedConnection);
    QObject::connect(&binanceTickersInformation, &exchange::TickersInformationWorker::trace, &window, &hmi::MainWindow::showTrace, Qt::QueuedConnection);
    binanceTickersInformation.init();
    exchange::TimerThread binanceTickersInformationThread(binanceTickersInformation.getPeriod());
    QObject::connect(binanceTickersInformationThread.timer(), &QTimer::timeout, &binanceTickersInformation, &exchange::TickersInformationWorker::exec);
    binanceTickersInformation.moveToThread(&binanceTickersInformationThread);

    // Coinbase: create the Tickers Information proxy and launch it in a separate thread
    exchange::TickersInformationWorker coinbaseTickersInformation(exchange::Service::Coinbase, 60000); // Period in ms
    QObject::connect(&window, &hmi::MainWindow::ping, &coinbaseTickersInformation, &exchange::TickersInformationWorker::pong, Qt::QueuedConnection);
    QObject::connect(&coinbaseTickersInformation, &exchange::TickersInformationWorker::tickersInformationUpdated, &manager, &exchange::Manager::updateTickersInformation, Qt::QueuedConnection);
    QObject::connect(&coinbaseTickersInformation, &exchange::TickersInformationWorker::tickersInformationUpdated, &window, &hmi::MainWindow::updateTickersInformation, Qt::QueuedConnection);
    QObject::connect(&coinbaseTickersInformation, &exchange::TickersInformationWorker::statisticsUpdated, &window, &hmi::MainWindow::updateTickersInformationStats, Qt::QueuedConnection);
    QObject::connect(&coinbaseTickersInformation, &exchange::TickersInformationWorker::warning, &window, &hmi::MainWindow::showWarning, Qt::QueuedConnection);
    QObject::connect(&coinbaseTickersInformation, &exchange::TickersInformationWorker::info, &window, &hmi::MainWindow::showInfo, Qt::QueuedConnection);
    QObject::connect(&coinbaseTickersInformation, &exchange::TickersInformationWorker::error, &window, &hmi::MainWindow::showError, Qt::QueuedConnection);
    QObject::connect(&coinbaseTickersInformation, &exchange::TickersInformationWorker::trace, &window, &hmi::MainWindow::showTrace, Qt::QueuedConnection);
    coinbaseTickersInformation.init();
    exchange::TimerThread coinbaseTickersInformationThread(coinbaseTickersInformation.getPeriod());
    QObject::connect(coinbaseTickersInformationThread.timer(), &QTimer::timeout, &coinbaseTickersInformation, &exchange::TickersInformationWorker::exec);
    coinbaseTickersInformation.moveToThread(&coinbaseTickersInformationThread);


    // Create the Order Book manager
    exchange::ProxiesManager proxiesManager(window, manager);
    QObject::connect(&window, &hmi::MainWindow::createProxy, &proxiesManager, &exchange::ProxiesManager::createProxy, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::slowdownProxy, &proxiesManager, &exchange::ProxiesManager::slowdownProxy, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::pauseProxy, &proxiesManager, &exchange::ProxiesManager::pauseProxy, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::resumeProxy, &proxiesManager, &exchange::ProxiesManager::resumeProxy, Qt::QueuedConnection);
    QObject::connect(&window, &hmi::MainWindow::destroyProxy, &proxiesManager, &exchange::ProxiesManager::destroyProxy, Qt::QueuedConnection);
    QObject::connect(&proxiesManager, &exchange::ProxiesManager::proxyCreated, &window, &hmi::MainWindow::activateProxy, Qt::QueuedConnection);
    QObject::connect(&proxiesManager, &exchange::ProxiesManager::proxyDestroyed, &window, &hmi::MainWindow::deactivateProxy, Qt::QueuedConnection);
    QObject::connect(&proxiesManager, &exchange::ProxiesManager::proxyDestroyed, &manager, &exchange::Manager::saveData, Qt::QueuedConnection);
    QObject::connect(&proxiesManager, &exchange::ProxiesManager::info, &window, &hmi::MainWindow::showInfo, Qt::QueuedConnection);
    QObject::connect(&proxiesManager, &exchange::ProxiesManager::warning, &window, &hmi::MainWindow::showWarning, Qt::QueuedConnection);
    QObject::connect(&proxiesManager, &exchange::ProxiesManager::error, &window, &hmi::MainWindow::showError, Qt::QueuedConnection);
    QObject::connect(&proxiesManager, &exchange::ProxiesManager::trace, &window, &hmi::MainWindow::showTrace, Qt::QueuedConnection);
    QThread proxiesManagerThread;
    proxiesManager.moveToThread(&proxiesManagerThread);
    proxiesManagerThread.start();

    int ec;
    try {
        std::cout << "Showing the main window and running the application" << std::endl;
        window.showMaximized();
        window.init();
        std::cout << "Launching the System Status proxy..." << std::endl;
        krakenSystemStatusThread.start();
        std::cout << "Launching the Tickers Information proxies..." << std::endl;
        krakenTickersInformationThread.start();
        binanceTickersInformationThread.start();
        coinbaseTickersInformationThread.start();
        ec = app.exec();
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        ec = -1;
    }

    std::cout << "Finalizing the application..." << std::endl;
    // Quit the manager thread's event loop and wait for it to finish
    std::cout << "Waiting for the manager thread to finish..." << std::endl;
    managerThread.quit();
    managerThread.wait();

    // Quit the Kraken System Status proxy thread's event loop and wait for it to finish
    std::cout << "Waiting for the System Status proxy to finish..." << std::endl;
    krakenSystemStatusThread.requestExit(true);
    krakenSystemStatusThread.wait();

    // Quit the Kraken Tickers Information proxy thread's event loop and wait for it to finish
    std::cout << "Waiting for the Tickers Information proxies to finish..." << std::endl;
    krakenTickersInformationThread.requestExit(true);
    krakenTickersInformationThread.wait();
    binanceTickersInformationThread.requestExit(true);
    binanceTickersInformationThread.wait();
    coinbaseTickersInformationThread.requestExit(true);
    coinbaseTickersInformationThread.wait();
 
    // Requests exit to all proxies and wait for them to finish, quit the proxies manager thread's event loop and wait for it to finish
    std::cout << "Waiting for the proxies manager thread to finish..." << std::endl;
    proxiesManager.requestExit();
    proxiesManagerThread.quit();
    proxiesManagerThread.wait();

    // Finalize the Curl library
    curl_global_cleanup();
   
    std::cout << "Done." << std::endl;
    return ec;
}
