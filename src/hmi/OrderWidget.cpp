// Class header
#include "OrderWidget.h"

// Qt headers

// hmi headers
#include "hmi/MyQtUtils.h"


namespace hmi {

    
    // The order form implementation
    void OrderWidget::updateView(const PortfolioWindow* portfolio) {
        portfolio_ = portfolio;
        if(portfolio_) {
            updateBalanceWidget_();
            setEnabled(!portfolio_->isReadOnly());
        } else {
            quantityLineEdit_->clear();
            priceLineEdit_->clear();
            totalLineEdit_->clear();
            balanceLineEdit_->clear();
            feeLineEdit_->clear();
            setEnabled(false);
        }
    }

    void OrderWidget::setMarket(exchange::Market market) {
        market_ = market;
        asset_ = exchange::getAssetType(market);
        currency_ = exchange::getCurrencyType(market);
        priceTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(currency_)));
        quantityTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(asset_)));
        totalTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(currency_)));
        feeTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(currency_)));
        updateBalanceWidget_();
        if(isEnabled()) {
            setLastAssetPrice_(true);
        }
    }

    void OrderWidget::setOperationType(exchange::OperationType operationType) {
        if(operationType == exchange::OperationType::Buy) {
            buyButton_->setChecked(true);
        } else {
            sellButton_->setChecked(true);
        }
        setOperationType_(operationType);
    }

    exchange::OperationType OrderWidget::getOperationType() const {
        return (buyButton_->isChecked()? exchange::OperationType::Buy : exchange::OperationType::Sell);
    }

    exchange::OrderData OrderWidget::getOrderData() const {
        exchange::OrderData order;
        order.market = market_;
        order.price = priceLineEdit_->text().toDouble();
        order.quantity = quantityLineEdit_->text().toDouble();
        order.total = totalLineEdit_->text().toDouble();
        return order;
    }

    void OrderWidget::resetInputFields() {
        quantityLineEdit_->clear();
        quantityLineEdit_->setStyleSheet("");
        quantityLineEdit_->setToolTip("");
        totalLineEdit_->clear();
        totalLineEdit_->setStyleSheet("");
        totalLineEdit_->setToolTip("");
        feeLineEdit_->clear();
        balanceLineEdit_->clear();
        slider_->setValue(0);
    }

    void OrderWidget::retranslateUi() {
        buyButton_->setText(tr("buy-selector-text"));
        sellButton_->setText(tr("sell-selector-text"));
        priceLineEdit_->setPlaceholderText(tr("price-placeholder-text"));
        quantityLineEdit_->setPlaceholderText(tr("quantity-placeholder-text"));
        totalLineEdit_->setPlaceholderText(tr("total-placeholder-text"));
        balanceLabel_->setText(tr("balance-label"));
        feeLabel_->setText(tr("estimated-fee-label"));
        validateInput_();
    }

    // PRIVATE SLOTS
    void OrderWidget::onOperationTypeChanged_(QAbstractButton* button) {
        //QMessageBox::information(this, "onOperationTypeChanged_()", "the method OrderWidget::onOperationTypeChanged_(QAbstractButton* button) has been called");
        setOperationType_(getOperationType());
    }

    void OrderWidget::onPriceLineEditChanged_(const QString& text) {
        if(portfolio_) {
            updating_ = true;
            exchange::OperationType operationType = getOperationType();
            const auto& generalBalance = portfolio_->getGeneralBalance();
            bool priceValid = false;
            double price = text.toDouble(&priceValid);
            if(priceValid && price > 0) {
                
                if(operationType == exchange::OperationType::Buy) {
                    bool totalValid = false;
                    double total = totalLineEdit_->text().toDouble(&totalValid);
                    if(totalValid && total > 0) {
                        double quantity = total * 0.9975 / price;
                        quantityLineEdit_->setText(MyQtUtils::stringfy(quantity, asset_));
                    }
                } else { // exchange::OperationType::Sell
                    bool quantityValid = false;
                    double quantity = quantityLineEdit_->text().toDouble(&quantityValid);
                    if(quantityValid && quantity > 0) {
                        double total = quantity * price * 0.9975;
                        totalLineEdit_->setText(MyQtUtils::stringfy(total, currency_));
                        double fee = (total / 0.9975) - total;
                        feeLineEdit_->setText(MyQtUtils::stringfy(fee, currency_));
                    }
                }
            } else {
                if(operationType == exchange::OperationType::Buy) {
                    quantityLineEdit_->clear();
                } else { // exchange::OperationType::Sell
                    totalLineEdit_->clear();
                    feeLineEdit_->clear();
                }
            }
            updating_ = false;
        }
    }

    void OrderWidget::onPriceLineEditReturnPressed_() {
        setLastAssetPrice_(false);
    }

    void OrderWidget::onQuantityLineEditChanged_(const QString& text) {
        if (updating_) return; // Avoid update loop
        if(portfolio_) {
            updating_ = true;
            exchange::OperationType operationType = getOperationType();
            const auto& generalBalance = portfolio_->getGeneralBalance();
            bool priceValid = false;
            bool quantityValid = false;
            double price = priceLineEdit_->text().toDouble(&priceValid);
            double quantity = text.toDouble(&quantityValid);
            if(quantityValid && quantity > 0) {
                if(operationType == exchange::OperationType::Buy) {
                    if(priceValid && price > 0) {
                        double total = quantity * price / 0.9975;
                        double percentage = total / generalBalance.getBalance(currency_).getQuantity() * 100;
                        double fee = total * 0.0025;
                        totalLineEdit_->setText(MyQtUtils::stringfy(total, currency_));
                        slider_->setValue(percentage);
                        feeLineEdit_->setText(MyQtUtils::stringfy(fee, currency_));
                    } else {
                        totalLineEdit_->clear();
                        slider_->setValue(0);
                        feeLineEdit_->clear();
                    }
                }
                else { // exchange::OperationType::Sell
                    if(priceValid && price > 0) {
                        double total = quantity * price * 0.9975;
                        double fee = (total / 0.9975) - total;
                        totalLineEdit_->setText(MyQtUtils::stringfy(total, currency_));
                        feeLineEdit_->setText(MyQtUtils::stringfy(fee, currency_));
                    } else {
                        totalLineEdit_->clear();
                        feeLineEdit_->clear();
                    }
                    double percentage = quantity / generalBalance.getBalance(asset_).getQuantity() * 100;
                    slider_->setValue(percentage);
                }  
            }
            else { // The quantity is not valid
                if(operationType == exchange::OperationType::Buy) {
                    feeLineEdit_->clear();
                } else if(operationType == exchange::OperationType::Sell) {
                    slider_->setValue(0);
                }
            }
            updating_ = false;
        }
    }

    void OrderWidget::onTotalLineEditChanged_(const QString& text) {
        if (updating_) return; // Avoid update loop
        if(portfolio_) {
            updating_ = true;
            exchange::OperationType operationType = getOperationType();
            const auto& generalBalance = portfolio_->getGeneralBalance();
            bool priceValid = false;
            bool totalValid = false;
            double price = priceLineEdit_->text().toDouble(&priceValid);
            double total = text.toDouble(&totalValid);
            if(totalValid && total > 0) {
                double fee;
                if(operationType == exchange::OperationType::Buy) {
                    fee = total * 0.0025;
                    if(priceValid && price > 0) {
                        double quantity = (total - fee) / price;
                        quantityLineEdit_->setText(MyQtUtils::stringfy(quantity, asset_));
                    }
                    else {
                        quantityLineEdit_->clear();
                    }
                    double percentage = total / generalBalance.getBalance(currency_).getQuantity() * 100;
                    slider_->setValue(percentage);
                } else { // exchange::OperationType::Sell
                    fee = (total / 0.9975) - total;
                    if(priceValid && price > 0) {
                        double quantity = (total + fee) / price;
                        double percentage = quantity / generalBalance.getBalance(asset_).getQuantity() * 100;
                        quantityLineEdit_->setText(MyQtUtils::stringfy(quantity, asset_));
                        slider_->setValue(percentage);
                    }
                    else {
                        quantityLineEdit_->clear();
                        slider_->setValue(0);
                    }
                }
                feeLineEdit_->setText(MyQtUtils::stringfy(fee, currency_));
            }
            else { // The total is not valid
                if(operationType == exchange::OperationType::Buy) {
                    slider_->setValue(0);
                } else if(operationType == exchange::OperationType::Sell) {
                    feeLineEdit_->clear();
                }
            }
            updating_ = false;
        }
    }

    void OrderWidget::onSliderValueChanged_(int value) {
        if (updating_) return; // Avoid update loop
        if(portfolio_) {
            updating_ = true;
            exchange::OperationType operationType = getOperationType();
            const auto& generalBalance = portfolio_->getGeneralBalance();
            bool priceValid = false;
            double price = priceLineEdit_->text().toDouble(&priceValid);
            if(operationType == exchange::OperationType::Buy) {
                double balance = generalBalance.getBalance(currency_).getQuantity();
                double total = balance * value / 100;
                if(total > 0) {
                    totalLineEdit_->setText(MyQtUtils::stringfy(total, currency_));
                    double fee = (total / 0.9975) - total;
                    feeLineEdit_->setText(MyQtUtils::stringfy(fee, currency_));
                    if(priceValid && price > 0) {
                        double quantity = (total * 0.9975) / price;
                        quantityLineEdit_->setText(MyQtUtils::stringfy(quantity, asset_));
                    }
                }
                else {
                    totalLineEdit_->clear();
                    feeLineEdit_->clear();
                    quantityLineEdit_->clear();
                }
            } else { // exchange::OperationType::Sell
                double balance = generalBalance.getBalance(asset_).getQuantity();
                double quantity = balance * value / 100;
                if(quantity > 0) {
                    quantityLineEdit_->setText(MyQtUtils::stringfy(quantity, asset_));
                    if(priceValid && price > 0) {
                        double total = quantity * price * 0.9975;
                        totalLineEdit_->setText(MyQtUtils::stringfy(total, currency_));
                        double fee = (total / 0.9975) - total;
                        feeLineEdit_->setText(MyQtUtils::stringfy(fee, currency_));
                    }
                }
                else {
                    quantityLineEdit_->clear();
                    totalLineEdit_->clear();
                    feeLineEdit_->clear();
                }
            }
            updating_ = false;
        }
    }

    void OrderWidget::onBalanceLineEditChanged_(const QString& text) {
        // Get the necesary input values
        double balance = text.toDouble();
        exchange::OperationType operationType = getOperationType();
        if(operationType == exchange::OperationType::Buy) {
            // Calculate the total and set the value to the total field and the slider
            bool totalValid = false;
            double total = totalLineEdit_->text().toDouble(&totalValid);
            if(totalValid && !totalLineEdit_->text().isEmpty()) {
                if(total > balance) { // The total cannot be greater than the balance
                    totalLineEdit_->setText(MyQtUtils::stringfy(balance, currency_));
                    total = balance;
                }
                double percentage = total / balance * 100;
                slider_->setValue(percentage);
                double fee = (total / 0.9975) - total;
                feeLineEdit_->setText(MyQtUtils::stringfy(fee, currency_));
            } else {
                slider_->setValue(0);
                feeLineEdit_->clear();
            }
        } else { // exchange::OperationType::Sell
            // Calculate the quantity and set the value to the quantity field and the slider
            bool quantityValid = false;
            double quantity = quantityLineEdit_->text().toDouble(&quantityValid);
            if(quantityValid && !quantityLineEdit_->text().isEmpty()) {
                if(quantity > balance) { // The quantity cannot be greater than the balance
                    quantityLineEdit_->setText(MyQtUtils::stringfy(balance, asset_));
                    quantity = balance;
                }
                double percentage = quantity / balance * 100;
                slider_->setValue(percentage);
            } else {
                slider_->setValue(0);
            }
        }
    }

    void OrderWidget::validateInput_() {
        // The validity flags for the input fields
        bool priceValid = false;
        bool quantityValid = false;
        bool totalValid = false;
        bool insufficientBuyBalance = false;
        bool insufficientSellBalance = false;
        // Get the input values
        double price = priceLineEdit_->text().toDouble(&priceValid);
        double quantity = quantityLineEdit_->text().toDouble(&quantityValid);
        double total = totalLineEdit_->text().toDouble(&totalValid);
        double balance = balanceLineEdit_->text().toDouble(); // Balance is always valid
        // The validation depends on the operation type
        exchange::OperationType operationType = getOperationType();
        if(operationType == exchange::OperationType::Buy) {
            insufficientBuyBalance = totalValid? (balance < total) : false;
        } else { // exchange::OperationType::Sell
            insufficientSellBalance = quantityValid? (balance < quantity) : false;
        }
        // Set the style sheet to the price field
        if(!priceValid && !priceLineEdit_->text().isEmpty()) {
            priceLineEdit_->setStyleSheet(WarningStyleSheet);
            priceLineEdit_->setToolTip(tr("invalid-price-tooltip")); // Invalid price: you must enter a valid number
        } else if(priceValid && price <= 0) {
            priceLineEdit_->setStyleSheet(WarningStyleSheet);
            priceLineEdit_->setToolTip(tr("negative-value-tooltip")); // Invalid value: you must enter a positive number greater than zero
        } else {
            priceLineEdit_->setStyleSheet("");
            priceLineEdit_->setToolTip("");
        }
        // Set the style sheet to the quantity field
        if(!quantityValid && !quantityLineEdit_->text().isEmpty()) {
            quantityLineEdit_->setStyleSheet(WarningStyleSheet);
            quantityLineEdit_->setToolTip(tr("invalid-quantity-tooltip")); // Invalid quantity: you must enter a valid number
        } else if(quantityValid && quantity <= 0) {
            quantityLineEdit_->setStyleSheet(WarningStyleSheet);
            quantityLineEdit_->setToolTip(tr("negative-value-tooltip")); // Invalid value: you must enter a positive number greater than zero
        } else if (insufficientSellBalance) {
            quantityLineEdit_->setStyleSheet(DangerStyleSheet);
            if(operationType == exchange::OperationType::Buy) {
                quantityLineEdit_->setToolTip(tr("insufficient-buy-balance-tooltip")); // Reduce the quantity: the total must be lower or equal to your balance
            } else {
                quantityLineEdit_->setToolTip(tr("insufficient-sell-balance-tooltip")); // Insufficient balance: the quantity must be lower or equal to your balance
            }
        } else {
            quantityLineEdit_->setStyleSheet("");
            quantityLineEdit_->setToolTip("");
        }
        // Set the style sheet to the total field
        if(!totalValid && !totalLineEdit_->text().isEmpty()) {
            totalLineEdit_->setStyleSheet(WarningStyleSheet);
            totalLineEdit_->setToolTip(tr("invalid-total-tooltip")); // Invalid total: you must enter a valid number
        } else if(totalValid && total <= 0) {
            totalLineEdit_->setStyleSheet(WarningStyleSheet);
            totalLineEdit_->setToolTip(tr("negative-value-tooltip")); // Invalid value: you must enter a positive number greater than zero
        } else if (insufficientBuyBalance) {
            totalLineEdit_->setStyleSheet(DangerStyleSheet);
            if(operationType == exchange::OperationType::Buy) {
                totalLineEdit_->setToolTip(tr("insufficient-buy-balance-tooltip"));
            } else {
                totalLineEdit_->setToolTip(tr("insufficient-sell-balance-tooltip"));
            }
        } else {
            totalLineEdit_->setStyleSheet("");
            totalLineEdit_->setToolTip("");
        }
        // Enable the order button if the input is valid
        bool valid = quantityValid && priceValid && totalValid && !insufficientBuyBalance && !insufficientSellBalance;
        emit inputValidated(valid);
    }


    void OrderWidget::init_() {
        QFont font;
        // Buy/Sell selector
        buyButton_ = new QPushButton(tr("buy-selector-text"));
        buyButton_->setCheckable(true);
        buyButton_->setMinimumHeight(30);
        buyButton_->setFixedWidth(77);
        buyButton_->setContentsMargins(0, 0, 0, 0);
        buyButton_->setFocusPolicy(Qt::NoFocus);
        sellButton_ = new QPushButton(tr("sell-selector-text"));
        sellButton_->setCheckable(true);
        sellButton_->setMinimumHeight(30);
        sellButton_->setFixedWidth(77);
        sellButton_->setContentsMargins(0, 0, 0, 0);
        sellButton_->setFocusPolicy(Qt::NoFocus);
        buttonGroup_ = new QButtonGroup(this);
        buttonGroup_->addButton(buyButton_, 0);
        buttonGroup_->addButton(sellButton_, 1);
        buttonGroup_->setExclusive(true);
        // Price
        priceLineEdit_ = new QLineEdit();
        priceLineEdit_->setAlignment(Qt::AlignRight);
        priceLineEdit_->setPlaceholderText(tr("price-placeholder-text"));
        priceLineEdit_->setFixedHeight(30);
        priceLineEdit_->setFixedWidth(126);
        priceLineEdit_->setClearButtonEnabled(true);
        font = priceLineEdit_->font();
        font.setPointSize(12);
        font.setStretch(QFont::Condensed);
        priceLineEdit_->setFont(font);
        priceTickerLabel_ = MyQtUtils::createTickerLabel(exchange::Ticket::EUR, 24);
        // Quantity
        quantityLineEdit_ = new QLineEdit();
        quantityLineEdit_->setAlignment(Qt::AlignRight);
        quantityLineEdit_->setPlaceholderText(tr("quantity-placeholder-text"));
        quantityLineEdit_->setFixedHeight(24);
        quantityLineEdit_->setFixedWidth(126);
        quantityLineEdit_->setClearButtonEnabled(true);
        font = quantityLineEdit_->font();
        font.setPointSize(12);
        font.setStretch(QFont::Condensed);
        quantityLineEdit_->setFont(font);
        quantityTickerLabel_ = MyQtUtils::createTickerLabel(exchange::Ticket::BTC, 24);
        // Total
        totalLineEdit_ = new QLineEdit();
        totalLineEdit_->setAlignment(Qt::AlignRight);
        totalLineEdit_->setPlaceholderText(tr("total-placeholder-text"));
        totalLineEdit_->setMinimumHeight(24);
        totalLineEdit_->setFixedWidth(126);
        totalLineEdit_->setClearButtonEnabled(true);
        font = totalLineEdit_->font();
        font.setPointSize(12);
        font.setStretch(QFont::Condensed);
        totalLineEdit_->setFont(font);
        totalTickerLabel_ = MyQtUtils::createTickerLabel(exchange::Ticket::EUR, 24);
        // Slider
        slider_ = new QSlider(Qt::Horizontal);
        slider_->setRange(0, 100);
        // Balance
        balanceLineEdit_ = new QLineEdit();
        balanceLineEdit_->setAlignment(Qt::AlignRight);
        balanceLineEdit_->setReadOnly(true);
        balanceLineEdit_->setFixedWidth(160);
        balanceLineEdit_->setEnabled(false);
        balanceLabel_ = new QLabel(tr("balance-label"));
        balanceTickerLabel_ = MyQtUtils::createTickerLabel(exchange::Ticket::EUR, 24);
        // Estimated Fee
        feeLineEdit_ = new QLineEdit();
        feeLineEdit_->setAlignment(Qt::AlignRight);
        feeLineEdit_->setReadOnly(true);
        feeLineEdit_->setFixedWidth(160);
        feeLineEdit_->setEnabled(false);
        feeLabel_ = new QLabel(tr("estimated-fee-label"));
        feeTickerLabel_ = MyQtUtils::createTickerLabel(exchange::Ticket::EUR, 24);

        // Layout
        QHBoxLayout* selectorLayout = new QHBoxLayout();
        selectorLayout->addWidget(buyButton_);
        selectorLayout->addWidget(sellButton_);
        // Remove the space, the padding and the margin between the buttons: The buy and the sell buttons must be joined
        selectorLayout->setSpacing(0);
        selectorLayout->setContentsMargins(0, 0, 0, 0);
        

        QHBoxLayout* priceLayout = new QHBoxLayout();
        priceLayout->addWidget(priceLineEdit_);
        priceLayout->addWidget(priceTickerLabel_);

        QHBoxLayout* quantityLayout = new QHBoxLayout();
        quantityLayout->addWidget(quantityLineEdit_);
        quantityLayout->addWidget(quantityTickerLabel_);

        QHBoxLayout* totalLayout = new QHBoxLayout();
        totalLayout->addWidget(totalLineEdit_);
        totalLayout->addWidget(totalTickerLabel_);

        QVBoxLayout* selectorQuantityLayout = new QVBoxLayout();
        selectorQuantityLayout->addLayout(selectorLayout);
        selectorQuantityLayout->addLayout(quantityLayout);

        QVBoxLayout* priceTotalLayout = new QVBoxLayout();
        priceTotalLayout->addLayout(priceLayout);
        priceTotalLayout->addLayout(totalLayout);

        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->addLayout(selectorQuantityLayout);
        topLayout->addLayout(priceTotalLayout);
        
        QHBoxLayout* balanceLayout = new QHBoxLayout();
        balanceLayout->addWidget(balanceLabel_);
        balanceLayout->addWidget(balanceLineEdit_);
        balanceLayout->addWidget(balanceTickerLabel_);

        QHBoxLayout* feeLayout = new QHBoxLayout();
        feeLayout->addWidget(feeLabel_);
        feeLayout->addWidget(feeLineEdit_);
        feeLayout->addWidget(feeTickerLabel_);

        QVBoxLayout* mainLayout = new QVBoxLayout();
        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(slider_);
        mainLayout->addLayout(balanceLayout);
        mainLayout->addLayout(feeLayout);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        
        // Set the layout to the main widget
        setLayout(mainLayout);

        connect(buttonGroup_, &QButtonGroup::buttonClicked, this, &OrderWidget::onOperationTypeChanged_);
        connect(priceLineEdit_, &QLineEdit::textChanged, this, &OrderWidget::onPriceLineEditChanged_);
        connect(priceLineEdit_, &QLineEdit::returnPressed, this, &OrderWidget::onPriceLineEditReturnPressed_);
        connect(quantityLineEdit_, &QLineEdit::textChanged, this, &OrderWidget::onQuantityLineEditChanged_);
        connect(quantityLineEdit_, &QLineEdit::returnPressed, [this] () {
            if(getOperationType() == exchange::OperationType::Sell) {
                quantityLineEdit_->setText(balanceLineEdit_->text());
            }
        });
        connect(totalLineEdit_, &QLineEdit::textChanged, this, &OrderWidget::onTotalLineEditChanged_);
        connect(totalLineEdit_, &QLineEdit::returnPressed, [this] () {
            if(getOperationType() == exchange::OperationType::Buy) {
                totalLineEdit_->setText(balanceLineEdit_->text());
            }
        });
        connect(slider_, &QSlider::valueChanged, this, &OrderWidget::onSliderValueChanged_);
        connect(balanceLineEdit_, &QLineEdit::textChanged, this, &OrderWidget::onBalanceLineEditChanged_);
        connect(quantityLineEdit_, &QLineEdit::textChanged, this, &OrderWidget::validateInput_);
        connect(priceLineEdit_, &QLineEdit::textChanged, this, &OrderWidget::validateInput_);
        connect(totalLineEdit_, &QLineEdit::textChanged, this, &OrderWidget::validateInput_);
    }

    void OrderWidget::setOperationType_(exchange::OperationType operationType) {
        //QMessageBox::information(this, "setOperationType_()", "the method OrderWidget::setOperationType_() has been called");
        resetInputFields();
        updateBalanceWidget_();
        setLastAssetPrice_(true);
        emit operationTypeChanged(operationType);
    }

    void OrderWidget::setLastAssetPrice_(bool updating) {
        updating_ = updating;
        try {
            if(portfolio_) {
                double price = simulated_? portfolio_->getSimulatedLastPrice() : portfolio_->getLastPrice();
                priceLineEdit_->setText(MyQtUtils::stringfyPrice(price, asset_, currency_));
            }
        } catch (const std::out_of_range&) {
            priceLineEdit_->clear();
        }
        updating_ = false;
    }

    void OrderWidget::updateBalanceWidget_() {
        if(portfolio_) {
            const auto& generalBalance = portfolio_->getGeneralBalance();
            if(getOperationType() == exchange::OperationType::Buy) {
                try {
                    balanceLineEdit_->setText(MyQtUtils::stringfy(generalBalance.getBalance(currency_).getQuantity(), currency_));
                } catch (const std::out_of_range&) {
                    balanceLineEdit_->clear();
                }
                balanceTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(currency_)));
            } else { // exchange::OperationType::Sell
                try {
                    balanceLineEdit_->setText(MyQtUtils::stringfy(generalBalance.getBalance(asset_).getQuantity(), asset_));
                } catch (const std::out_of_range&) {
                    balanceLineEdit_->clear();
                }
                balanceTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(asset_)));
            }
        }
    }

    void OrderWidget::updatefeeWidget_() {
        if(portfolio_) {
            float multiplier = simulated_? portfolio_->getMultiplier() : 1.0;
            float increment = simulated_? portfolio_->getIncrement(asset_) : 0.0;
            if(getOperationType() == exchange::OperationType::Buy) {
                double total = totalLineEdit_->text().toDouble();
                exchange::Fee fee = exchange::calculateBuyFee(market_, total, portfolio_->getReferenceCurrency(), portfolio_->getTickersInformation(), multiplier, increment);
                feeLineEdit_->setText(MyQtUtils::stringfy(fee.quantity, currency_));
            } else { // exchange::OperationType::Sell
                double quantity = quantityLineEdit_->text().toDouble();
                exchange::Fee fee = exchange::calculateSellFee(market_, quantity, portfolio_->getReferenceCurrency(), portfolio_->getTickersInformation(), multiplier, increment);
                feeLineEdit_->setText(MyQtUtils::stringfy(fee.quantity, asset_));
            }
        }
    }

} // namespace hmi

