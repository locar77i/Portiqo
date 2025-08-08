// headers
#include "PortfolioWindow.h"

// Qt headers
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QHeaderView>
#include <QtCore/qmath.h>

// Main headers
#include "Types.h"

// hmi headers
#include "hmi/MyQtUtils.h"


namespace hmi
{

    // funtion that returns a diferent color for every OperationType
    const char* getStyleSheet_(exchange::OperationType type) {
        switch (type) {
            case exchange::OperationType::Buy:
                return BuyStyleSheet;
            case exchange::OperationType::Sell:
                return SellStyleSheet;
            case exchange::OperationType::Deposit:
                return DepositStyleSheet;
            case exchange::OperationType::Withdraw:
                return WithdrawStyleSheet;
            default:
                return ErrorStyleSheet;
        }
    }

    // funtion that returns a diferent color for every OperationType
    QColor getColor_(exchange::OperationType type) {
        switch (type) {
            case exchange::OperationType::Buy:
                return QColor(BUYCOLOR_FG);
            case exchange::OperationType::Sell:
                return QColor(SELLCOLOR_FG);
            case exchange::OperationType::Deposit:
                return QColor(DEPOSITCOLOR_FG);
            case exchange::OperationType::Withdraw:
                return QColor(WITHDRAWCOLOR_FG);
            default:
                return Qt::red;
        }
    }

    // funtion that returns a diferent color for every asset type
    QColor getColor_(exchange::Ticket ticket) {
        switch (ticket) {
            case exchange::Ticket::EUR:
                return QColor("#1C1F2A");           
            case exchange::Ticket::USD:
                return QColor("#6B7B8C");
            case exchange::Ticket::GBP:
                return QColor("#1C1F2A");
            case exchange::Ticket::BTC:
                return QColor("#F7931A");
            case exchange::Ticket::ETH:
                return QColor("#3D3E3F");
            case exchange::Ticket::LTC:
                return QColor("#345D9D");
            case exchange::Ticket::BCH:
                return QColor("#8DC351");
            case exchange::Ticket::DASH:
                return QColor("#1C75BC");
            case exchange::Ticket::XRP:
                return QColor("#346AA9");
            case exchange::Ticket::ADA:
                return QColor("#0088CC");
            default:
                return QColor("#393939");
        }
    }


    PortfolioWindow::PortfolioWindow(const exchange::PortfolioData& data, exchange::Market market, QWidget *parent)
        : QMainWindow(parent)
        , mainWidget_(nullptr)
        , portfolioBalance_(nullptr)
        , portfolioBalanceDetails_(nullptr)
        , statusBarTimer_(this)
        , balance_()
        , name_(data.name.c_str())
        , filename_(data.filename.c_str())
        , referenceCurrency_(data.referenceCurrency)
        , modified_(false)
        , locked_(true)
        , readOnly_(false)
        , market_()
        , asset_()
        , currency_()
        , stats_()
        , tickersInformation_()
        , multiplier_(1.0)
        , increments_()
    {
        init_();
        setWindowTitle(name_);
        setMarket(market);
        addOperations_(data.operations);
        setBalance_(data.balance);
        stats_ = data.stats;
        statsWidget_->updateView(
            stats_.getOverview(tickersInformation_.get(), multiplier_, increments_)
        );
    }

    PortfolioWindow::~PortfolioWindow() {
        emit trace("Portfolio window '" + name_ + "' destroyed");
    }

    void PortfolioWindow::setModified(bool modified) {
        if(modified != modified_) {
            modified_ = modified;
            updateWindowTitle_();
            enablePortfolioActions_(modified_);
        }
    }

    void PortfolioWindow::setReadOnly(bool readOnly) {
        readOnly_ = readOnly;
        updateWindowTitle_();
        if(readOnly_) {
            lockPortfolioAction->setEnabled(false);
            orderAssetAction->setEnabled(false);
            transferAssetAction->setEnabled(false);
            removeLastPortfolioOperationAction->setEnabled(false);
            statusBarTimer_.start(); // Start the timer: emit a timeout signal every 1000 ms = 1s by default
        } else {
            lockPortfolioAction->setEnabled(true);
            orderAssetAction->setEnabled(true);
            transferAssetAction->setEnabled(true);
            removeLastPortfolioOperationAction->setEnabled(true);
            statusBarTimer_.stop();
        }
    }

    void PortfolioWindow::updateView(const exchange::PortfolioUpdate& data) {
        addOperation_(data.operation);
        setBalance_(data.balance);
        stats_ = data.stats;
        statsWidget_->updateView(
            stats_.getOverview(tickersInformation_.get(), multiplier_, increments_)
        );
    }

    void PortfolioWindow::updateView(const exchange::PortfolioRemove& data) {
        removeOperation_(data.timePoint);
        setBalance_(data.balance);
        stats_ = data.stats;
        statsWidget_->updateView(
            stats_.getOverview(tickersInformation_.get(), multiplier_, increments_)
        );
    }

    void PortfolioWindow::setMarket(exchange::Market market) {
        market_ = market;
        asset_ = exchange::getAssetType(market);
        currency_ = exchange::getCurrencyType(market);
        updatePriceModifiers_();
        updateSimulatedLastPrice_();
    }

    void PortfolioWindow::setTickersInformation(const exchange::TickersInformation& tickersInformation) {
        tickersInformation_ = tickersInformation;
        statsWidget_->updateView(
            stats_.getOverview(tickersInformation_.get(), multiplier_, increments_)
        );
        setBalance_(balance_);
        updateSimulatedLastPrice_();
    }

    double PortfolioWindow::getSimulatedLastPrice(exchange::Market market) const {
        double lastPrice;
        exchange::Ticket asset = exchange::getAssetType(market);
        auto it = increments_.find(asset);
        if(it != increments_.end()) {
            lastPrice = tickersInformation_.getLastPrice(market) * (multiplier_ + it->second);
        } else {
            lastPrice = tickersInformation_.getLastPrice(market) * multiplier_;
        }
        return lastPrice;
    }

    void PortfolioWindow::retranslateUi() {
        setTranslatableText_();
        mainWidget_->retranslateUi();
        portfolioBalance_->retranslateUi();
        statsWidget_->retranslateUi();
        portfolioBalanceDetails_->retranslateUi();
    }
    
    void PortfolioWindow::init_() {
        // Create the toolbar of the portfolio window
        toolbar_ = new QToolBar(this);
        toolbar_->setMovable(true);
        toolbar_->setFloatable(false);
        toolbar_->setContextMenuPolicy(Qt::PreventContextMenu);
        addToolBar(Qt::TopToolBarArea, toolbar_);
        // Create the actions of the toolbar
        savePortfolioAction = new QAction(this);
        savePortfolioAction->setIcon(QIcon(":/icons/Save-32x32.png"));
        toolbar_->addAction(savePortfolioAction);
        savePortfolioAsAction = new QAction(this);
        savePortfolioAsAction->setIcon(QIcon(":/icons/Save-New-32x32.png"));
        toolbar_->addAction(savePortfolioAsAction);
        toolbar_->addSeparator();
        showDocksAction = new QAction(this);
        showDocksAction->setIcon(QIcon(":/icons/Window-32x32.png"));
        toolbar_->addAction(showDocksAction);
        toolbar_->addSeparator();
        lockPortfolioAction = new QAction(this);
        lockPortfolioAction->setIcon(QIcon(":/icons/Padlock-32x32.png"));
        lockPortfolioAction->setCheckable(true);
        lockPortfolioAction->setChecked(true);
        toolbar_->addAction(lockPortfolioAction);
        orderAssetAction = new QAction(this);
        orderAssetAction->setIcon(QIcon(":/icons/Copy-Import-32x32.png"));
        toolbar_->addAction(orderAssetAction);
        transferAssetAction = new QAction(this);
        transferAssetAction->setIcon(QIcon(":/icons/Copy-Redo-32x32.png"));
        toolbar_->addAction(transferAssetAction);
        removeLastPortfolioOperationAction = new QAction(this);
        removeLastPortfolioOperationAction->setIcon(QIcon(":/icons/Copy-Delete-32x32.png"));
        toolbar_->addAction(removeLastPortfolioOperationAction);
        orderAssetAction->setVisible(false);
        transferAssetAction->setVisible(false);
        removeLastPortfolioOperationAction->setVisible(false);

        // Create the price toolbar of the portfolio window
        priceToolbar_ = new QToolBar(this);
        priceToolbar_->setContextMenuPolicy(Qt::PreventContextMenu);
        priceToolbar_->setMovable(true);
        priceToolbar_->setFloatable(false);
        priceToolbar_->setVisible(false);
        priceToolbar_->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        //priceToolbar_->setMaximumHeight(34);
        addToolBar(Qt::TopToolBarArea, priceToolbar_);
        // Create the simulated last label
        simulatedLastLabel_ = new QLabel(this);
        simulatedLastLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        simulatedLastLabel_->setFixedWidth(40);
        // Create the simulated last line edit
        simulatedLastLineEdit_ = new QLineEdit(this);
        simulatedLastLineEdit_->setReadOnly(true);
        simulatedLastLineEdit_->setFixedWidth(100);
        simulatedLastLineEdit_->setAlignment(Qt::AlignRight);
        QFont font = simulatedLastLineEdit_->font();
        font.setStretch(QFont::Condensed);
        font.setPointSize(11);
        simulatedLastLineEdit_->setFont(font);
        // Create the market modifier
        marketModifier_ = new LabeledSlider(this);
        marketModifier_->setRange(-80, 300);
        marketModifier_->setValue(0);
        marketModifier_->setMaximumWidth(320);
        // Create the asset modifier
        assetModifier_ = new LabeledSlider(this);
        assetModifier_->setRange(-15, 200);
        assetModifier_->setValue(0);
        assetModifier_->setMaximumWidth(320);
        // add the widgets to the price toolbar
        priceToolbar_->addWidget(simulatedLastLabel_);
        priceToolbar_->addWidget(simulatedLastLineEdit_);
        priceToolbar_->addSeparator();
        priceToolbar_->addWidget(marketModifier_);
        priceToolbar_->addSeparator();
        priceToolbar_->addWidget(assetModifier_);

        // Create the status bar of the portfolio window
        statusBar_ = new QStatusBar(this);
        setStatusBar(statusBar_);
        statusBar_->setVisible(false);
        // Auto-hide the status bar when there are no messages to show
        connect(statusBar_, &QStatusBar::messageChanged, this, [this](const QString &message) {
            statusBar_->setVisible(!message.isEmpty());
        });
        
        // Create the central widget of the portfolio window
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        // Set the main and bottom widgets of the portfolio window
        mainWidget_ = new PortfolioMain(this);
        statsWidget_ = new PortfolioStats(referenceCurrency_, this);
        statsWidget_->setFixedHeight(80);
        // Create the side widgets of the portfolio window
        portfolioBalance_ = new PortfolioBalance(referenceCurrency_, this);
        portfolioBalance_->setMinimumWidth(320);
        portfolioBalanceDetails_ = new PortfolioBalanceDetails(this);
        portfolioBalanceDetails_->setMinimumWidth(320);

        // EXPERIMENTAL
        balanceDock_ = new QDockWidget(tr("balance"), this);
        balanceDock_->setWidget(portfolioBalance_);
        balanceDetailsDock_ = new QDockWidget(tr("balance-details"), this);
        balanceDetailsDock_->setWidget(portfolioBalanceDetails_);
        addDockWidget(Qt::RightDockWidgetArea, balanceDock_);
        addDockWidget(Qt::RightDockWidgetArea, balanceDetailsDock_);

        // Create the main layout of the portfolio window
        mainLayout_ = new QVBoxLayout(centralWidget);
        mainLayout_->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        mainLayout_->addWidget(mainWidget_);
        mainLayout_->addWidget(statsWidget_);

        // set the main layout of the portfolio window
        centralWidget->setLayout(mainLayout_);

        // Disable the portfolio actions
        enablePortfolioActions_(false);

        setTranslatableText_();

        setMinimumSize(720, 240); // The minimum size of the portfolio window

        // Connections
        connect(savePortfolioAction, &QAction::triggered, this, &PortfolioWindow::savePortfolioTriggered_);
        connect(savePortfolioAsAction, &QAction::triggered, this, &PortfolioWindow::savePortfolioAsTriggered_);
        connect(lockPortfolioAction, &QAction::toggled, this, &PortfolioWindow::lockPortfolioToggled_);
        connect(showDocksAction, &QAction::triggered, [this]() {
            balanceDock_->setVisible(true);
            balanceDetailsDock_->setVisible(true);
        });
        connect(orderAssetAction, &QAction::triggered, this, &PortfolioWindow::orderAsset);
        connect(transferAssetAction, &QAction::triggered, this, &PortfolioWindow::transferAsset);
        connect(removeLastPortfolioOperationAction, &QAction::triggered, this, &PortfolioWindow::removeLastPortfolioOperationTriggered_);
        connect(portfolioBalance_, &PortfolioBalance::showBalanceInformation, this, &PortfolioWindow::showBalanceInformation_);
        connect(portfolioBalance_, &PortfolioBalance::buyAssetUsing, [this](exchange::Ticket asset, double multiplier) {
            buyAssetUsing(this, asset, multiplier);
        });
        connect(portfolioBalance_, &PortfolioBalance::buyAssetUsingTheProfit, [this](exchange::Ticket asset) {
            buyAssetUsingTheProfit(this, asset);
        });
        connect(portfolioBalance_, &PortfolioBalance::sellAssetUsing, [this](exchange::Ticket asset, double multiplier) {
            sellAssetUsing(this, asset, multiplier);
        });
        connect(portfolioBalance_, &PortfolioBalance::sellAssetToLeaveTheProfit, [this](exchange::Ticket asset) {
            sellAssetToLeaveTheProfit(this, asset);
        });
        connect(marketModifier_, &LabeledSlider::valueChanged, [this](int value) {
            float multiplier = 1 + (float)value/100;
            setMultiplier_(multiplier);
            updateSimulatedLastPrice_();
        });
        connect(assetModifier_, &LabeledSlider::valueChanged, [this](int value) {
            setIncrement_(asset_, (float)value/100);
            updateSimulatedLastPrice_();
        });

        connect(portfolioBalanceDetails_, &PortfolioBalanceDetails::calculateDetailsFor, this, &PortfolioWindow::calculateDetailsFor); 

        connect(&statusBarTimer_, &CounterTimer::timeout, [this](unsigned int seconds) {
            if(seconds > 0 && seconds < 25) {
                statusBar_->showMessage(tr("waiting-backend-response").arg(seconds), 1000); // Elapsed time: %1s. Waitting response from the backend...
            } else if(seconds >= 25 && seconds < 30) {
                statusBar_->showMessage(tr("backend-not-responding").arg(seconds), 1000); // Elapsed time: %1s. The backend is not responding...
            } else if(seconds == 30) {
                emit readOnlyTimeout(this, seconds);
                statusBar_->showMessage(tr("backend-response-is-lost"), 5000);  // The response from the backend has been lost. The portfolio is now unlocked.
            } else if(seconds == 36) {
                statusBarTimer_.stop();
            }
        });
    }


    void PortfolioWindow::addOperations_(const std::vector<std::shared_ptr<exchange::Operation>>& operations) {
        // Set the operations in the main part of the portfolio window
        if (mainWidget_) {
            mainWidget_->setRowCount(static_cast<int>(operations.size()));
            for (int row = 0; row < static_cast<int>(operations.size()); ++row) {
                mainWidget_->setOperation(row, operations[row]);
            }
        }
    }

    void PortfolioWindow::addOperation_(std::shared_ptr<exchange::Operation> operation) {
        if (mainWidget_) {
            // remove sorting to avoid problems
            int col = mainWidget_->horizontalHeader()->sortIndicatorSection();
            mainWidget_->setSortingEnabled(false);
            //
            int row = mainWidget_->rowCount();
            mainWidget_->setRowCount(row + 1);
            if(operation) { 
                mainWidget_->setOperation(row, operation);
            }
            // restore sorting
            mainWidget_->setSortingEnabled(true);
            mainWidget_->sortByColumn(col, mainWidget_->horizontalHeader()->sortIndicatorOrder());
        }
    }

    void PortfolioWindow::removeOperation_(std::chrono::system_clock::time_point timePoint) {
        // Remove the operation using the timePoint parameter
        if (mainWidget_) {
            int row = -1;
            for (int ii = 0; ii < mainWidget_->rowCount(); ++ii) {
                TimePointTableWidgetItem* item = dynamic_cast<TimePointTableWidgetItem*>(mainWidget_->item(ii, 0));
                if (item && item->getTimePoint() == timePoint) {
                    row = ii;
                    break;
                }
            }
            if (row >= 0) {
                mainWidget_->removeRow(row);
            }
        }
    }

    void PortfolioWindow::updateWindowTitle_() {
        QString title = name_;
        if(modified_) {
            title += "*";
        }
        if(readOnly_) {
            title += " " + tr("read-only-mode");
        }
        setWindowTitle(title);
    }
    
    void PortfolioWindow::enablePortfolioActions_(bool portfolioIsModified) {
        savePortfolioAction->setEnabled(portfolioIsModified);
    }

    void PortfolioWindow::updateSimulatedLastPrice_() {
        simulatedLastLineEdit_->setText(MyQtUtils::stringfyPriceHR(getSimulatedLastPrice(), asset_, currency_));
    }

    void PortfolioWindow::updatePriceModifiers_() {
        // Update the price slider (multiplier)
        if(currency_ == exchange::Ticket::EUR || currency_ == exchange::Ticket::USD || currency_ == exchange::Ticket::GBP) {
            marketModifier_->blockSignals(true);
            marketModifier_->setValue((getMultiplier() - 1) * 100);
            marketModifier_->blockSignals(false);
            marketModifier_->setEnabled(true);
        }
        else {
            marketModifier_->blockSignals(true);
            marketModifier_->setValue(0);
            marketModifier_->blockSignals(false);
            marketModifier_->setEnabled(false);
        }
        // Update the asset modifier
        assetModifier_->setValue(static_cast<int>(getIncrement(asset_) * 100));
    }

    void PortfolioWindow::setBalance_(const exchange::GeneralBalance& generalBalance) {
        balance_ = generalBalance;
        // Updates the balance widget with the provided general balance. Only includes balances that have entries.
        if (portfolioBalance_) {
            int totalRows = static_cast<int>(generalBalance.getBalances().size());
            portfolioBalance_->setRowCount(totalRows);
            int row = 0;
            for (const auto& item : generalBalance.getBalances()) {
                const auto& balance = item.second;
                if(!balance.getBalanceEntries().empty()) {
                    try {
                        exchange::Market market = exchange::getMarket(balance.getAsset(), referenceCurrency_);
                        double averagePrice = stats_.getInvestmentAveragePrice(market);
                        portfolioBalance_->setBalance(row, balance, averagePrice, getSimulatedLastPrice(market), referenceCurrency_);
                    } catch (const std::exception&) {
                        portfolioBalance_->setBalance(row, balance, 1, 1, balance.getAsset());
                    }
                    ++row;
                }
            }
            // Clear extra rows if any in reverse order to avoid invalidating the indexes
            for (int ii = totalRows - 1; ii >= row; --ii) {
                portfolioBalance_->removeRow(ii);
            }
        }
    }

    void PortfolioWindow::setTranslatableText_() {
        toolbar_->setWindowTitle(tr("portfolio-operations-toolbar"));
        savePortfolioAction->setText(tr("save-portfolio-action"));
        savePortfolioAsAction->setText(tr("save-portfolio-as-action"));
        lockPortfolioAction->setText(tr("lock-portfolio-action"));
        orderAssetAction->setText(tr("order-asset-action"));
        transferAssetAction->setText(tr("transfer-asset-action"));
        removeLastPortfolioOperationAction->setText(tr("remove-last-operation-action"));
        priceToolbar_->setWindowTitle(tr("price-management-toolbar"));
        simulatedLastLabel_->setText(tr("simulated-last-label"));
        simulatedLastLabel_->setToolTip(tr("simulated-last-tooltip"));
        simulatedLastLineEdit_->setToolTip(tr("simulated-last-tooltip"));
        marketModifier_->setText(tr("market-modifier-label"));
        marketModifier_->setToolTip(tr("market-modifier-tooltip"));
        assetModifier_->setText(tr("asset-modifier-label"));
        assetModifier_->setToolTip(tr("asset-modifier-tooltip"));
    }

    // PRIVATE SLOTS
    void PortfolioWindow::lockPortfolioToggled_(bool locked) {
        locked_ = locked;
        lockPortfolioAction->setIcon(locked? QIcon(":/icons/Padlock-32x32.png") : QIcon(":/icons/Padlock-Delete-32x32.png"));
        orderAssetAction->setVisible(!locked);
        transferAssetAction->setVisible(!locked);
        removeLastPortfolioOperationAction->setVisible(!locked);
        priceToolbar_->setVisible(!locked);
        if(locked) {
            resetModifiers_();
            updatePriceModifiers_();
        }
        emit lockPortfolio(name_, locked);
    }

    void PortfolioWindow::removeLastPortfolioOperationTriggered_() {
        if (QMessageBox::question(this, tr("confirm-remove"), tr("confirm-remove-question")) == QMessageBox::Yes) {  // Are you sure you want to remove the last operation?
            emit removeLastPortfolioOperation(this);
        }
    }

    void PortfolioWindow::showBalanceInformation_(const exchange::Balance& balance) {
        // Show the balance details in the sidebar of the portfolio window
        portfolioBalanceDetails_->setBalanceDetails(balance.getAsset(), balance.getBalanceEntries());
        // When the dock is hidden, show it
        if(balanceDetailsDock_->isHidden()) {
            balanceDetailsDock_->setVisible(true);
        }
        // When the dock is stacked and its not the current dock, show it at the front of the stack
        if(balanceDetailsDock_->isFloating() && balanceDetailsDock_->isHidden()) {
            balanceDetailsDock_->show();
        }
        // Raise the balanceDetailsDock_ to make it the visible tab
        balanceDetailsDock_->raise();
    }

    void PortfolioWindow::closeEvent(QCloseEvent *event) {
        emit closed(this);
        event->accept(); // Accept the event to proceed with closing
    }


    // Implement the PortfolioMain class
    PortfolioMain::PortfolioMain(QWidget *parent)
        : QTableWidget(parent) {
        init_();
    }

    PortfolioMain::~PortfolioMain() {
        // Destructor implementation
    }

    void PortfolioMain::setTranslatableText_(unsigned int row) {
        // Type:
        exchange::OperationType type = item(row, 1)->data(Qt::UserRole).value<exchange::OperationType>();
        if(type == exchange::OperationType::Buy) {
            exchange::Buy buy = item(row, 5)->data(Qt::UserRole).value<exchange::Buy>();
            // %1: quantity, %2: price, %3: total, %4: fee
            item(row, 5)->setText(tr("buy-description")
                .arg(MyQtUtils::stringfy(buy.getQuantity(), buy.getAsset(), true))
                .arg(MyQtUtils::stringfy(buy.getPrice(), buy.getCurrency(), true))
                .arg(MyQtUtils::stringfy(buy.getTotal(), buy.getCurrency(), true))
                .arg(MyQtUtils::stringfy(buy.getFee(), buy.getCurrency(), true))
            );
        } else if(type == exchange::OperationType::Sell) {
            exchange::Sell sell = item(row, 5)->data(Qt::UserRole).value<exchange::Sell>();
            item(row, 5)->setText(tr("sell-description")
                .arg(MyQtUtils::stringfy(sell.getQuantity(), sell.getAsset(), true))
                .arg(MyQtUtils::stringfy(sell.getPrice(), sell.getCurrency(), true))
                .arg(MyQtUtils::stringfy(sell.getTotal(), sell.getCurrency(), true))
                .arg(MyQtUtils::stringfy(sell.getFee(), sell.getCurrency(), true))
            );  
        } else if(type == exchange::OperationType::Deposit) {
            exchange::Deposit deposit = item(row, 5)->data(Qt::UserRole).value<exchange::Deposit>();
            item(row, 5)->setText(tr("deposit-description")
                .arg(MyQtUtils::stringfy(deposit.getQuantity(), deposit.getAsset(), true))
                .arg(MyQtUtils::stringfy(deposit.getFee(), deposit.getAsset(), true))
            );
            
        } else if(type == exchange::OperationType::Withdraw) {
            exchange::Withdraw withdraw = item(row, 5)->data(Qt::UserRole).value<exchange::Withdraw>();
            item(row, 5)->setText(tr("withdraw-description")
                .arg(MyQtUtils::stringfy(withdraw.getQuantity(), withdraw.getAsset(), true))
                .arg(MyQtUtils::stringfy(withdraw.getFee(), withdraw.getAsset(), true))
            );
        }
    }

    void PortfolioMain::setOperation(unsigned int row, std::shared_ptr<exchange::Operation> operation) {
        QTableWidgetItem* item;
        QFont font;
        // Get the selected row to keep it selected after the update
        TimePointTableWidgetItem* selectedItem = nullptr;
        if(!selectedItems().isEmpty()) {
            selectedItem = dynamic_cast<TimePointTableWidgetItem*>(selectedItems().first());
        }
        exchange::Order* order = nullptr;
        double price = 0.0;
        double quantity = 0.0;
        double total = 0.0;
        double fee = 0.0;
        if(operation->getType() == exchange::OperationType::Buy || operation->getType() == exchange::OperationType::Sell) {
            order = dynamic_cast<exchange::Order*>(operation.get());
            if(order) {
                price = order->getPrice();
                quantity = order->getQuantity();
                total = order->getTotal();
                fee = order->getFee();
            }
        }
        // Date: set the date of the operation with the format "DD/MM/YYYY hh:mm:ss"
        item = new TimePointTableWidgetItem(QString(formatTimePoint(operation->getTimePoint())), operation->getTimePoint());
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        item->setToolTip("ID: " + QString::fromStdString(operation->getId()));
        item->setData(Qt::UserRole, QVariant::fromValue(operation->getId()));
        setItem(row, 0, item);
        // Operation: set the operation type
        item = new QTableWidgetItem(QString::fromStdString(operation->getName()));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        item->setBackground(MyQtUtils::getBackgroundColor(operation->getType()));
        item->setForeground(MyQtUtils::getForegroundColor(operation->getType()));
        item->setData(Qt::UserRole, QVariant::fromValue(operation->getType()));
        setItem(row, 1, item);
        // Quantity:
        item = new QTableWidgetItem(MyQtUtils::stringfy(operation->getQuantity(), operation->getAsset()));
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        item->setForeground(QColor("#393939"));
        font = item->font();
        font.setPointSize(11);
        font.setStretch(QFont::Condensed);
        item->setFont(font);
        if(order) {
            item->setToolTip(MyQtUtils::stringfy(total, order->getCurrency(), true));
        }
        setItem(row, 2, item);
        // Asset:
        item = new QTableWidgetItem(QString::fromStdString(operation->getAssetName()));
        item->setForeground(getColor_(operation->getAsset()));
        font = item->font();
        font.setPointSize(11);
        font.setStretch(QFont::Condensed);
        font.setBold(true);
        item->setFont(font);
        item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        if(order) {
            item->setToolTip(MyQtUtils::stringfy(price, order->getCurrency(), true));
        }
        setItem(row, 3, item);
        // Fee:
        item = new QTableWidgetItem(MyQtUtils::stringfy(operation->getFee(), exchange::Ticket::EUR, true));
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        setItem(row, 4, item);
        // Description:
        item = new QTableWidgetItem();
        if(operation->getType() == exchange::OperationType::Buy) {
            exchange::Buy buy = dynamic_cast<exchange::Buy&>(*operation);
            item->setData(Qt::UserRole, QVariant::fromValue(buy));
        } else if(operation->getType() == exchange::OperationType::Sell) {
            exchange::Sell sell = dynamic_cast<exchange::Sell&>(*operation);
            item->setData(Qt::UserRole, QVariant::fromValue(sell));
        } else if(operation->getType() == exchange::OperationType::Deposit) {
            exchange::Deposit deposit = dynamic_cast<exchange::Deposit&>(*operation);
            item->setData(Qt::UserRole, QVariant::fromValue(deposit));
        } else if(operation->getType() == exchange::OperationType::Withdraw) {
            exchange::Withdraw withdraw = dynamic_cast<exchange::Withdraw&>(*operation);
            item->setData(Qt::UserRole, QVariant::fromValue(withdraw));
        }
        setItem(row, 5, item);
        // Select the row that was selected before the update
        if(selectedItem) { // If there are a row selected, select it
            selectRow_(selectedItem->getTimePoint());
        } else { // If not rows selected, select the last one
            selectRow_(operation->getTimePoint());
        }

        setTranslatableText_(row);
    }

    void PortfolioMain::retranslateUi() {
        setTranslatableText_();
    }

    void PortfolioMain::init_() {
        setColumnCount(6);
        // Set a different color for the header
        horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #f0f0f0 }");
        // Remove the vertical header
        verticalHeader()->setVisible(false);
        // Configure diferent stretch for each column
        horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed); setColumnWidth(1, 90);
        horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed); setColumnWidth(2, 80);
        horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed); setColumnWidth(3, 60);
        horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed); setColumnWidth(4, 80);
        horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
        // Disable editing
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        // Set the selection behavior to select rows
        setSelectionBehavior(QAbstractItemView::SelectRows);
        // Set the selection mode to single selection
        setSelectionMode(QAbstractItemView::SingleSelection);
        // Set the row height
        verticalHeader()->setDefaultSectionSize(18);
        // Set rows color
        setAlternatingRowColors(false);
        // Enable sorting
        setSortingEnabled(true);

        setTranslatableText_();
    }

    void PortfolioMain::selectRow_(const std::chrono::system_clock::time_point& timePoint) {
        // Select the row with the provided date
        for (int row = 0; row < rowCount(); ++row) {
            QTableWidgetItem* item = this->item(row, 0);
            if(item) {
                TimePointTableWidgetItem* dateItem = dynamic_cast<TimePointTableWidgetItem*>(item);
                if (dateItem && dateItem->getTimePoint() == timePoint) {
                    selectRow(row);
                    break;
                }
            }
        }
    }

    void PortfolioMain::setTranslatableText_() {
        setHorizontalHeaderLabels({tr("date-title"), tr("operation-title"), tr("quantity-title"), tr("asset-title"), tr("fee-title"), tr("description-title")});
        for (int row = 0; row < rowCount(); ++row) {
            setTranslatableText_(row);
        }
    }



    // Implement the PortfolioBalance class
    void PortfolioBalance::retranslateUi() {
        setTranslatableText_();
    }

    void PortfolioBalance::init_() {
        // Create the layout of the sidebar part of the portfolio window
        setColumnCount(4);
        // Set a different color for the header
        horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #f0f0f0 }");
        // Remove the vertical header
        verticalHeader()->setVisible(false);
        // Configure diferent stretch for each column
        horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        // Disable editing
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        // Set the selection behavior to select rows
        setSelectionBehavior(QAbstractItemView::SelectRows);
        // Set the selection mode to single selection
        setSelectionMode(QAbstractItemView::SingleSelection);
        // Set the row height
        verticalHeader()->setDefaultSectionSize(14);
        // Set rows color
        setAlternatingRowColors(true);

        // Context menu
        contextMenu_ = new QMenu(this);
        buyUsingTheHalfAction_ = new QAction(this);
        buyUsingTheHalfAction_->setIcon(QIcon(":/icons/Buy-50-32x32.png"));
        contextMenu_->addAction(buyUsingTheHalfAction_);
        buyUsingTheThirdAction_ = new QAction(this);
        buyUsingTheThirdAction_->setIcon(QIcon(":/icons/Buy-33-32x32.png"));
        contextMenu_->addAction(buyUsingTheThirdAction_);
        buyUsingTheFourthAction_ = new QAction(this);
        buyUsingTheFourthAction_->setIcon(QIcon(":/icons/Buy-25-32x32.png"));
        contextMenu_->addAction(buyUsingTheFourthAction_);
        buyUsingTheFifthAction_ = new QAction(this);
        buyUsingTheFifthAction_->setIcon(QIcon(":/icons/Buy-20-32x32.png"));
        contextMenu_->addAction(buyUsingTheFifthAction_);
        sellTheHalfAction_ = new QAction(this);
        sellTheHalfAction_->setIcon(QIcon(":/icons/Sell-50-32x32.png"));
        contextMenu_->addAction(sellTheHalfAction_);
        sellTheThirdAction_ = new QAction(this);
        sellTheThirdAction_->setIcon(QIcon(":/icons/Sell-33-32x32.png"));
        contextMenu_->addAction(sellTheThirdAction_);
        sellTheFourthAction_ = new QAction(this);
        sellTheFourthAction_->setIcon(QIcon(":/icons/Sell-25-32x32.png"));
        contextMenu_->addAction(sellTheFourthAction_);
        sellTheFifthAction_ = new QAction(this);
        sellTheFifthAction_->setIcon(QIcon(":/icons/Sell-20-32x32.png"));
        contextMenu_->addAction(sellTheFifthAction_);
        contextMenu_->addSeparator();
        buyUsingTheProfitAction_ = new QAction(this);
        buyUsingTheProfitAction_->setIcon(QIcon(":/icons/Copy-Import-32x32.png"));
        contextMenu_->addAction(buyUsingTheProfitAction_);
        sellToLeaveTheProfit = new QAction(this);
        sellToLeaveTheProfit->setIcon(QIcon(":/icons/Copy-Export-32x32.png"));
        contextMenu_->addAction(sellToLeaveTheProfit);
        contextMenu_->addSeparator();
        buyUsingAllAction_ = new QAction(this);
        buyUsingAllAction_->setIcon(QIcon(":/icons/Buy-100-32x32.png"));
        contextMenu_->addAction(buyUsingAllAction_);
        sellAllAction_ = new QAction(this);
        sellAllAction_->setIcon(QIcon(":/icons/Sell-100-32x32.png"));
        contextMenu_->addAction(sellAllAction_);
        setContextMenuPolicy(Qt::CustomContextMenu); // Enable custom context menu policy

        // Connect the right click event to show the context menu
        connect(this, &QTableWidget::customContextMenuRequested, [this](const QPoint& pos) {
            QTableWidgetItem* item = itemAt(pos);
            if(item) {
                // Get the first item of the row
                QTableWidgetItem* item0 = this->item(item->row(), 0);
                if(item0 && item0->data(Qt::UserRole).canConvert<exchange::Ticket>()) {
                    exchange::Ticket asset = item0->data(Qt::UserRole).value<exchange::Ticket>();
                    QTableWidgetItem* item1 = this->item(item->row(), 1);
                    double quantity = item1->data(Qt::UserRole).value<exchange::Balance>().getQuantity();
                    if(quantity > 0) { // TODO: Set a configuration paramenter for the lower quantity to enable orders
                        if(asset == referenceCurrency_) {
                            configureActionsFor_(exchange::OperationType::Buy);
                        } else {
                            configureActionsFor_(exchange::OperationType::Sell);
                        }
                        contextMenu_->exec(viewport()->mapToGlobal(pos));
                    }
                }
            }
        });

        connect(buyUsingTheHalfAction_, &QAction::triggered, [this]() {
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit buyAssetUsing(asset, 50);
        });
        connect(buyUsingTheThirdAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit buyAssetUsing(asset, 33.33333333333333); 
        });
        connect(buyUsingTheFourthAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit buyAssetUsing(asset, 25); 
        });
        connect(buyUsingTheFifthAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit buyAssetUsing(asset, 20); 
        });
        connect(buyUsingTheProfitAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit buyAssetUsingTheProfit(asset); 
        });
        connect(buyUsingAllAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit buyAssetUsing(asset, 100); 
        });
        connect(sellTheHalfAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit sellAssetUsing(asset, 50); 
        });
        connect(sellTheThirdAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit sellAssetUsing(asset, 33.33333333333333); 
        });
        connect(sellTheFourthAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit sellAssetUsing(asset, 25); 
        });
        connect(sellTheFifthAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit sellAssetUsing(asset, 20); 
        });
        connect(sellToLeaveTheProfit, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit sellAssetToLeaveTheProfit(asset); 
        });
        connect(sellAllAction_, &QAction::triggered, [this]() { 
            exchange::Ticket asset = item(currentRow(), 0)->data(Qt::UserRole).value<exchange::Ticket>();
            emit sellAssetUsing(asset, 100); 
        });

        setTranslatableText_();

        // Connections
        // When the row is clicked, emit the signal to show the balance information
        connect(this, &QTableWidget::itemDoubleClicked, [this](QTableWidgetItem* item) {
            if(item) { // Get the second item of the row
                QTableWidgetItem* item1 = this->item(item->row(), 1);
                if(item1 && item1->data(Qt::UserRole).canConvert<exchange::Balance>()) {
                    emit showBalanceInformation(item1->data(Qt::UserRole).value<exchange::Balance>());
                }
            }
        });

    }

    void PortfolioBalance::configureActionsFor_(exchange::OperationType type) {
        buyUsingTheHalfAction_->setVisible(type == exchange::OperationType::Buy);
        buyUsingTheThirdAction_->setVisible(type == exchange::OperationType::Buy);
        buyUsingTheFourthAction_->setVisible(type == exchange::OperationType::Buy);
        buyUsingTheFifthAction_->setVisible(type == exchange::OperationType::Buy);
        buyUsingTheProfitAction_->setVisible(type == exchange::OperationType::Buy);
        buyUsingAllAction_->setVisible(type == exchange::OperationType::Buy);
        sellTheHalfAction_->setVisible(type == exchange::OperationType::Sell);
        sellTheThirdAction_->setVisible(type == exchange::OperationType::Sell);
        sellTheFourthAction_->setVisible(type == exchange::OperationType::Sell);
        sellTheFifthAction_->setVisible(type == exchange::OperationType::Sell);
        sellToLeaveTheProfit->setVisible(type == exchange::OperationType::Sell);
        sellAllAction_->setVisible(type == exchange::OperationType::Sell);
    }

    void PortfolioBalance::setTranslatableText_() {
        setHorizontalHeaderLabels({tr("asset-title"), tr("balance-title"), tr("average-price-title"), tr("worth-title")});
        buyUsingTheHalfAction_->setText(tr("buy-using-the-half-action"));
        buyUsingTheThirdAction_->setText(tr("buy-using-the-third-action"));
        buyUsingTheFourthAction_->setText(tr("buy-using-the-fourth-action"));
        buyUsingTheFifthAction_->setText(tr("buy-using-the-fifth-action"));
        buyUsingTheProfitAction_->setText(tr("buy-using-the-profit-action"));
        buyUsingAllAction_->setText(tr("buy-using-all-action"));
        sellTheHalfAction_->setText(tr("sell-the-half-action"));
        sellTheThirdAction_->setText(tr("sell-the-third-action"));
        sellTheFourthAction_->setText(tr("sell-the-fourth-action"));
        sellTheFifthAction_->setText(tr("sell-the-fifth-action"));
        sellToLeaveTheProfit->setText(tr("sell-to-leave-the-profit-action"));
        sellAllAction_->setText(tr("sell-all-action"));
    }

    void PortfolioBalance::setBalance(unsigned int row, const exchange::Balance& balance, double averagePrice, double price, exchange::Ticket referenceCurrency) {
        QTableWidgetItem* item;
        QFont font;
        // Asset:
        item = new QTableWidgetItem(QString::fromStdString(balance.getAssetName()));
        item->setTextAlignment(Qt::AlignCenter);
        item->setForeground(getColor_(balance.getAsset()));
        font = item->font();
        font.setPointSize(11);
        font.setStretch(QFont::Condensed);
        font.setBold(true);
        item->setFont(font);
        item->setData(Qt::UserRole, QVariant::fromValue(balance.getAsset()));
        setItem(row, 0, item);
        // Balance:
        item = new QTableWidgetItem(MyQtUtils::stringfy(balance.getQuantity(), balance.getAsset(), true));
        item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        item->setForeground(QColor("#393939"));
        font = item->font();
        font.setPointSize(12);
        font.setStretch(QFont::Condensed);
        item->setFont(font);
        item->setData(Qt::UserRole, QVariant::fromValue(balance));
        setItem(row, 1, item);
        if(balance.getAsset() != referenceCurrency) {
            // Average price:
            item = new QTableWidgetItem(MyQtUtils::stringfyPriceHR(averagePrice, balance.getAsset(), referenceCurrency));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            item->setForeground(QColor("#393939"));
            font = item->font();
            font.setPointSize(12);
            font.setStretch(QFont::Condensed);
            item->setFont(font);
            setItem(row, 2, item);
            // Worth:
            item = new QTableWidgetItem(MyQtUtils::stringfy(balance.getQuantity() * price, referenceCurrency, true));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            item->setForeground(QColor("#393939"));
            font = item->font();
            font.setPointSize(12);
            font.setStretch(QFont::Condensed);
            item->setFont(font);
            setItem(row, 3, item);
        }
    }


    // Implement the PortfolioBalanceDetails class
    PortfolioBalanceDetails::PortfolioBalanceDetails(QWidget *parent)
        : QTableWidget(parent)
    {
        // Create the layout of the balance details part of the portfolio window
        init_();
    }

    PortfolioBalanceDetails::~PortfolioBalanceDetails() {}

    void PortfolioBalanceDetails::setBalanceDetails(exchange::Ticket ticket, const std::vector<exchange::BalanceEntry>& entries) {
        // Set the balance details in the table
        setRowCount(static_cast<int>(entries.size()));
        double total = 0;
        for (int row = 0; row < static_cast<int>(entries.size()); ++row) {
            total += entries[row].quantity;
            QString text;
            QString tooltip;
            QTableWidgetItem* item;
            // Date:
            item = new QTableWidgetItem(formatTimePoint(entries[row].date, "%d/%m/%Y"));
            item->setTextAlignment(Qt::AlignCenter);
            tooltip = formatTimePoint(entries[row].date);
            item->setToolTip(tooltip);
            item->setData(Qt::UserRole, QVariant::fromValue(entries[row].id));
            setItem(row, 0, item);
            // Type:
            item = new QTableWidgetItem(QString::fromStdString(exchange::getOperationName(entries[row].type)));
            item->setTextAlignment(Qt::AlignCenter);
            item->setBackground(MyQtUtils::getBackgroundColor(entries[row].type));
            item->setForeground(MyQtUtils::getForegroundColor(entries[row].type));
            item->setToolTip("ID: " + QString::fromStdString(entries[row].id));
            item->setData(Qt::UserRole, QVariant::fromValue(entries[row].type));
            setItem(row, 1, item);
            // Amount:
            text = getText_(entries[row].type, entries[row].quantity, ticket);
            item = new QTableWidgetItem(text);
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            item->setForeground(getForegroundColor_(entries[row].type, entries[row].quantity));
            setItem(row, 2, item);
            // Total:
            item = new QTableWidgetItem(MyQtUtils::stringfy(total, ticket, true));
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            // Set a tooltip with 
            tooltip = "Total: " + MyQtUtils::stringfy(total, ticket, true);
            item->setToolTip(tooltip);
            setItem(row, 3, item);

            setTranslatableText_(row);
        }
    }

    void PortfolioBalanceDetails::retranslateUi() {
        setTranslatableText_();
    }

    void PortfolioBalanceDetails::init_() {
        // Create the layout of the balance details part of the portfolio window
        setColumnCount(4);
        // Set a different color for the header
        horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #f0f0f0 }");
        // Remove the vertical header
        verticalHeader()->setVisible(false);
        // Configure diferent stretch for each column
        horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
        // Disable editing
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        // Set the selection behavior to select rows
        setSelectionBehavior(QAbstractItemView::SelectRows);
        // Set the selection mode to single selection
        setSelectionMode(QAbstractItemView::ContiguousSelection);
        // Set the row height
        verticalHeader()->setDefaultSectionSize(18);
        // Set rows color
        setAlternatingRowColors(false);

        // Context menu
        contextMenu_ = new QMenu(this);
        calculateDetailsForAction_ = new QAction(this);
        calculateDetailsForAction_->setIcon(QIcon(":/icons/Information-32x32.png"));
        contextMenu_->addAction(calculateDetailsForAction_);
        setContextMenuPolicy(Qt::CustomContextMenu); // Enable custom context menu policy

        // Connect the right click event to show the context menu
        connect(this, &QTableWidget::customContextMenuRequested, [this](const QPoint& pos) {
            QTableWidgetItem* item = itemAt(pos);
            if(item) {
                contextMenu_->exec(viewport()->mapToGlobal(pos));
            }
        });

        // oN context menu action triggered send a signal congtgainig the id list of the selected rows
        connect(calculateDetailsForAction_, &QAction::triggered, [this]() {
            std::vector<std::string> ids;
            for (auto item : selectedItems()) {
                if (item->column() == 0) {
                    if (item->data(Qt::UserRole).canConvert<std::string>()) {
                        ids.push_back(item->data(Qt::UserRole).value<std::string>());
                    }
                }
            }
            emit calculateDetailsFor(ids);
        });

        setTranslatableText_();
    }

    QString PortfolioBalanceDetails::getText_(exchange::OperationType type, double quantity, exchange::Ticket ticket) const {
        QString text = MyQtUtils::stringfy(std::abs(quantity), ticket, true);
        switch (type) {
            case exchange::OperationType::Buy:
                return (quantity > 0 ? "+" : "-") + text;
            case exchange::OperationType::Sell:
                return (quantity > 0 ? "+" : "-") + text;
            case exchange::OperationType::Deposit:
                return "+" + text;
            case exchange::OperationType::Withdraw:
                return "-" + text;
            default:
                return text;
        }
    }

    QColor PortfolioBalanceDetails::getForegroundColor_(exchange::OperationType type, double quantity) const {
        switch (type) {
            case exchange::OperationType::Buy:
                return (quantity > 0? Qt::darkGreen : Qt::darkRed);
            case exchange::OperationType::Sell:
                return (quantity > 0? Qt::darkGreen : Qt::darkRed);
            case exchange::OperationType::Deposit:
                return Qt::darkGreen;
            case exchange::OperationType::Withdraw:
                return Qt::darkRed;
            default:
                return Qt::black;
        }
    }

    void PortfolioBalanceDetails::setTranslatableText_() {
        setHorizontalHeaderLabels({tr("date-title"), tr("type-title"), tr("amount-title"), tr("balance-title")});
        for(int row = 0; row < rowCount(); ++row) {
            setTranslatableText_(row);
        }
    }

    void PortfolioBalanceDetails::setTranslatableText_(unsigned int row) {
        exchange::OperationType type = item(row, 1)->data(Qt::UserRole).value<exchange::OperationType>();
        calculateDetailsForAction_->setText(tr("show-details-for-action"));
    }

    void PortfolioBalanceDetails::focusOutEvent(QFocusEvent *event) {
        QTableWidget::focusOutEvent(event);
        //hide();
    }




    // Implement the PortfolioStats class
    void PortfolioStats::updateView(const exchange::StatsOverview& overview) {
        // Update the materialized stats
        if(exchange::fuzzyCompare(overview.materialized.gainLoss, 0.0, 0.001)) {
            gainLossLabel_->setText(tr("gainloss-label"));
            gainLossEdit_->setStyleSheet("");
            worthEdit_->setStyleSheet("");
        } else if(overview.materialized.gainLoss > 0) {
            gainLossLabel_->setText(QString::number(overview.materialized.gainLossPercentage, 'f', 1) + "% " + tr("gain-label"));
            gainLossEdit_->setStyleSheet(GainStyleSheet);
            worthEdit_->setStyleSheet(GainStyleSheet);
        } else { // overview.materialized.gainLoss < 0
            gainLossLabel_->setText(QString::number(overview.materialized.gainLossPercentage, 'f', 1) + "% " + tr("loss-label"));
            gainLossEdit_->setStyleSheet(LossStyleSheet);
            worthEdit_->setStyleSheet(LossStyleSheet);
        }
        costEdit_->setText(MyQtUtils::stringfyHR(overview.materialized.cost, referenceCurrency_, true));
        worthEdit_->setText(MyQtUtils::stringfyHR(overview.materialized.worth, referenceCurrency_, true));
        gainLossEdit_->setText(MyQtUtils::stringfyHR(overview.materialized.gainLoss, referenceCurrency_, true));
        if(overview.materialized.netProfit > 0) {
            netProfitLabel_->setText(tr("profit-label"));
            netProfitEdit_->setText(MyQtUtils::stringfyHR(overview.materialized.netProfit, referenceCurrency_, true));
            taxesLabel_->setText(QString::number(overview.materialized.taxesPercentage, 'f', 1) + "% " + tr("taxes-label"));
            taxesEdit_->setText(MyQtUtils::stringfyHR(overview.materialized.taxes, referenceCurrency_, true));
        }
        else { // overview.materialized.netProfit <= 0
            netProfitLabel_->setText(tr("no-profit-label"));
            netProfitEdit_->setText(MyQtUtils::stringfyHR(0, referenceCurrency_, true));
            taxesLabel_->setText(tr("no-taxes-label"));
            taxesEdit_->setText(MyQtUtils::stringfyHR(0, referenceCurrency_, true));
        }
        volumeEdit_->setText(MyQtUtils::stringfyHR(overview.materialized.volume, referenceCurrency_, true));
        // Update the unmaterialized stats   
        if(exchange::fuzzyCompare(overview.unmaterialized.gainLoss, 0.0, 0.001)) {
            unmaterializedGainLossLabel_->setText(tr("gainloss-label"));
            unmaterializedGainLossEdit_->setStyleSheet("");
            unmaterializedWorthEdit_->setStyleSheet("");
        } else if(overview.unmaterialized.gainLoss > 0) {
            unmaterializedGainLossLabel_->setText(QString::number(overview.unmaterialized.gainLossPercentage, 'f', 1) + "% " + tr("gain-label"));
            unmaterializedGainLossEdit_->setStyleSheet(GainStyleSheet);
            unmaterializedWorthEdit_->setStyleSheet(GainStyleSheet);
        } else { // overview.unmaterialized.gainLoss < 0
            unmaterializedGainLossLabel_->setText(QString::number(overview.unmaterialized.gainLossPercentage, 'f', 1) + "% " + tr("loss-label"));
            unmaterializedGainLossEdit_->setStyleSheet(LossStyleSheet);
            unmaterializedWorthEdit_->setStyleSheet(LossStyleSheet);
        }
        unmaterializedCostEdit_->setText(MyQtUtils::stringfyHR(overview.unmaterialized.cost, referenceCurrency_, true));
        unmaterializedWorthEdit_->setText(MyQtUtils::stringfyHR(overview.unmaterialized.worth, referenceCurrency_, true));
        unmaterializedGainLossEdit_->setText(MyQtUtils::stringfyHR(overview.unmaterialized.gainLoss, referenceCurrency_, true));
        if(overview.unmaterialized.netProfit > 0) {
            unmaterializedNetProfitLabel_->setText(tr("profit-label"));
            unmaterializedNetProfitEdit_->setText(MyQtUtils::stringfyHR(overview.unmaterialized.netProfit, referenceCurrency_, true));
            unmaterializedTaxesLabel_->setText(QString::number(overview.unmaterialized.taxesPercentage, 'f', 1) + "% " + tr("taxes-label"));
            unmaterializedTaxesEdit_->setText(MyQtUtils::stringfyHR(overview.unmaterialized.taxes, referenceCurrency_, true));
        }
        else { // overview.unmaterialized.netProfit <= 0
            unmaterializedNetProfitLabel_->setText(tr("no-profit-label"));
            unmaterializedNetProfitEdit_->setText(MyQtUtils::stringfyHR(0, referenceCurrency_, true));
            unmaterializedTaxesLabel_->setText(tr("no-taxes-label"));
            unmaterializedTaxesEdit_->setText(MyQtUtils::stringfyHR(0, referenceCurrency_, true));
        }
        unmaterializedVolumeEdit_->setText(MyQtUtils::stringfyHR(overview.unmaterialized.volume, referenceCurrency_, true));
    }

    void PortfolioStats::retranslateUi() {
        setTranslatableText_();
    }

    void PortfolioStats::init_() {
        mainLayout_ = new QVBoxLayout(this);
        mainLayout_->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        setLayout(mainLayout_);
        // Create a layout for the private widgets with their labels
        QHBoxLayout* materializedLayout = new QHBoxLayout();
        QHBoxLayout* unmaterializedLayout = new QHBoxLayout();
        mainLayout_->addLayout(materializedLayout);
        mainLayout_->addLayout(unmaterializedLayout);
        // Create the cost layout
        QVBoxLayout* costLayout = new QVBoxLayout();
        costLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        costLayout->setSpacing(0);
        costLabel_ = new QLabel(this);
        costLabel_->setFixedHeight(18);
        costEdit_ = new QLineEdit(this);
        costEdit_->setReadOnly(true);
        costEdit_->setDisabled(true);
        costEdit_->setAlignment(Qt::AlignRight);
        costEdit_->setFixedHeight(18);
        costLayout->addWidget(costLabel_);
        costLayout->addWidget(costEdit_);
        materializedLayout->addLayout(costLayout);
        // Create the worth layout
        QVBoxLayout* worthLayout = new QVBoxLayout();
        worthLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        worthLayout->setSpacing(0);
        worthLabel_ = new QLabel(this);
        worthLabel_->setFixedHeight(18);
        worthEdit_ = new QLineEdit(this);
        worthEdit_->setReadOnly(true);
        worthEdit_->setDisabled(true);
        worthEdit_->setAlignment(Qt::AlignRight);
        worthEdit_->setFixedHeight(18);
        worthLayout->addWidget(worthLabel_);
        worthLayout->addWidget(worthEdit_);
        materializedLayout->addLayout(worthLayout);
        // Create the gain/loss layout
        QVBoxLayout* gainLossLayout = new QVBoxLayout();
        gainLossLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        gainLossLayout->setSpacing(0);
        gainLossLabel_ = new QLabel(this);
        gainLossLabel_->setFixedHeight(18);
        gainLossEdit_ = new QLineEdit(this);
        gainLossEdit_->setReadOnly(true);
        gainLossEdit_->setDisabled(true);
        gainLossEdit_->setAlignment(Qt::AlignRight);
        gainLossEdit_->setFixedHeight(18);
        gainLossLayout->addWidget(gainLossLabel_);
        gainLossLayout->addWidget(gainLossEdit_);
        materializedLayout->addLayout(gainLossLayout);
        // Create the net profit layout
        QVBoxLayout* netProfitLayout = new QVBoxLayout();
        netProfitLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        netProfitLayout->setSpacing(0);
        netProfitLabel_ = new QLabel(this);
        netProfitLabel_->setFixedHeight(18);
        netProfitEdit_ = new QLineEdit(this);
        netProfitEdit_->setReadOnly(true);
        netProfitEdit_->setDisabled(true);
        netProfitEdit_->setAlignment(Qt::AlignRight);
        netProfitEdit_->setFixedHeight(18);
        netProfitLayout->addWidget(netProfitLabel_);
        netProfitLayout->addWidget(netProfitEdit_);
        materializedLayout->addLayout(netProfitLayout);
        // Create the taxes layout
        QVBoxLayout* taxesLayout = new QVBoxLayout();
        taxesLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        taxesLayout->setSpacing(0);
        taxesLabel_ = new QLabel(this);
        taxesLabel_->setFixedHeight(18);
        taxesEdit_ = new QLineEdit(this);
        taxesEdit_->setReadOnly(true);
        taxesEdit_->setDisabled(true);
        taxesEdit_->setAlignment(Qt::AlignRight);
        taxesEdit_->setFixedHeight(18);
        taxesLayout->addWidget(taxesLabel_);
        taxesLayout->addWidget(taxesEdit_);
        materializedLayout->addLayout(taxesLayout);
        // Create the volume layout
        QVBoxLayout* volumeLayout = new QVBoxLayout();
        volumeLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        volumeLayout->setSpacing(0);
        volumeLabel_ = new QLabel(this);
        volumeLabel_->setFixedHeight(18);
        volumeEdit_ = new QLineEdit(this);
        volumeEdit_->setReadOnly(true);
        volumeEdit_->setDisabled(true);
        volumeEdit_->setAlignment(Qt::AlignRight);
        volumeEdit_->setFixedHeight(18);
        volumeLayout->addWidget(volumeLabel_);
        volumeLayout->addWidget(volumeEdit_);
        materializedLayout->addLayout(volumeLayout);
        ////////////////////////////////////////////////////////////////////////////
        // Create the cost layout
        QVBoxLayout* investmentCostLayout = new QVBoxLayout();
        investmentCostLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        investmentCostLayout->setSpacing(0);
        investmentCostLabel_ = new QLabel(this);
        investmentCostLabel_->setFixedHeight(18);
        unmaterializedCostEdit_ = new QLineEdit(this);
        unmaterializedCostEdit_->setReadOnly(true);
        unmaterializedCostEdit_->setDisabled(true);
        unmaterializedCostEdit_->setAlignment(Qt::AlignRight);
        unmaterializedCostEdit_->setFixedHeight(18);
        investmentCostLayout->addWidget(investmentCostLabel_);
        investmentCostLayout->addWidget(unmaterializedCostEdit_);
        unmaterializedLayout->addLayout(investmentCostLayout);
        // Create the worth layout
        QVBoxLayout* unmaterializedWorthLayout = new QVBoxLayout();
        unmaterializedWorthLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        unmaterializedWorthLayout->setSpacing(0);
        unmaterializedWorthLabel_ = new QLabel(this);
        unmaterializedWorthLabel_->setFixedHeight(18);
        unmaterializedWorthEdit_ = new QLineEdit(this);
        unmaterializedWorthEdit_->setReadOnly(true);
        unmaterializedWorthEdit_->setDisabled(true);
        unmaterializedWorthEdit_->setAlignment(Qt::AlignRight);
        unmaterializedWorthEdit_->setFixedHeight(18);
        unmaterializedWorthLayout->addWidget(unmaterializedWorthLabel_);
        unmaterializedWorthLayout->addWidget(unmaterializedWorthEdit_);
        unmaterializedLayout->addLayout(unmaterializedWorthLayout);
        // Create the gain/loss layout
        QVBoxLayout* unmaterializedGainLossLayout = new QVBoxLayout();
        unmaterializedGainLossLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        unmaterializedGainLossLayout->setSpacing(0);
        unmaterializedGainLossLabel_ = new QLabel(this);
        unmaterializedGainLossLabel_->setFixedHeight(18);
        unmaterializedGainLossEdit_ = new QLineEdit(this);
        unmaterializedGainLossEdit_->setReadOnly(true);
        unmaterializedGainLossEdit_->setDisabled(true);
        unmaterializedGainLossEdit_->setAlignment(Qt::AlignRight);
        unmaterializedGainLossEdit_->setFixedHeight(18);
        unmaterializedGainLossLayout->addWidget(unmaterializedGainLossLabel_);
        unmaterializedGainLossLayout->addWidget(unmaterializedGainLossEdit_);
        unmaterializedLayout->addLayout(unmaterializedGainLossLayout);
        // Create the net profit layout
        QVBoxLayout* unmaterializedNetProfitLayout = new QVBoxLayout();
        unmaterializedNetProfitLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        unmaterializedNetProfitLayout->setSpacing(0);
        unmaterializedNetProfitLabel_ = new QLabel(this);
        unmaterializedNetProfitLabel_->setFixedHeight(18);
        unmaterializedNetProfitEdit_ = new QLineEdit(this);
        unmaterializedNetProfitEdit_->setReadOnly(true);
        unmaterializedNetProfitEdit_->setDisabled(true);
        unmaterializedNetProfitEdit_->setAlignment(Qt::AlignRight);
        unmaterializedNetProfitEdit_->setFixedHeight(18);
        unmaterializedNetProfitLayout->addWidget(unmaterializedNetProfitLabel_);
        unmaterializedNetProfitLayout->addWidget(unmaterializedNetProfitEdit_);
        unmaterializedLayout->addLayout(unmaterializedNetProfitLayout);
        // Create the taxes layout
        QVBoxLayout* unmaterializedTaxesLayout = new QVBoxLayout();
        unmaterializedTaxesLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        unmaterializedTaxesLayout->setSpacing(0);
        unmaterializedTaxesLabel_ = new QLabel(this);
        unmaterializedTaxesLabel_->setFixedHeight(18);
        unmaterializedTaxesEdit_ = new QLineEdit(this);
        unmaterializedTaxesEdit_->setReadOnly(true);
        unmaterializedTaxesEdit_->setDisabled(true);
        unmaterializedTaxesEdit_->setAlignment(Qt::AlignRight);
        unmaterializedTaxesEdit_->setFixedHeight(18);
        unmaterializedTaxesLayout->addWidget(unmaterializedTaxesLabel_);
        unmaterializedTaxesLayout->addWidget(unmaterializedTaxesEdit_);
        unmaterializedLayout->addLayout(unmaterializedTaxesLayout);
        // Create the volume layout
        QVBoxLayout* unmaterializedVolumeLayout = new QVBoxLayout();
        unmaterializedVolumeLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        unmaterializedVolumeLayout->setSpacing(0);
        unmaterializedVolumeLabel_ = new QLabel(this);
        unmaterializedVolumeLabel_->setFixedHeight(18);
        unmaterializedVolumeEdit_ = new QLineEdit(this);
        unmaterializedVolumeEdit_->setReadOnly(true);
        unmaterializedVolumeEdit_->setDisabled(true);
        unmaterializedVolumeEdit_->setAlignment(Qt::AlignRight);
        unmaterializedVolumeEdit_->setFixedHeight(18);
        unmaterializedVolumeLayout->addWidget(unmaterializedVolumeLabel_);
        unmaterializedVolumeLayout->addWidget(unmaterializedVolumeEdit_);
        unmaterializedLayout->addLayout(unmaterializedVolumeLayout);

        setTranslatableText_();
    }

    void PortfolioStats::setTranslatableText_() {
        costLabel_->setText(tr("cost-label"));
        costLabel_->setToolTip(tr("cost-tooltip")); // Total cost of the sold assets
        costEdit_->setToolTip(tr("cost-tooltip"));
        worthLabel_->setText(tr("worth-label"));
        worthLabel_->setToolTip(tr("total-worth-tooltip")); // Total worth of the sold assets
        worthEdit_->setToolTip(tr("total-worth-tooltip"));
        gainLossLabel_->setText(tr("gainloss-label"));
        gainLossLabel_->setToolTip(tr("gainloss-tooltip")); // Total gain or loss of the sold assets
        gainLossEdit_->setToolTip(tr("gainloss-tooltip"));
        netProfitLabel_->setText(tr("profit-label"));
        netProfitLabel_->setToolTip(tr("profit-tooltip")); // Total profit of the sold assets
        netProfitEdit_->setToolTip(tr("profit-tooltip"));
        taxesLabel_->setText(tr("taxes-label"));
        taxesLabel_->setToolTip(tr("taxes-tooltip")); // Total taxes to pay for the sold assets
        taxesEdit_->setToolTip(tr("taxes-tooltip"));
        volumeLabel_->setText(tr("volume-label"));
        volumeLabel_->setToolTip(tr("volume-tooltip")); // Total volume related to the sold assets
        volumeEdit_->setToolTip(tr("volume-tooltip"));
        investmentCostLabel_->setText(tr("invested-label"));
        investmentCostLabel_->setToolTip(tr("unmaterialized-cost-tooltip")); // Total cost of the bought assets (unmaterialized)
        unmaterializedCostEdit_->setToolTip(tr("unmaterialized-cost-tooltip"));
        unmaterializedWorthLabel_->setText(tr("worth-label"));
        unmaterializedWorthLabel_->setToolTip(tr("unmaterialized-worth-tooltip")); // Total worth of the bought assets (unmaterialized)
        unmaterializedWorthEdit_->setToolTip(tr("unmaterialized-worth-tooltip"));
        unmaterializedGainLossLabel_->setText(tr("gainloss-label"));
        unmaterializedGainLossLabel_->setToolTip(tr("unmaterialized-gainloss-tooltip")); // Total gain or loss of the bought assets (unmaterialized)
        unmaterializedGainLossEdit_->setToolTip(tr("unmaterialized-gainloss-tooltip"));
        unmaterializedNetProfitLabel_->setText(tr("profit-label"));
        unmaterializedNetProfitLabel_->setToolTip(tr("unmaterialized-profit-tooltip")); // Total profit of the bought assets (unmaterialized)
        unmaterializedNetProfitEdit_->setToolTip(tr("unmaterialized-profit-tooltip"));
        unmaterializedTaxesLabel_->setText(tr("taxes-label"));
        unmaterializedTaxesLabel_->setToolTip(tr("unmaterialized-taxes-tooltip")); // Total taxes to pay for the bought assets (unmaterialized)
        unmaterializedTaxesEdit_->setToolTip(tr("unmaterialized-taxes-tooltip"));
        unmaterializedVolumeLabel_->setText(tr("volume-label"));
        unmaterializedVolumeLabel_->setToolTip(tr("unmaterialized-volume-tooltip")); // Total volume related to the bought assets (unmaterialized)
        unmaterializedVolumeEdit_->setToolTip(tr("unmaterialized-volume-tooltip"));
    }

} // namespace hmi
