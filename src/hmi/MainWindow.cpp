// headers
#include "MainWindow.h"

// Qt headers
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QStyleFactory>
#include <QtGui/QActionGroup>
#include <QtGui/QFont>

// hmi headers
#include "MyQtUtils.h"


namespace hmi
{
    MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , tickersInformation_()
        , orderDialog_(nullptr)
        , transferDialog_(nullptr)
        , portfolioTree_(nullptr)
        , systemLog_(nullptr)
    {
        init_();
    }

    MainWindow::~MainWindow()
    {}

    void MainWindow::loadTranslation(const QString &languageCode) {
        hmi::loadTranslation(*qApp, translator_, languageCode);
        retranslateUi();
        systemLog_->info(tr("language-loaded")  // "Language '%1' loaded"
            .arg(QLocale::languageToString(QLocale(languageCode).language()).toUpper())
         );
    }

    void MainWindow::retranslateUi() {
        setTranslatableText_();
        newDialog_->retranslateUi();
        portfolioTree_->retranslateUi();
        orderDialog_->retranslateUi();
        orderForm_->retranslateUi();
        transferDialog_->retranslateUi();
        transferForm_->retranslateUi();
        tickersInformationDialog_->retranslateUi();
        recentTradesDialog_->retranslateUi();
        orderBooksDialog_->retranslateUi();
        systemLog_->retranslateUi();
        for(auto subWindow : mdiArea->subWindowList()) {
            PortfolioWindow* window = qobject_cast<PortfolioWindow*>(subWindow->widget());
            if(window) {
                window->retranslateUi();
            }
        }

    }

    void MainWindow::portfolioCreated(const exchange::PortfolioData &data, bool openFromFile) {
        exchange::Market market = marketComboBox_->currentData().value<exchange::Market>();
        // Create the portfolio window
        PortfolioWindow *portfolio = new PortfolioWindow(data, market, this);
        portfolio->setTickersInformation(tickersInformation_);
        // Create a new row in the portfolio tree
        portfolioTree_->add(portfolio);
        setPortfolioModified_(portfolio, data.filename.empty());
        connect(portfolio, &PortfolioWindow::savePortfolio, this, &MainWindow::savePortfolio_);
        connect(portfolio, &PortfolioWindow::savePortfolioAs, this, &MainWindow::savePortfolioAs_);
        connect(portfolio, &PortfolioWindow::lockPortfolio, this, &MainWindow::lockPortfolio);
        connect(portfolio, &PortfolioWindow::orderAsset, this, &MainWindow::orderAsset_);
        connect(portfolio, &PortfolioWindow::transferAsset, this, &MainWindow::transferAsset_);
        connect(portfolio, &PortfolioWindow::removeLastPortfolioOperation, this, &MainWindow::removeLastPortfolioOperation_);
        connect(portfolio, &PortfolioWindow::readOnlyTimeout, this, &MainWindow::readOnlyTimeout_);
        connect(portfolio, &PortfolioWindow::buyAssetUsing, [this](const PortfolioWindow* portfolio, exchange::Ticket asset, double percentage) {
            if(portfolio) {
                double total = portfolio->getGeneralBalance().getBalance(asset).getQuantity() * percentage / 100;
                if(portfolio->isLocked()) {
                    orderForm_->setForBuy(total);
                } else {
                    orderDialog_->showForBuy(total);
                }
            }
        });
        connect(portfolio, &PortfolioWindow::buyAssetUsingTheProfit, [this](const PortfolioWindow* portfolio, exchange::Ticket asset) {
            if(portfolio) {
                double total = portfolio->getGeneralBalance().getBalance(asset).getQuantity() - portfolio->getStats().getMaterializedTaxes();
                if(portfolio->isLocked()) {
                    orderForm_->setForBuy(total);
                } else {
                    orderDialog_->showForBuy(total);
                }
            }
        });
        connect(portfolio, &PortfolioWindow::sellAssetUsing, [this](const PortfolioWindow* portfolio, exchange::Ticket asset, double percentage) {
            if(portfolio) {
                exchange::Market market = exchange::getMarket(asset, portfolio->getReferenceCurrency());
                marketComboBox_->setCurrentIndex(marketComboBox_->findData(QVariant::fromValue(market)));
                double quantity = portfolio->getGeneralBalance().getBalance(asset).getQuantity() * percentage / 100;
                if(portfolio->isLocked()) {
                    orderForm_->setForSell(quantity);
                } else {
                    orderDialog_->showForSell(quantity);
                }
            }
        });
        connect(portfolio, &PortfolioWindow::sellAssetToLeaveTheProfit, [this](const PortfolioWindow* portfolio, exchange::Ticket asset) {
            if(portfolio) {
                exchange::Market market = exchange::getMarket(asset, portfolio->getReferenceCurrency());
                marketComboBox_->setCurrentIndex(marketComboBox_->findData(QVariant::fromValue(market)));
                double increment = portfolio->getIncrement(exchange::getAssetType(market));
                double multiplier = portfolio->getMultiplier();
                double lastPrice = tickersInformation_.getLastPrice(market, portfolio->getMultiplier(), portfolio->getIncrement(exchange::getAssetType(market)));
                double amount = portfolio->getGeneralBalance().getTotalQuantity(portfolio->getReferenceCurrency());
                if(amount < portfolio->getGeneralBalance().getCapital()) {
                    double total = portfolio->getGeneralBalance().getCapital() - amount;
                    total += portfolio->getStats().estimateTaxesAfterSelling(market, total, lastPrice, multiplier, increment);
                    double quantity = total / 0.9975 / lastPrice;
                    if(portfolio->isLocked()) {
                        orderForm_->setForSell(quantity);
                    } else {
                        orderDialog_->showForSell(quantity);
                    }
                }
            }
        });
        connect(portfolio, &PortfolioWindow::calculateDetailsFor, [this](const std::vector<std::string>& ids) {
            for(const std::string& id : ids) {
                systemLog_->info("Show details for: " + QString::fromStdString(id));
            }
        });
        connect(portfolio, &PortfolioWindow::info, systemLog_, &SystemLog::info);
        connect(portfolio, &PortfolioWindow::warning, systemLog_, &SystemLog::warning);
        connect(portfolio, &PortfolioWindow::error, systemLog_, &SystemLog::error);
        connect(portfolio, &PortfolioWindow::trace, systemLog_, &SystemLog::trace);
        connect(portfolio, &PortfolioWindow::closed, this, &MainWindow::portfolioWindowClosed_);
        QMdiSubWindow *subWindow = mdiArea->addSubWindow(portfolio);
        // Ensure the subwindow is deleted when closed to free up resources
        subWindow->setAttribute(Qt::WA_DeleteOnClose);
        subWindow->showMaximized();
        mdiArea->setActiveSubWindow(subWindow);
        // Show the information in the system log
        systemLog_->trace("Number of QMdiSubWindow: " + QString::number(mdiArea->subWindowList().count()));
        if(openFromFile) {
            systemLog_->info(tr("portfolio-loaded-from-file")  // "Portfolio '%1' loaded from file '%2' (Total: %3 operations)   [V%4]"
                .arg(data.name.c_str())
                .arg(data.filename.c_str())
                .arg(data.operations.size())
                .arg(data.version.c_str())
            );
        }
        else {
            systemLog_->info(tr("new-portfolio-created")  // "Newly created '%1' portfolio   [V%2]"
                .arg(data.name.c_str())
                .arg(data.version.c_str())
            );
            
        }
    }


    void MainWindow::portfolioOperationAdded(const QString& name, const exchange::PortfolioUpdate& data)
    {
        try {
            // Find the 'name' portfolio window to add the operation
            PortfolioWindow* portfolio = getPortfolioWindow_(name);
            portfolio->updateView(data);
            portfolio->setReadOnly(false);
            setPortfolioModified_(portfolio, true);
            updateStatusFromActivePortfolio_();
        }
        catch (const std::exception& e) {
            systemLog_->warning(e.what());
        }
    }

    void MainWindow::portfolioOperationRemoved(const QString& name, const exchange::PortfolioRemove& data)
    {
        try {
            // Find the 'name' portfolio window to remove the operation
            PortfolioWindow* portfolio = getPortfolioWindow_(name);
            portfolio->updateView(data);
            portfolio->setReadOnly(false);
            setPortfolioModified_(portfolio, true);
            updateStatusFromActivePortfolio_();
        }
        catch (const std::exception& e) {
            systemLog_->warning(e.what());
        }
    }

    void MainWindow::portfolioSaved(const QString& name, unsigned int numOperations)
    {
        setPortfolioModified_(getPortfolioWindow_(name), false);
        enablePortfolioActions_(getActivePortfolio_());
    }

    void MainWindow::portfolioSavedAs(const QString& name, const QString& filename, unsigned int numOperations)
    {
        PortfolioWindow* portfolio = getPortfolioWindow_(name);
        setPortFolioFilename_(portfolio, filename);
        setPortfolioModified_(portfolio, false);
        enablePortfolioActions_(getActivePortfolio_());
    }

    void MainWindow::updateTickersInformationStats(exchange::Service service, exchange::ProxyType type, exchange::WorkStats workStats) {
        frequencyLineEdit_->setText(QString::number(workStats.frequency, 'f', 2) + " Hz");
        frequencyLineEdit_->setToolTip(tr("proxy-work-stats-tooltip")  // "Max elapsed time: %1 ms  (%2 executions, %3 cycles)"
            .arg(workStats.maxElapsedTime)
            .arg(workStats.count)
            .arg(workStats.cycles)
        );
    }

    void MainWindow::updateSystemStatusStats(exchange::Service service, exchange::ProxyType type, exchange::WorkStats workStats) {
        static unsigned int count = 0;
        if(++count % 30 == 0) {
            systemLog_->info(tr("proxy-work-stats-description")  // "Freqquency: %1 hz - Max elapsed time: %2 ms  (%3 executions, %4 cycles)"
                .arg(workStats.frequency, 0, 'f', 2)
                .arg(workStats.maxElapsedTime)
                .arg(workStats.count)
                .arg(workStats.cycles)
            );
        }
    }

    void MainWindow::updateProxyStatus(exchange::Service service, exchange::ProxyType type, exchange::Market market, exchange::ProxyStatus status) {
        switch(type) {
            case exchange::ProxyType::TickersInformation:
                tickersInformationDialog_->updateProxyStatus(service, market, status);
                break;
            case exchange::ProxyType::RecentTrades:
                recentTradesDialog_->updateProxyStatus(service, market, status);
                break;
            case exchange::ProxyType::OrderBook:
                orderBooksDialog_->updateProxyStatus(service, market, status);
                break;
            default:
                systemLog_->warning(tr("unknown-proxy-type")  // "Unknown proxy type: %1"
                    .arg(exchange::getProxyTypeName(type).c_str())
                );
                break;
        }
    }

    void MainWindow::updateProxyStatistics(exchange::Service service, exchange::ProxyType type, exchange::Market market, exchange::WorkStats workStats) {
        switch(type) {
            case exchange::ProxyType::TickersInformation:
                tickersInformationDialog_->updateProxyStatistics(service, market, workStats);
                break;
            case exchange::ProxyType::RecentTrades:
                recentTradesDialog_->updateProxyStatistics(service, market, workStats);
                break;
            case exchange::ProxyType::OrderBook:
                orderBooksDialog_->updateProxyStatistics(service, market, workStats);
                break;
            default:
                systemLog_->warning(tr("unknown-proxy-type")  // "Unknown proxy type: %1"
                    .arg(exchange::getProxyTypeName(type).c_str())
                );
                break;
        }
    }

    void MainWindow::updateSystemStatus(exchange::SystemStatus systemStatus)
    {
        try {
            QIcon icon;
            switch(systemStatus.value) {
                case exchange::Status::Online:
                    icon = QIcon(":/icons/Network-Tick-32x32.png");
                    break;
                case exchange::Status::Offline:
                    icon = QIcon(":/icons/Network-Close-32x32.png");
                    break;
                case exchange::Status::Error:
                    icon = QIcon(":/icons/Network-Warning-32x32.png");
                    break;
                //case exchange::Status::Unknown:
                default:
                    icon = QIcon(":/icons/Network-32x32.png");
                    break;
            }
            // Get time in string format
            QString timestamp = formatTimePoint(systemStatus.timestamp);
            QString message = tr("kraken-proxy-status-description").arg(exchange::getStatusName(systemStatus.value).c_str()).arg(timestamp); // Kraken Proxy status: %1 - last update on %2  (Server time)
            pingAction_->setIcon(icon);
            pingAction_->setToolTip(message);
            statusBar()->showMessage(message, 5000);
        }
        catch (const std::exception& e) {
            systemLog_->warning(e.what());
        }
    }

    void MainWindow::updateTickersInformation(exchange::Service service, exchange::TickersInformation tickersInformation) {
        if(service == exchange::Service::Kraken) {
            tickersInformation_ = tickersInformation;
            updateTickerInformation_();
            // set the TickersInformation on all portfolio windows
            for(auto &window : mdiArea->subWindowList()) {
                PortfolioWindow *portfolio = dynamic_cast<PortfolioWindow*>(window->widget());
                if(portfolio) {
                    portfolio->setTickersInformation(tickersInformation_);
                }
            }
        }
    }

    void MainWindow::updateOhlcDataSummary(exchange::Market market, exchange::OhlcDataSummary ohlcDataSummary)
    {
        if(market == exchange::Market::BTC_EUR) {
            //systemLog_->info(QString("OHLC data updated: ") + ohlcDataSummary.description().c_str());
        }
    }

    void MainWindow::activateProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market, unsigned int period) {
        switch(type) {
            case exchange::ProxyType::TickersInformation:
                tickersInformationDialog_->activateProxy(service, market, period);
                systemLog_->info(tr("tickers-information-proxy-activated")
                    .arg(exchange::getServiceName(service).c_str())
                    .arg(exchange::getMarketName(market).c_str())
                );
                break;
            case exchange::ProxyType::RecentTrades:
                recentTradesDialog_->activateProxy(service, market, period);
                systemLog_->info(tr("recent-trades-proxy-activated")
                    .arg(exchange::getServiceName(service).c_str())
                    .arg(exchange::getMarketName(market).c_str())
                );
                break;
            case exchange::ProxyType::OrderBook:
                orderBooksDialog_->activateProxy(service, market, period);
                systemLog_->info(tr("order-book-proxy-activated")
                    .arg(exchange::getServiceName(service).c_str())
                    .arg(exchange::getMarketName(market).c_str())
                );
                break;
            default:
                systemLog_->warning(tr("unknown-proxy-type")  // "Unknown proxy type: %1"
                    .arg(exchange::getProxyTypeName(type).c_str())
                );
                break;
        }
    }

    void MainWindow::deactivateProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market) {
        switch(type) {
            case exchange::ProxyType::TickersInformation:
                tickersInformationDialog_->deactivateProxy(service, market);
                systemLog_->info(tr("tickers-information-proxy-deactivated")
                    .arg(exchange::getServiceName(service).c_str())
                    .arg(exchange::getMarketName(market).c_str())
                );
                break;
            case exchange::ProxyType::RecentTrades:
                recentTradesDialog_->deactivateProxy(service, market);
                systemLog_->info(tr("recent-trades-proxy-deactivated")
                    .arg(exchange::getServiceName(service).c_str())
                    .arg(exchange::getMarketName(market).c_str())
                );
                break;
            case exchange::ProxyType::OrderBook:
                orderBooksDialog_->deactivateProxy(service, market);
                systemLog_->info(tr("order-book-proxy-deactivated")
                    .arg(exchange::getServiceName(service).c_str())
                    .arg(exchange::getMarketName(market).c_str())
                );
                break;
            default:
                systemLog_->warning(tr("unknown-proxy-type")  // "Unknown proxy type: %1"
                    .arg(exchange::getProxyTypeName(type).c_str())
                );
                break;
        }
    }

    void MainWindow::updateTickerInformation_() {
        exchange::Market market = marketComboBox_->currentData().value<exchange::Market>();
        try {
            const exchange::TickerInformation& tickerInfo = tickersInformation_.get(market);
            lastLineEdit_->setText(MyQtUtils::stringfyPriceHR(tickerInfo.last, tickerInfo.asset, tickerInfo.currency));
            tradesLineEdit_->setText(QLocale(QLocale::English).toString(tickerInfo.trades));
            highLineEdit_->setText(MyQtUtils::stringfyPriceHR(tickerInfo.high, tickerInfo.asset, tickerInfo.currency));
            lowLineEdit_->setText(MyQtUtils::stringfyPriceHR(tickerInfo.low, tickerInfo.asset, tickerInfo.currency));
            volumeLineEdit_->setText(MyQtUtils::stringfyLN(tickerInfo.volume, tickerInfo.asset, true));
            totalVolumeLineEdit_->setText(MyQtUtils::stringfyLN(tickerInfo.volume * tickerInfo.last, tickerInfo.currency, true));
        }
        catch (const std::exception& e) {
            systemLog_->warning(tr("exception-updating-ticker-info") // "Error updating the ticker information: %1 (market: %2)"
                .arg(e.what())
                .arg(exchange::getMarketName(market).c_str())
            );
        }
    }

    // PRIVATE
    void MainWindow::init_()
    {
/*
        QStringList availableStyles = QStyleFactory::keys();
        QMessageBox::information(this, tr("available-styles"), availableStyles.join(", "));
*/
        setWindowIcon(QIcon(":/icons/Compressed-File-32x32.png"));

        mdiArea = new QMdiArea;
        setCentralWidget(mdiArea);

        createActions_();
        createMenubar_();
        createToolbars_();

        // Create the new portfolio dialog
        newDialog_ = new NewDialog(this);
        newDialog_->setModal(true);
        newDialog_->setFixedSize(240, 150);

        // Create a dock widget for the portfolio tree
        portfolioTree_ = new PortfolioTree(this);
        portfolioTreeDockWidget_ = new QDockWidget(this);
        portfolioTreeDockWidget_->setWidget(portfolioTree_);
        // Allow the dock widget to be docked at the left or right of the main window
        portfolioTreeDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        addDockWidget(Qt::LeftDockWidgetArea, portfolioTreeDockWidget_);

        // Create a dock widget for the system log
        systemLog_ = new SystemLog(this);
        systemLogDockWidget_ = new QDockWidget(this);
        systemLogDockWidget_->setWidget(systemLog_);
        systemLogDockWidget_->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
        systemLogDockWidget_->setMinimumHeight(100);
        addDockWidget(Qt::BottomDockWidgetArea, systemLogDockWidget_);

        // Create the order form and its dock widget
        orderForm_ = new OrderForm(this);
        orderForm_->updateView(nullptr); // Disable the order form
        orderForm_->setMarket(exchange::Market::BTC_EUR);
        orderDockWidget_ = new QDockWidget(this);
        orderDockWidget_->setWidget(orderForm_);
        orderDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        orderDockWidget_->setFixedSize(330, 220);
        addDockWidget(Qt::LeftDockWidgetArea, orderDockWidget_);

        // Create the transfer form and its dock widget
        transferForm_ = new TransferForm(this);
        transferForm_->updateView(nullptr); // Disable the transfer form
        transferForm_->setAsset(exchange::Ticket::EUR);
        transferDockWidget_ = new QDockWidget(this);
        transferDockWidget_->setWidget(transferForm_);
        transferDockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        transferDockWidget_->setFixedSize(330, 220);
        addDockWidget(Qt::LeftDockWidgetArea, transferDockWidget_);

        // Create the order dialog
        orderDialog_ = new OrderDialog(this);
        orderDialog_->updateView(nullptr);
        orderDialog_->setMarket(exchange::Market::BTC_EUR);
        orderDialog_->hide();
        // Create the transfer dialog
        transferDialog_ = new TransferDialog(this);
        transferDialog_->updateView(nullptr);
        transferDialog_->setAsset(exchange::Ticket::EUR);
        transferDialog_->hide();

        // Stack the orderDockWidget_ and the transferDockWidget_ dock widgets
        tabifyDockWidget(orderDockWidget_, transferDockWidget_);
        orderDockWidget_->raise(); // Set the orderDockWidget_ the active tab widget

        // Create the frequency label and line edit
        frequencyLabel_ = new QLabel(this);
        frequencyLineEdit_ = new QLineEdit();
        frequencyLineEdit_->setReadOnly(true);
        frequencyLineEdit_->setFixedWidth(80);
        frequencyLineEdit_->setAlignment(Qt::AlignRight);
        // Create the layout for the status bar
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addStretch();
        layout->addWidget(frequencyLabel_);
        layout->addWidget(frequencyLineEdit_);
        // Create a widget to hold the layout
        QWidget* statusBarWidget = new QWidget(this);
        statusBarWidget->setLayout(layout);
        // Create the status bar
        statusBar_ = new QStatusBar(this);
        statusBar_->addPermanentWidget(statusBarWidget);
        setStatusBar(statusBar_);
        

        // Connect the signals of the actions
        connect(newPortfolioAction, &QAction::triggered, this, &MainWindow::newPortfolioTriggered_);
        connect(openPortfolioAction, &QAction::triggered, this, &MainWindow::openPortfolioTriggered_);
        connect(savePortfolioAction, &QAction::triggered, this, &MainWindow::savePortfolioTriggered_);
        connect(savePortfolioAsAction, &QAction::triggered, this, &MainWindow::savePortfolioAsTriggered_);
        connect(saveAllPortfoliosAction, &QAction::triggered, this, &MainWindow::saveAllPortfoliosTriggered_);
        connect(showPortfolioTreeAction, &QAction::triggered, this, &MainWindow::showPortfolioTreeTriggered_);
        connect(showSystemLogAction, &QAction::triggered, this, &MainWindow::showSystemLogTriggered_);
        connect(showOrderFormAction, &QAction::triggered, this, &MainWindow::showOrderFormTriggered_);
        connect(showTransferFormAction, &QAction::triggered, this, &MainWindow::showTransferFormTriggered_);
        connect(enLanguageAction, &QAction::triggered, [this] { loadTranslation("en"); });
        connect(esLanguageAction, &QAction::triggered, [this] { loadTranslation("es"); });
        connect(fusionStyleAction, &QAction::triggered, [this] { setStyle("Fusion"); });
        connect(windowsStyleAction, &QAction::triggered, [this] { setStyle("Windows"); });
        connect(windowsVistaStyleAction, &QAction::triggered, [this] { setStyle("WindowsVista"); });
        connect(windows11StyleAction, &QAction::triggered, [this] { setStyle("Windows11"); });
        connect(aboutAction, &QAction::triggered, this, &MainWindow::aboutTriggered_);
        connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
        connect(exitAction, &QAction::triggered, this, &MainWindow::close);
        connect(pingAction_, &QAction::triggered, this, &MainWindow::ping);
        connect(executeProxiesAction_, &QAction::triggered, [this] {
            executeProxiesAction_->setIcon(QIcon(":/icons/Power-Off-32x32.png"));
            executeProxiesAction_->setEnabled(false);
            // Set the position of both dialogs side to side to the center of the screen with a separation
            int separation = 60;
            int x = (QGuiApplication::primaryScreen()->geometry().width() - orderBooksDialog_->width() - recentTradesDialog_->width() - separation) / 2;
            int y = (QGuiApplication::primaryScreen()->geometry().height()) / 2;
            orderBooksDialog_->move(x, y);
            recentTradesDialog_->move(x + orderBooksDialog_->width() + separation, y);
            showTickersInformationDialogAction_->trigger();
            showRecentTradesDialogAction_->trigger();
            showOrderBooksDialogAction_->trigger();
            // Launch the configured proxies
            // Binance
            recentTradesDialog_->executeProxy(exchange::Service::Binance, exchange::Market::BTC_EUR, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Binance, exchange::Market::BTC_EUR, 60000);
            recentTradesDialog_->executeProxy(exchange::Service::Binance, exchange::Market::BTC_USDT, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Binance, exchange::Market::BTC_USDT, 60000);
            recentTradesDialog_->executeProxy(exchange::Service::Binance, exchange::Market::BTC_FDUSD, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Binance, exchange::Market::BTC_FDUSD, 60000);
            recentTradesDialog_->executeProxy(exchange::Service::Binance, exchange::Market::BTC_USDC, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Binance, exchange::Market::BTC_USDC, 60000);
            // Coinbase
            recentTradesDialog_->executeProxy(exchange::Service::Coinbase, exchange::Market::BTC_EUR, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Coinbase, exchange::Market::BTC_EUR, 60000);
            recentTradesDialog_->executeProxy(exchange::Service::Coinbase, exchange::Market::BTC_USD, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Coinbase, exchange::Market::BTC_USD, 60000);
            recentTradesDialog_->executeProxy(exchange::Service::Coinbase, exchange::Market::BTC_USDT, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Coinbase, exchange::Market::BTC_USDT, 60000);
            // Kraken 
            recentTradesDialog_->executeProxy(exchange::Service::Kraken, exchange::Market::BTC_EUR, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Kraken, exchange::Market::BTC_EUR, 60000);
            recentTradesDialog_->executeProxy(exchange::Service::Kraken, exchange::Market::BTC_USD, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Kraken, exchange::Market::BTC_USD, 60000);
            recentTradesDialog_->executeProxy(exchange::Service::Kraken, exchange::Market::BTC_USDT, 60000);
            orderBooksDialog_->executeProxy(exchange::Service::Kraken, exchange::Market::BTC_USDT, 60000); 
        });
        connect(showTickersInformationDialogAction_, &QAction::triggered, tickersInformationDialog_, &ProxiesManagerDialog::show);
        connect(showRecentTradesDialogAction_, &QAction::triggered, recentTradesDialog_, &ProxiesManagerDialog::show);
        connect(showOrderBooksDialogAction_, &QAction::triggered, orderBooksDialog_, &ProxiesManagerDialog::show);
        connect(marketComboBox_, &QComboBox::currentIndexChanged, this, &MainWindow::onMarketComboBoxChanged_);
        // Connect the signals of the dialogs
        connect(newDialog_, &NewDialog::createPortfolio, this, &MainWindow::createPortfolio);
        connect(orderDialog_, &OrderDialog::orderSimulated, this, &MainWindow::orderSimulated_);
        connect(transferDialog_, &TransferDialog::transferSimulated, this, &MainWindow::transferSimulated_);
        connect(orderForm_, &OrderForm::orderConfirmed, this, &MainWindow::orderConfirmed_);
        connect(transferForm_, &TransferForm::transferConfirmed, this, &MainWindow::transferConfirmed_);
        connect(tickersInformationDialog_, &ProxiesManagerDialog::createProxy, this, &MainWindow::createProxy);
        connect(tickersInformationDialog_, &ProxiesManagerDialog::slowdownProxy, this, &MainWindow::slowdownProxy);
        connect(tickersInformationDialog_, &ProxiesManagerDialog::pauseProxy, this, &MainWindow::pauseProxy);
        connect(tickersInformationDialog_, &ProxiesManagerDialog::resumeProxy, this, &MainWindow::resumeProxy);
        connect(tickersInformationDialog_, &ProxiesManagerDialog::destroyProxy, this, &MainWindow::destroyProxy);
        connect(recentTradesDialog_, &ProxiesManagerDialog::createProxy, this, &MainWindow::createProxy);
        connect(recentTradesDialog_, &ProxiesManagerDialog::slowdownProxy, this, &MainWindow::slowdownProxy);
        connect(recentTradesDialog_, &ProxiesManagerDialog::pauseProxy, this, &MainWindow::pauseProxy);
        connect(recentTradesDialog_, &ProxiesManagerDialog::resumeProxy, this, &MainWindow::resumeProxy);
        connect(recentTradesDialog_, &ProxiesManagerDialog::destroyProxy, this, &MainWindow::destroyProxy);
        connect(orderBooksDialog_, &ProxiesManagerDialog::createProxy, this, &MainWindow::createProxy);
        connect(orderBooksDialog_, &ProxiesManagerDialog::slowdownProxy, this, &MainWindow::slowdownProxy);
        connect(orderBooksDialog_, &ProxiesManagerDialog::pauseProxy, this, &MainWindow::pauseProxy);
        connect(orderBooksDialog_, &ProxiesManagerDialog::resumeProxy, this, &MainWindow::resumeProxy);
        connect(orderBooksDialog_, &ProxiesManagerDialog::destroyProxy, this, &MainWindow::destroyProxy);
        connect(mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::portfolioWindowActivated_);
        // Connect the signals of the portfolio tree
        connect(portfolioTree_, &PortfolioTree::portfolioSelected, this, &MainWindow::portfolioSelected_);
        connect(portfolioTree_, &PortfolioTree::savePortfolio, this, &MainWindow::savePortfolio_);
        connect(portfolioTree_, &PortfolioTree::savePortfolioAs, this, &MainWindow::savePortfolioAs_);
        connect(portfolioTree_, &PortfolioTree::closePortfolioWindow, this, &MainWindow::closePortfolioWindow_);
        // Connect the signals of the dock widgets
        connect(portfolioTreeDockWidget_, &QDockWidget::visibilityChanged, showPortfolioTreeAction, &QAction::setChecked);
        connect(systemLogDockWidget_, &QDockWidget::visibilityChanged, showSystemLogAction, &QAction::setChecked);
        connect(orderDockWidget_, &QDockWidget::visibilityChanged, showOrderFormAction, &QAction::setChecked);
        connect(transferDockWidget_, &QDockWidget::visibilityChanged, showTransferFormAction, &QAction::setChecked);

        // Disable the portfolio actions at the beginning
        enablePortfolioActions_(nullptr);

        orderDialog_->setModal(true);
        transferDialog_->setModal(true);

        exchange::Market market = marketComboBox_->currentData().value<exchange::Market>();
        orderForm_->setMarket(market);
        orderDialog_->setMarket(market);

        setTranslatableText_();

        // Set the minimum size of the main window
        setMinimumSize(1200, 600);
    }

    void MainWindow::createActions_() {
        newPortfolioAction = new QAction(this);
        newPortfolioAction->setIcon(QIcon(":/icons/New-32x32.png"));
        openPortfolioAction = new QAction(this);
        openPortfolioAction->setIcon(QIcon(":/icons/Folder-Open-32x32.png"));
        savePortfolioAction = new QAction(this);
        savePortfolioAction->setIcon(QIcon(":/icons/Save-32x32.png"));
        savePortfolioAsAction = new QAction(this);
        savePortfolioAsAction->setIcon(QIcon(":/icons/Save-New-32x32.png"));
        saveAllPortfoliosAction = new QAction(this);
        saveAllPortfoliosAction->setIcon(QIcon(":/icons/Save-Save-32x32.png"));
        exitAction = new QAction(this);
        exitAction->setIcon(QIcon(":/icons/Power-Off-32x32.png"));
        // View actions
        showPortfolioTreeAction = new QAction(this);
        showPortfolioTreeAction->setIcon(QIcon(":/icons/Window-32x32.png"));
        showPortfolioTreeAction->setCheckable(true);
        showPortfolioTreeAction->setChecked(true);
        showSystemLogAction = new QAction(this);
        showSystemLogAction->setIcon(QIcon(":/icons/Window-32x32.png"));
        showSystemLogAction->setCheckable(true);
        showSystemLogAction->setChecked(true);
        showOrderFormAction = new QAction(this);
        showOrderFormAction->setIcon(QIcon(":/icons/Window-32x32.png"));
        showOrderFormAction->setCheckable(true);
        showOrderFormAction->setChecked(true);
        showTransferFormAction = new QAction(this);
        showTransferFormAction->setIcon(QIcon(":/icons/Window-32x32.png"));
        showTransferFormAction->setCheckable(true);
        showTransferFormAction->setChecked(true);
        // Help actions
        enLanguageAction = new QAction(this);
        enLanguageAction->setIcon(QIcon(":/icons/en_US-32x32.png"));
        enLanguageAction->setCheckable(true);
        esLanguageAction = new QAction(this);
        esLanguageAction->setIcon(QIcon(":/icons/es_ES-32x32.png"));
        esLanguageAction->setCheckable(true);
        QActionGroup* languageActionGroup = new QActionGroup(this);
        languageActionGroup->addAction(enLanguageAction);
        languageActionGroup->addAction(esLanguageAction);
        languageActionGroup->setExclusive(true);
        esLanguageAction->setChecked(true);
        // Styles actions
        fusionStyleAction = new QAction(this);
        fusionStyleAction->setIcon(QIcon(":/icons/Style-32x32.png"));
        fusionStyleAction->setCheckable(true);
        windowsStyleAction = new QAction(this);
        windowsStyleAction->setIcon(QIcon(":/icons/Style-32x32.png"));
        windowsStyleAction->setCheckable(true);
        windowsVistaStyleAction = new QAction(this);
        windowsVistaStyleAction->setIcon(QIcon(":/icons/Style-32x32.png"));
        windowsVistaStyleAction->setCheckable(true);
        windows11StyleAction = new QAction(this);
        windows11StyleAction->setIcon(QIcon(":/icons/Style-32x32.png"));
        windows11StyleAction->setCheckable(true);
        // Set the styles actions excluyents betwen them
        QActionGroup* styleActionGroup = new QActionGroup(this);
        styleActionGroup->addAction(fusionStyleAction);
        styleActionGroup->addAction(windowsStyleAction);
        styleActionGroup->addAction(windowsVistaStyleAction);
        styleActionGroup->addAction(windows11StyleAction);
        styleActionGroup->setExclusive(true);
        windows11StyleAction->setChecked(true);
        // About actions
        aboutAction = new QAction(this);
        aboutAction->setIcon(QIcon(":/icons/Information-32x32.png"));
        aboutQtAction = new QAction(this);
        aboutQtAction->setIcon(QIcon(":/icons/Information-32x32.png"));
        // Exchange actions
        pingAction_ = new QAction(this);
        pingAction_->setIcon(QIcon(":/icons/Network-32x32.png"));
        executeProxiesAction_ = new QAction(this);
        executeProxiesAction_->setIcon(QIcon(":/icons/Power-On-32x32.png"));
        executeProxiesAction_->setCheckable(true);
        executeProxiesAction_->setChecked(false);
        showTickersInformationDialogAction_ = new QAction(this);
        showTickersInformationDialogAction_->setIcon(QIcon(":/icons/Information-32x32.png"));
        showRecentTradesDialogAction_ = new QAction(this);
        showRecentTradesDialogAction_->setIcon(QIcon(":/icons/Group-Tick-32x32.png"));
        showOrderBooksDialogAction_ = new QAction(this);
        showOrderBooksDialogAction_->setIcon(QIcon(":/icons/Notepad-Edit-32x32.png"));
    }

    void MainWindow::createMenubar_() {
        fileMenu_ = new QMenu(this);
        fileMenu_->addAction(newPortfolioAction);
        fileMenu_->addAction(openPortfolioAction);
        fileMenu_->addAction(savePortfolioAction);
        fileMenu_->addAction(savePortfolioAsAction);
        fileMenu_->addAction(saveAllPortfoliosAction);
        fileMenu_->addSeparator();
        fileMenu_->addAction(exitAction);
        menuBar()->addMenu(fileMenu_);

        viewMenu_ = new QMenu(this);
        viewMenu_->addAction(showPortfolioTreeAction);
        viewMenu_->addAction(showSystemLogAction);
        viewMenu_->addAction(showOrderFormAction);
        viewMenu_->addAction(showTransferFormAction);
        menuBar()->addMenu(viewMenu_);

        helpMenu_ = new QMenu(this);
        helpMenu_->addSeparator();  // "Language"
        helpMenu_->addAction(enLanguageAction);
        helpMenu_->addAction(esLanguageAction);
        helpMenu_->addSeparator();  // "Style"
        helpMenu_->addAction(fusionStyleAction);
        helpMenu_->addAction(windowsStyleAction);
        helpMenu_->addAction(windowsVistaStyleAction);
        helpMenu_->addAction(windows11StyleAction);
        helpMenu_->addSeparator();
        helpMenu_->addAction(aboutAction);
        helpMenu_->addAction(aboutQtAction);
        menuBar()->addMenu(helpMenu_);
    }

    void MainWindow::createToolbars_() {
        toolbar_ = new QToolBar(this);
        toolbar_->setContextMenuPolicy(Qt::PreventContextMenu);
        addToolBar(toolbar_);
        toolbar_->setMovable(true);
        toolbar_->setFloatable(false);
        toolbar_->addAction(newPortfolioAction);
        toolbar_->addAction(openPortfolioAction);
        toolbar_->addAction(savePortfolioAction);
        toolbar_->addAction(savePortfolioAsAction);
        toolbar_->addAction(saveAllPortfoliosAction);
        toolbar_->addSeparator();
        // Create the exchange toolbar
        exchangeToolbar_ = new QToolBar(this);
        exchangeToolbar_->setContextMenuPolicy(Qt::PreventContextMenu);
        addToolBar(exchangeToolbar_);
        exchangeToolbar_->setMovable(true);
        exchangeToolbar_->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        exchangeToolbar_->setFloatable(false);
        exchangeToolbar_->addAction(pingAction_);
        exchangeToolbar_->addAction(executeProxiesAction_);
        exchangeToolbar_->addAction(showTickersInformationDialogAction_);
        exchangeToolbar_->addAction(showRecentTradesDialogAction_);
        exchangeToolbar_->addAction(showOrderBooksDialogAction_);
        
        exchangeToolbar_->addSeparator();
        // Create the market combo box
        marketLabel_ = new QLabel(this);
        marketLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        marketLabel_->setFixedWidth(50);
        marketComboBox_ = new QComboBox(this);
        for(auto it : exchange::KrakenPairs) {
            marketComboBox_->addItem(exchange::getMarketName(it.first).c_str(), QVariant::fromValue(it.first));
        }
        marketComboBox_->setCurrentIndex(0);
        exchangeToolbar_->addWidget(marketLabel_);
        exchangeToolbar_->addWidget(marketComboBox_);
        QFont font;
        lastLabel_ = new QLabel(this);
        lastLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lastLabel_->setFixedWidth(45);
        lastLineEdit_ = new QLineEdit(this);
        lastLineEdit_->setReadOnly(true);
        lastLineEdit_->setFixedWidth(100);
        lastLineEdit_->setAlignment(Qt::AlignRight);
        font = lastLineEdit_->font();
        font.setStretch(QFont::Condensed);
        font.setPointSize(11);
        lastLineEdit_->setFont(font);
        exchangeToolbar_->addWidget(lastLabel_);
        exchangeToolbar_->addWidget(lastLineEdit_);
        tradesLabel_ = new QLabel(this);
        tradesLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tradesLabel_->setFixedWidth(80);
        tradesLineEdit_ = new QLineEdit(this);
        tradesLineEdit_->setReadOnly(true);
        tradesLineEdit_->setFixedWidth(70);
        tradesLineEdit_->setAlignment(Qt::AlignRight);
        font = tradesLineEdit_->font();
        font.setStretch(QFont::Condensed);
        font.setPointSize(11);
        tradesLineEdit_->setFont(font);
        exchangeToolbar_->addWidget(tradesLabel_);
        exchangeToolbar_->addWidget(tradesLineEdit_);
        lowLabel_ = new QLabel(this);
        lowLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lowLabel_->setFixedWidth(35);
        lowLineEdit_ = new QLineEdit(this);
        lowLineEdit_->setReadOnly(true);
        lowLineEdit_->setFixedWidth(100);
        lowLineEdit_->setAlignment(Qt::AlignRight);
        font = lowLineEdit_->font();
        font.setStretch(QFont::Condensed);
        font.setPointSize(11);
        lowLineEdit_->setFont(font);
        exchangeToolbar_->addWidget(lowLabel_);
        exchangeToolbar_->addWidget(lowLineEdit_);
        highLabel_ = new QLabel(this);
        highLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        highLabel_->setFixedWidth(35);
        highLineEdit_ = new QLineEdit(this);
        highLineEdit_->setReadOnly(true);
        highLineEdit_->setFixedWidth(100);
        highLineEdit_->setAlignment(Qt::AlignRight);
        font = highLineEdit_->font();
        font.setStretch(QFont::Condensed);
        font.setPointSize(11);
        highLineEdit_->setFont(font);
        exchangeToolbar_->addWidget(highLabel_);
        exchangeToolbar_->addWidget(highLineEdit_);
        volumeLabel_ = new QLabel(this);
        volumeLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        volumeLabel_->setFixedWidth(55);
        volumeLineEdit_ = new QLineEdit(this);
        volumeLineEdit_->setReadOnly(true);
        volumeLineEdit_->setFixedWidth(100);
        volumeLineEdit_->setAlignment(Qt::AlignRight);
        font = volumeLineEdit_->font();
        font.setStretch(QFont::Condensed);
        font.setPointSize(11);
        volumeLineEdit_->setFont(font);
        totalVolumeLineEdit_ = new QLineEdit(this);
        totalVolumeLineEdit_->setReadOnly(true);
        totalVolumeLineEdit_->setFixedWidth(100);
        totalVolumeLineEdit_->setAlignment(Qt::AlignRight);
        font = totalVolumeLineEdit_->font();
        font.setStretch(QFont::Condensed);
        font.setPointSize(11);
        totalVolumeLineEdit_->setFont(font);
        exchangeToolbar_->addWidget(volumeLabel_);
        exchangeToolbar_->addWidget(volumeLineEdit_);
        exchangeToolbar_->addWidget(totalVolumeLineEdit_);

        // Create the tickers information dialog
        tickersInformationDialog_ = new ProxiesManagerDialog(exchange::ProxyType::TickersInformation, this);
        tickersInformationDialog_->setWindowTitle(tr("tickers-information-dialog-title"));
        tickersInformationDialog_->setMinimumWidth(600);
        tickersInformationDialog_->setMaximumWidth(800);
        tickersInformationDialog_->setModal(false);
        tickersInformationDialog_->hide();

        // Create the recent trades dialog
        recentTradesDialog_ = new ProxiesManagerDialog(exchange::ProxyType::RecentTrades, this);
        recentTradesDialog_->setWindowTitle(tr("recent-trades-dialog-title"));
        recentTradesDialog_->setMinimumWidth(600);
        recentTradesDialog_->setMaximumWidth(800);
        recentTradesDialog_->setModal(false);
        recentTradesDialog_->hide();

        // Create the order books dialog
        orderBooksDialog_ = new ProxiesManagerDialog(exchange::ProxyType::OrderBook, this);
        orderBooksDialog_->setWindowTitle(tr("order-books-dialog-title"));
        orderBooksDialog_->setMinimumWidth(600);
        orderBooksDialog_->setMaximumWidth(800);
        orderBooksDialog_->setModal(false);
        orderBooksDialog_->hide();
    }

    void MainWindow::newPortfolioTriggered_() {
        // Show the new portfolio dialog
        newDialog_->show();
    }

    void MainWindow::openPortfolioTriggered_() {
        // Open a file dialog to select one or more portfolios
        QStringList filenames = QFileDialog::getOpenFileNames(nullptr, tr("open-portfolio"), QDir::currentPath(), tr("portfolio-file-type")); // "Portfolio files (*.bin)"
        for (const QString &filename : filenames) {
            if (!filename.isEmpty()) {
                emit openPortfolio(filename);
            }
        }
    }

    void MainWindow::savePortfolioTriggered_() {
        // Check if there is an active portfolio
        if(mdiArea->currentSubWindow()) {
            PortfolioWindow *portfolio = qobject_cast<PortfolioWindow *>(mdiArea->currentSubWindow()->widget());
            save_(portfolio);
        } else {
            systemLog_->warning(tr("no-active-portfolio"));
        }
    }

    void MainWindow::savePortfolioAsTriggered_() {
        // Check if there is an active portfolio
        if(mdiArea->currentSubWindow()) {
            PortfolioWindow *portfolio = qobject_cast<PortfolioWindow *>(mdiArea->currentSubWindow()->widget());
            saveAs_(portfolio);
        } else {
            systemLog_->warning(tr("no-active-portfolio"));
        }
    }

    void MainWindow::saveAllPortfoliosTriggered_() {
        for (QMdiSubWindow *subWindow : mdiArea->subWindowList()) {
            PortfolioWindow *portfolio = qobject_cast<PortfolioWindow *>(subWindow->widget());
            save_(portfolio);
        }
    }

    void MainWindow::showPortfolioTreeTriggered_() {
        showPortfolioTreeAction->isChecked() ? portfolioTreeDockWidget_->show() : portfolioTreeDockWidget_->hide();
    }
    void MainWindow::showSystemLogTriggered_() {
        showSystemLogAction->isChecked() ? systemLogDockWidget_->show() : systemLogDockWidget_->hide();
    }
    void MainWindow::showOrderFormTriggered_() {
        showOrderFormAction->isChecked() ? orderDockWidget_->show() : orderDockWidget_->hide();
    }
    void MainWindow::showTransferFormTriggered_() {
        showTransferFormAction->isChecked() ? transferDockWidget_->show() : transferDockWidget_->hide();
    }

    void MainWindow::aboutTriggered_() {
        QMessageBox::about(this, tr("about-dialog-title"), tr("about-dialog-body"));
    }

    void MainWindow::savePortfolio_(const QString &name) {
        try {
            PortfolioWindow* portfolio = getPortfolioWindow_(name);
            if(portfolio->isModified()) {
                save_(portfolio);
            }
        } catch (const std::exception &e) {
            systemLog_->warning(e.what());
        }
    }

    void MainWindow::savePortfolioAs_(const QString &name) {
        try {
            PortfolioWindow* portfolio = getPortfolioWindow_(name);
            saveAs_(portfolio);
        } catch (const std::exception &e) {
            systemLog_->warning(e.what());
        }
    }

    void MainWindow::readOnlyTimeout_(const PortfolioWindow* portfolio, unsigned int seconds) {
        const_cast<PortfolioWindow*>(portfolio)->setReadOnly(false);
        //systemLog_->warning("The portfolio '" + portfolio->getName() + "' is no longer read-only, but the operation has not been confirmed in " + QString::number(seconds) + " seconds");
        systemLog_->warning(tr("portfolio-readonly-timeout-warning").arg(portfolio->getName()).arg(seconds));
        updateStatusFromActivePortfolio_();
    }

    void MainWindow::closePortfolioWindow_(const QString &name) {
        try {
            getPortfolioSubWindow_(name)->close(); 
        } catch (const std::exception &e) {
            systemLog_->warning(e.what());
        }
    }


    void MainWindow::save_(PortfolioWindow *portfolio, bool feedback) {
        if (portfolio) {
            if(portfolio->getFilename().isEmpty()) {
                saveAs_(portfolio, feedback);
            }
            else {
                emit savePortfolio(portfolio->getName(), feedback);
            }
        }
        else {
            systemLog_->warning(tr("portfolio-is-null"));
        }
    }


    void MainWindow::saveAs_(PortfolioWindow *portfolio, bool feedback) {
        if (portfolio) {
            QString suggestedFilename = portfolio->getName() + ".bin";
            QString filename = QFileDialog::getSaveFileName(nullptr, tr("save-portfolio-as"), QDir::currentPath() + "/" + suggestedFilename, tr("portfolio-file-type"));
            if (!filename.isEmpty()) {
                emit savePortfolioAs(portfolio->getName(), filename, feedback);
            } else {
                systemLog_->info(tr("save-canceled-by-user"));
            }
        }
        else {
            systemLog_->warning(tr("portfolio-is-null"));
        }
    }


    void MainWindow::portfolioWindowActivated_(QMdiSubWindow *window) {
        PortfolioWindow * portfolio = nullptr;
        if (window) {
            portfolio = qobject_cast<PortfolioWindow *>(window->widget());
            enablePortfolioActions_(portfolio);
        } else {
            enablePortfolioActions_(nullptr);
        }
        orderForm_->updateView(portfolio);
        transferForm_->updateView(portfolio);
        orderDialog_->updateView(portfolio);
        transferDialog_->updateView(portfolio);
    }

    void MainWindow::portfolioSelected_(const QString &name) {
        // Find the portfolio window with the name 'name' and activate it
        for (QMdiSubWindow *subWindow : mdiArea->subWindowList()) {
            PortfolioWindow *portfolio = qobject_cast<PortfolioWindow *>(subWindow->widget());
            if (portfolio && portfolio->getName() == name) {
                mdiArea->setActiveSubWindow(subWindow);
                // Maximize the subwindow
                subWindow->showMaximized();
                return;
            }
        }
    }

    void MainWindow::portfolioWindowClosed_(PortfolioWindow* portfolioWindow) {
        // Get the portfolio name from the closed portfolioWindow
        if (portfolioWindow) {
            bool force = true;
            if(portfolioWindow->isModified()) {
                // Request the user to save the portfolio before closing
                //QString text = "Portfolio '" + portfolioWindow->getName() + "' has unsaved changes. Do you want to save them?";
                if(QMessageBox::question(this, tr("save-unsaved-title"), tr("portfolio-unsaved-body").arg(portfolioWindow->getName())) == QMessageBox::Yes) {
                    save_(portfolioWindow, false); // feedback = false
                }
            }
            emit closePortfolio(portfolioWindow->getName(), force);
            portfolioTree_->removePortfolioItem(portfolioWindow->getName());
        } else {
            systemLog_->warning(tr("no-active-portfolio"));
        }
    }

    void MainWindow::onMarketComboBoxChanged_(int index) {
        exchange::Market market = marketComboBox_->currentData().value<exchange::Market>();
        for(auto &window : mdiArea->subWindowList()) {
            PortfolioWindow *portfolio = dynamic_cast<PortfolioWindow*>(window->widget());
            if(portfolio) {
                portfolio->setMarket(market);
            }
        }
        orderForm_->setMarket(market);
        orderDialog_->setMarket(market);
        updateTickerInformation_();
    }

    PortfolioWindow* MainWindow::getActivePortfolio_() {
        if(mdiArea->currentSubWindow()) {
            return qobject_cast<PortfolioWindow *>(mdiArea->currentSubWindow()->widget());
        }
        return nullptr;
        
    }

    PortfolioWindow* MainWindow::getPortfolioWindow_(const QString &name)
    {
        for (QMdiSubWindow *subWindow : mdiArea->subWindowList()) {
            PortfolioWindow *portfolio = qobject_cast<PortfolioWindow *>(subWindow->widget());
            if (portfolio && portfolio->getName() == name) {
                return portfolio;
            }
        }
        throw std::runtime_error(tr("portfolio-window-not-found").arg(name).toStdString()); // "Portfolio window '%1' not found"
    }

    QMdiSubWindow* MainWindow::getPortfolioSubWindow_(const QString &name) 
    {
        for (QMdiSubWindow *subWindow : mdiArea->subWindowList()) {
            PortfolioWindow *portfolio = qobject_cast<PortfolioWindow *>(subWindow->widget());
            if (portfolio && portfolio->getName() == name) {
                return subWindow;
            }
        }
        throw std::runtime_error(tr("portfolio-window-not-found").arg(name).toStdString()); // "Portfolio window '%1' not found"
    }

    void MainWindow::enablePortfolioActions_(const PortfolioWindow* portfolio) const {
        bool enabled = portfolio != nullptr;
        bool portfolioIsModified = enabled && portfolio->isModified();
        // Enable/disable the save portfolio action
        savePortfolioAction->setEnabled(portfolioIsModified);
        savePortfolioAsAction->setEnabled(enabled);
        // Enable/disable the save all portfolios action
        bool enableSaveAllPortfoliosAction = false;
        for (QMdiSubWindow *subWindow : mdiArea->subWindowList()) {
            PortfolioWindow *portfolio = qobject_cast<PortfolioWindow *>(subWindow->widget());
            if (portfolio && portfolio->isModified()) {
                enableSaveAllPortfoliosAction = true;
                break;
            }
        }
        saveAllPortfoliosAction->setEnabled(enableSaveAllPortfoliosAction);
    }

    void MainWindow::setPortFolioFilename_(PortfolioWindow* portfolio, const QString& filename) {
        if(portfolio) {
            portfolio->setFilename(filename);
            portfolioTree_->updateView(portfolio->getName());
        }
        else {
            systemLog_->warning(tr("unable-to-set-portfolio-filename")); // Unable to set the filename of the portfolio window  (null pointer returned)
        }
    }

    void MainWindow::setPortfolioModified_(PortfolioWindow* portfolio, bool modified) {
        if(portfolio) {
            portfolio->setModified(modified);
            portfolioTree_->updateView(portfolio->getName());
        } else {
            systemLog_->warning(tr("unable-to-set-portfolio-modified"));    // Unable to set the modified flag of the portfolio window (null pointer returned)
        }
    }

    void MainWindow::updateStatusFromActivePortfolio_() {
        const PortfolioWindow* portfolio = getActivePortfolio_();
        enablePortfolioActions_(portfolio);
        portfolioTree_->updateView(portfolio->getName());
        orderForm_->updateView(portfolio);
        transferForm_->updateView(portfolio);
        orderDialog_->updateView(portfolio);
        transferDialog_->updateView(portfolio);
    }

    void MainWindow::setTranslatableText_() {
        setWindowTitle(tr("mainWindow-title"));
        // Dock widgets
        portfolioTreeDockWidget_->setWindowTitle(tr("portfolios-explorer-title"));
        systemLogDockWidget_->setWindowTitle(tr("system-log-title"));
        orderDockWidget_->setWindowTitle(tr("order-form-title"));
        transferDockWidget_->setWindowTitle(tr("transfer-form-title"));
        // Menus
        fileMenu_->setTitle(tr("file-menu"));
        viewMenu_->setTitle(tr("view-menu"));
        helpMenu_->setTitle(tr("help-menu"));
        // Toolbars
        toolbar_->setWindowTitle(tr("portfolio-operations-toolbar"));
        exchangeToolbar_->setWindowTitle(tr("market-information-toolbar"));
        // Actions
        newPortfolioAction->setText(tr("new-portfolio-action"));
        openPortfolioAction->setText(tr("open-portfolio-action"));
        savePortfolioAction->setText(tr("save-portfolio"));
        savePortfolioAsAction->setText(tr("save-portfolio-as-action"));
        saveAllPortfoliosAction->setText(tr("save-all-portfolios-action"));
        exitAction->setText(tr("close-application"));
        // View actions
        showPortfolioTreeAction->setText(tr("show-portfolio-tree-action"));
        showSystemLogAction->setText(tr("show-system-log-action"));
        showOrderFormAction->setText(tr("show-order-form-action"));
        showTransferFormAction->setText(tr("show-transfer-form-action"));
        // Help actions
        enLanguageAction->setText(tr("en-language-action"));
        esLanguageAction->setText(tr("es-language-action"));
        // Styles actions
        fusionStyleAction->setText("Fusion");
        windowsStyleAction->setText("Windows");
        windowsVistaStyleAction->setText("Windows Vista");
        windows11StyleAction->setText("Windows 11");
        aboutAction->setText(tr("about-action"));
        aboutQtAction->setText(tr("about-qt-action"));
        // Exchange actions
        pingAction_->setText(tr("ping-action"));
        executeProxiesAction_->setText(tr("execute-proxies-action"));
        showTickersInformationDialogAction_->setText(tr("show-tickers-information-dialog-action"));
        showRecentTradesDialogAction_->setText(tr("show-recent-trades-dialog-action"));
        showOrderBooksDialogAction_->setText(tr("show-order-books-dialog-action"));
        // Labels and tooltips
        marketComboBox_->setToolTip(tr("write-first-letters-to-search"));
        marketLabel_->setText(tr("market-label"));
        marketLabel_->setToolTip(tr("market-label-tooltip"));
        lastLabel_->setText(tr("last-label"));
        lastLabel_->setToolTip(tr("Last-label-tooltip"));
        lastLineEdit_->setToolTip(tr("last-lineedit-tooltip"));
        tradesLabel_->setText(tr("trades-label"));
        tradesLabel_->setToolTip(tr("trades-label-tooltip"));
        tradesLineEdit_->setToolTip(tr("trades-lineedit-tooltip"));
        lowLabel_->setText(tr("low-label"));
        lowLabel_->setToolTip(tr("low-label-tooltip"));
        lowLineEdit_->setToolTip(tr("low-lineedit-tooltip"));
        highLabel_->setText(tr("high-label"));
        highLabel_->setToolTip(tr("high-label-tooltip"));
        highLineEdit_->setToolTip(tr("high-lineedit-tooltip"));
        volumeLabel_->setText(tr("volume-label"));
        volumeLabel_->setToolTip(tr("volume-label-tooltip"));
        volumeLineEdit_->setToolTip(tr("volume-lineedit-tooltip"));
        totalVolumeLineEdit_->setToolTip(tr("total-volume-lineedit-tooltip"));
        frequencyLabel_->setText(tr("kraken-proxy-frequency-label"));
    }

    void MainWindow::closeEvent(QCloseEvent *event) {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, tr("close-mainwindow-title"), tr("close-mainwindow-body"), QMessageBox::Cancel | QMessageBox::Yes, QMessageBox::Yes); // Are you sure you want to exit?
        if (resBtn != QMessageBox::Yes) {
            event->ignore();
        } else {
            // Close all subwindows
            mdiArea->closeAllSubWindows();
            emit finalize();
            //event->accept(); // Accept the event to proceed with closing
            QMainWindow::closeEvent(event);
        }
    }

} // namespace hmi
