// header
#include "ProxiesManagerDialog.h"

// Qt headers
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>



namespace hmi {

    void ProxiesManager::executeProxy(exchange::Market market, unsigned int period) {
        market_ = market;
        period_ = period;
        createButton_->setEnabled(false);
        emit createProxy(service_, market_, period_);
    }

    void ProxiesManager::activateProxy(exchange::Market market, unsigned int period) {
        if(market_ == market && period_ == period) {
            createButton_->setEnabled(true);
            // Remove the martket from the market combobox
            for(int i = 0; i < marketComboBox_->count(); ++i) {
                if(marketComboBox_->itemData(i).value<exchange::Market>() == market) {
                    setMarketComboBoxItemVisibility_(market, false);
                    break;
                }
            }
        }
    }

    void ProxiesManager::deactivateProxy(exchange::Market market) {
        for(int row = 0; row < activeProxies_->rowCount(); ++row) {
            if(activeProxies_->item(row, 1)->data(Qt::UserRole).value<exchange::Market>() == market) {
                activeProxies_->removeRow(row);
                setMarketComboBoxItemVisibility_(market, true);
                break;
            }
        }
    }

QString ProxiesManager::getStatusLabel_(exchange::ProxyStatus status) {
        switch(status) {
            case exchange::ProxyStatus::Running:
                return tr("proxy-status-running");
                break;
            case exchange::ProxyStatus::Paused:
                return tr("proxy-status-paused");
                break;
            case exchange::ProxyStatus::SlowedDown:
                return tr("proxy-status-slowed-down");
                break;
            case exchange::ProxyStatus::SpeededUp:
                return tr("proxy-status-speeded-up");
                break;
            default:
                return tr("unknow-status");
        }
    }

    QIcon ProxiesManager::getStatusIcon_(exchange::ProxyStatus status) {
        switch(status) {
            case exchange::ProxyStatus::Running:
                return QIcon(":/icons/Right-Arrow-32x32.png");
                break;
            case exchange::ProxyStatus::SlowedDown:
                return QIcon(":/icons/Red-Down-Arrow-32x32.png");
                break;
            case exchange::ProxyStatus::SpeededUp:
                return QIcon(":/icons/Green-Up-Arrow-32x32.png");
                break;
            case exchange::ProxyStatus::Paused:
                return QIcon(":/icons/Paused-32x32.png");
                break;
            default:
                return QIcon();
        }
    }

    QString ProxiesManager::getResponseLabel_(exchange::Response response) {
        switch(response) {
            case exchange::Response::Ok:
                return tr("response-ok");
                break;
            case exchange::Response::Warning:
                return tr("response-warning");
                break;
            case exchange::Response::Error:
                return tr("response-error");
                break;
            case exchange::Response::Stop:
                return tr("response-stop");
                break;
            case exchange::Response::Banned:
                return tr("response-banned");
                break;
            default:
                return tr("unknow-response");
        }
    }

    QIcon ProxiesManager::getResponseIcon_(exchange::Response response) {
        switch(response) {
            case exchange::Response::Ok:
                return QIcon(":/icons/Tick-32x32.png");
                break;
            case exchange::Response::Warning:
                return QIcon(":/icons/Warning-32x32.png");
                break;
            case exchange::Response::Error:
                return QIcon(":/icons/Error-32x32.png");
                break;
            case exchange::Response::Stop:
                return QIcon(":/icons/Stop-32x32.png");
                break;
            case exchange::Response::Banned:
                return QIcon(":/icons/Banned-32x32.png");
                break;
            default:
                return QIcon();
        }
    }

    void ProxiesManager::updateProxyStatus(exchange::Market market, exchange::ProxyStatus status) {
        int row = findOrCreateRow_(market);
        QTableWidgetItem* item = activeProxies_->item(row, 2);
        if(item) {
            item->setText(getStatusLabel_(status));
            item->setIcon(getStatusIcon_(status));
            item->setData(Qt::UserRole, QVariant::fromValue(status));
        }
    }

    void ProxiesManager::updateProxyStatistics(exchange::Market market, exchange::WorkStats workStats) {
        int row = findOrCreateRow_(market);
        QTableWidgetItem* item;
        item = activeProxies_->item(row, 3);
        if(item) {
            item->setText(getResponseLabel_(workStats.response));
            item->setIcon(getResponseIcon_(workStats.response));
            item->setData(Qt::UserRole, QVariant::fromValue(workStats.response));
        }
        item = activeProxies_->item(row, 4);
        if(item) {
            item->setText(QString::number(workStats.cycles));
        }
        item = activeProxies_->item(row, 5);
        if(item) {
            item->setText(QString::number(workStats.frequency, 'f', 2) + " Hz");
        }
    }

    void ProxiesManager::init_() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Name field
        QHBoxLayout *topLayout = new QHBoxLayout();

        // Market combobox
        marketLabel_ = new QLabel(this);
        marketLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        //marketLabel_->setFixedWidth(50);
        marketComboBox_ = new QComboBox(this);
        const auto it = exchange::AvailablePairs.find(service_);
        if(it != exchange::AvailablePairs.end()) {
            for(const auto& pair : it->second) {
                marketComboBox_->addItem(exchange::getMarketName(pair.first).c_str(), QVariant::fromValue(pair.first));
            }
        }
        marketComboBox_->setCurrentIndex(0);
        topLayout->addWidget(marketLabel_);
        topLayout->addWidget(marketComboBox_);
        topLayout->addSpacing(10);
        // Period spinbox
        periodLabel_ = new QLabel(this);
        periodLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        //periodLabel_->setFixedWidth(50);
        periodComboBox_ = new QComboBox(this);
        periodComboBox_->addItem("1s", QVariant::fromValue(1));
        periodComboBox_->addItem("2s", QVariant::fromValue(2));
        periodComboBox_->addItem("3s", QVariant::fromValue(3));
        periodComboBox_->addItem("4s", QVariant::fromValue(4));
        periodComboBox_->addItem("5s", QVariant::fromValue(5));
        periodComboBox_->addItem("10s", QVariant::fromValue(10));
        periodComboBox_->addItem("15s", QVariant::fromValue(15));
        periodComboBox_->addItem("20s", QVariant::fromValue(20));
        periodComboBox_->addItem("30s", QVariant::fromValue(30));
        periodComboBox_->addItem("1m", QVariant::fromValue(60));
        periodComboBox_->addItem("2m", QVariant::fromValue(120));
        periodComboBox_->addItem("5m", QVariant::fromValue(300));
        periodComboBox_->addItem("10m", QVariant::fromValue(600));
        periodComboBox_->addItem("15m", QVariant::fromValue(900));
        periodComboBox_->addItem("20m", QVariant::fromValue(1200));
        periodComboBox_->addItem("30m", QVariant::fromValue(1800));
        periodComboBox_->addItem("1h", QVariant::fromValue(3600));
        periodComboBox_->addItem("2h", QVariant::fromValue(7200));
        periodComboBox_->addItem("4h", QVariant::fromValue(14400));
        periodComboBox_->addItem("6h", QVariant::fromValue(21600));
        periodComboBox_->setCurrentIndex(0);
        topLayout->addWidget(periodLabel_);
        topLayout->addWidget(periodComboBox_);
        topLayout->addSpacing(10);
        // Create button
        createButton_ = new QPushButton(this);
        createButton_->setIcon(QIcon(":/icons/Power-On-32x32.png"));
        topLayout->addWidget(createButton_);
        topLayout->addStretch();

        connect(createButton_, &QPushButton::clicked, [this]() {
            auto market = marketComboBox_->currentData().value<exchange::Market>();
            auto period = periodComboBox_->currentData().value<int>() * 1000;
            executeProxy(market, period);
        });
        
        // Active proxies table
        activeProxies_ = new QTableWidget(this);
        activeProxies_->setColumnCount(6);
        activeProxies_->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #f0f0f0 }");
        activeProxies_->verticalHeader()->setVisible(false);
        activeProxies_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        activeProxies_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        activeProxies_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        activeProxies_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
        activeProxies_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
        activeProxies_->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
        activeProxies_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        activeProxies_->setSelectionBehavior(QAbstractItemView::SelectRows);
        activeProxies_->setSelectionMode(QAbstractItemView::SingleSelection);
        activeProxies_->verticalHeader()->setDefaultSectionSize(14);
        activeProxies_->setAlternatingRowColors(false);
        activeProxies_->setContextMenuPolicy(Qt::CustomContextMenu);

        // Context menu
        contextMenu_ = new QMenu(this);
        slowdownProxyAction_ = new QAction(this);
        slowdownProxyAction_->setIcon(QIcon(":/icons/Controls-Advance-Frame-32x32.png"));
        contextMenu_->addAction(slowdownProxyAction_);
        pauseProxyAction_ = new QAction(this);
        pauseProxyAction_->setIcon(QIcon(":/icons/Controls-Pause-32x32.png"));
        contextMenu_->addAction(pauseProxyAction_);
        resumeProxyAction_ = new QAction(this);
        resumeProxyAction_->setIcon(QIcon(":/icons/Controls-Play-32x32.png"));
        contextMenu_->addAction(resumeProxyAction_);
        destroyProxyAction_ = new QAction(this);
        destroyProxyAction_->setIcon(QIcon(":/icons/Power-Off-32x32.png"));
        contextMenu_->addAction(destroyProxyAction_);
        

        // Connect the right click event to show the context menu
        connect(activeProxies_, &QTableWidget::customContextMenuRequested, [this](const QPoint& pos) {
            QTableWidgetItem* item = activeProxies_->itemAt(pos);
            if(item) {
                item = item = activeProxies_->item(item->row(), 2);
                if(item) {
                    exchange::ProxyStatus status = item->data(Qt::UserRole).value<exchange::ProxyStatus>();
                    if(status == exchange::ProxyStatus::Running) {
                        slowdownProxyAction_->setEnabled(true);
                        pauseProxyAction_->setEnabled(true);
                        resumeProxyAction_->setEnabled(false);
                    } else if(status == exchange::ProxyStatus::Paused) {
                        slowdownProxyAction_->setEnabled(false);
                        pauseProxyAction_->setEnabled(false);
                        resumeProxyAction_->setEnabled(true);
                    } else { // SlowedDown or SpeededUp
                        slowdownProxyAction_->setEnabled(true);
                        pauseProxyAction_->setEnabled(true);
                        resumeProxyAction_->setEnabled(true);
                    }
                }
                contextMenu_->exec(activeProxies_->viewport()->mapToGlobal(pos));
            }
        });

        connect(slowdownProxyAction_, &QAction::triggered, [this]() {
            int row = activeProxies_->currentRow();
            if(row >= 0) {
                exchange::Market market = activeProxies_->item(row, 1)->data(Qt::UserRole).value<exchange::Market>();
                auto period = periodComboBox_->currentData().value<int>() * 1000;
                emit slowdownProxy(service_, market, period);
            }
        });
        connect(pauseProxyAction_, &QAction::triggered, [this]() {
            int row = activeProxies_->currentRow();
            if(row >= 0) {
                exchange::Market market = activeProxies_->item(row, 1)->data(Qt::UserRole).value<exchange::Market>();
                emit pauseProxy(service_, market);
            }
        });
        connect(resumeProxyAction_, &QAction::triggered, [this]() {
            int row = activeProxies_->currentRow();
            if(row >= 0) {
                exchange::Market market = activeProxies_->item(row, 1)->data(Qt::UserRole).value<exchange::Market>();
                emit resumeProxy(service_, market);
            }
        });
        connect(destroyProxyAction_, &QAction::triggered, [this]() {
            int row = activeProxies_->currentRow();
            if(row >= 0) {
                exchange::Market market = activeProxies_->item(row, 1)->data(Qt::UserRole).value<exchange::Market>();
                emit destroyProxy(service_, market);
            }
        });
        
        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(activeProxies_);
        setLayout(mainLayout);

        setTranslatableText_();
    }

    int ProxiesManager::findOrCreateRow_(exchange::Market market) {
        for(int row = 0; row < activeProxies_->rowCount(); ++row) {
            if(activeProxies_->item(row, 1)->data(Qt::UserRole).value<exchange::Market>() == market) {
                return row;
            }
        }
        int row = activeProxies_->rowCount();
        activeProxies_->insertRow(row);
        QTableWidgetItem* item;
        item = new QTableWidgetItem();
        item->setText(exchange::getServiceName(service_).c_str());
        item->setData(Qt::UserRole, QVariant::fromValue(service_));
        activeProxies_->setItem(row, 0, item);
        item = new QTableWidgetItem();
        item->setText(exchange::getMarketName(market).c_str());
        item->setData(Qt::UserRole, QVariant::fromValue(market));
        activeProxies_->setItem(row, 1, item);
        activeProxies_->setItem(row, 2, new QTableWidgetItem());
        activeProxies_->setItem(row, 3, new QTableWidgetItem());
        activeProxies_->setItem(row, 4, new QTableWidgetItem());
        activeProxies_->setItem(row, 5, new QTableWidgetItem());
        return row;
    }

    void ProxiesManager::resetInputFields_() {
        marketComboBox_->setCurrentIndex(0);
        periodComboBox_->setCurrentIndex(0);
    }

    void ProxiesManager::setTranslatableText_() {
        // Market combobox
        marketLabel_->setText(tr("market-label"));
        marketLabel_->setToolTip(tr("market-label-tooltip"));
        marketComboBox_->setToolTip(tr("write-first-letters-to-search"));
        // Period combobox
        periodLabel_->setText(tr("period-label"));
        periodLabel_->setToolTip(tr("period-label-tooltip"));
        periodComboBox_->setToolTip(tr("write-first-letters-to-search"));
        // Create button
        createButton_->setText(tr("create-proxy-button"));
        // Actions
        slowdownProxyAction_->setText(tr("slowdown-proxy-action"));
        pauseProxyAction_->setText(tr("pause-proxy-action"));
        resumeProxyAction_->setText(tr("resume-proxy-action"));
        destroyProxyAction_->setText(tr("stop-proxy-action"));
        // Active proxies table
        activeProxies_->setHorizontalHeaderLabels({tr("service-title"), tr("market-title"), tr("proxy-status-title"), tr("last-response-title"), tr("execution-cycles-title"), tr("frequency-title")});
        for(int row = 0; row < activeProxies_->rowCount(); ++row) {
            QTableWidgetItem* item = activeProxies_->item(row, 2);
            if(item) {
                item->setText(getStatusLabel_(item->data(Qt::UserRole).value<exchange::ProxyStatus>()));
            }
            item = activeProxies_->item(row, 3);
            if(item) {
                item->setText(getResponseLabel_(item->data(Qt::UserRole).value<exchange::Response>()));
            }  
        }
    }

    void ProxiesManager::setMarketComboBoxItemVisibility_(exchange::Market market, bool visible) {
        if(visible) {
            marketComboBox_->addItem(exchange::getMarketName(market).c_str(), QVariant::fromValue(market));
        }
        else {
            int index = marketComboBox_->findData(QVariant::fromValue(market));
            if(index >= 0) {
                marketComboBox_->removeItem(index);
            }
        }
    }


/////////////////////////////////////////////////////////////////////////////////////////

    void ProxiesManagerDialog::executeProxy(exchange::Service service, exchange::Market market, unsigned int period) {
        ProxiesManager* proxiesManager = getProxiesManager_(service);
        if(proxiesManager) {
            proxiesManager->executeProxy(market, period);
        }
    }

    void ProxiesManagerDialog::activateProxy(exchange::Service service, exchange::Market market, unsigned int period) {
        ProxiesManager* proxiesManager = getProxiesManager_(service);
        if(proxiesManager) {
            proxiesManager->activateProxy(market, period);
        }
    }

    void ProxiesManagerDialog::deactivateProxy(exchange::Service service, exchange::Market market) {
         ProxiesManager* proxiesManager = getProxiesManager_(service);
        if(proxiesManager) {
            proxiesManager->deactivateProxy(market);
        }
    }

    void ProxiesManagerDialog::updateProxyStatus(exchange::Service service, exchange::Market market, exchange::ProxyStatus status) {
         ProxiesManager* proxiesManager = getProxiesManager_(service);
        if(proxiesManager) {
            proxiesManager->updateProxyStatus(market, status);
        }
    }

    void ProxiesManagerDialog::updateProxyStatistics(exchange::Service service, exchange::Market market, exchange::WorkStats workStats) {
         ProxiesManager* proxiesManager = getProxiesManager_(service);
        if(proxiesManager) {
            proxiesManager->updateProxyStatistics(market, workStats);
        }
    }

    void ProxiesManagerDialog::init_() {
        // Tab widget
        tabWidget_ = new QTabWidget(this);
        // Create the service proxies widget
        for(const auto& pair : exchange::AvailableServices) {
            exchange::Service service = pair.first;
            if(service != exchange::Service::Unknown) {
                ProxiesManager* proxiesManager = new ProxiesManager(service, this);
                connect(proxiesManager, &ProxiesManager::createProxy, [this](exchange::Service service, exchange::Market market, unsigned int period) {
                    emit createProxy(service, type_, market, period);
                });
                connect(proxiesManager, &ProxiesManager::slowdownProxy, [this](exchange::Service service, exchange::Market market, unsigned int period) {
                    emit slowdownProxy(service, type_, market, period);
                });
                connect(proxiesManager, &ProxiesManager::pauseProxy, [this](exchange::Service service, exchange::Market market) {
                    emit pauseProxy(service, type_, market);
                });
                connect(proxiesManager, &ProxiesManager::resumeProxy, [this](exchange::Service service, exchange::Market market) {
                    emit resumeProxy(service, type_, market);
                });
                connect(proxiesManager, &ProxiesManager::destroyProxy, [this](exchange::Service service, exchange::Market market) {
                    emit destroyProxy(service, type_, market);
                });
                tabWidget_->addTab(proxiesManager, exchange::getServiceName(proxiesManager->getServiceType()).c_str());
            }
        }
        // Create the main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->addWidget(tabWidget_);
        setLayout(mainLayout);

        setTranslatableText_();
    }

    void ProxiesManagerDialog::resetInputFields_() {
    }

    void ProxiesManagerDialog::setTranslatableText_() {
    }

    ProxiesManager* ProxiesManagerDialog::getProxiesManager_(exchange::Service service) {
        for(int i = 0; i < tabWidget_->count(); ++i) {
            ProxiesManager* proxiesManager = dynamic_cast<ProxiesManager*>(tabWidget_->widget(i));
            if(proxiesManager && proxiesManager->getServiceType() == service) {
                return proxiesManager;
            }
        }
        return nullptr;
    }

} // namespace hmi
