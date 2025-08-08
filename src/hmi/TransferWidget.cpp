// Class header
#include "TransferWidget.h"

// Qt headers
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>

// exchange headers
#include "exchange/Exchange.h"

// hmi headers
#include "hmi/MyQtUtils.h"


namespace hmi {


    // The order widget implementation
    void TransferWidget::updateView(const PortfolioWindow* portfolio) {
        portfolio_ = portfolio;
        if(portfolio_) {
            updateBalanceWidget_();
            updatefeeWidget_();
            setEnabled(!portfolio_->isReadOnly());
        } else {
            quantityLineEdit_->clear();
            balanceLineEdit_->clear();
            feeLineEdit_->clear();
            setEnabled(false);
        }
    }

    void TransferWidget::setAsset(exchange::Ticket asset) {
        asset_ = asset;
        quantityTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(asset)));
        feeTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(asset)));
        assetComboBox_->setCurrentIndex(assetComboBox_->findData(QVariant::fromValue(asset)));
        updateBalanceWidget_();
        updatefeeWidget_();
    }

    void TransferWidget::setOperationType(exchange::OperationType operationType) {
        if(operationType == exchange::OperationType::Deposit) {
            depositButton_->setChecked(true);
        } else {
            withdrawButton_->setChecked(true);
        }
        setOperationType_(operationType);
    }

    exchange::OperationType TransferWidget::getOperationType() const {
        return (depositButton_->isChecked()? exchange::OperationType::Deposit : exchange::OperationType::Withdraw);
    }

    exchange::TransferData TransferWidget::getTransferData() const {
        exchange::TransferData transfer;
        transfer.asset = assetComboBox_->currentData().value<exchange::Ticket>();
        transfer.quantity = quantityLineEdit_->text().toDouble();
        return transfer;
    }

    void TransferWidget::resetInputFields() {
        quantityLineEdit_->clear();
        quantityLineEdit_->setStyleSheet("");
        quantityLineEdit_->setToolTip("");
        slider_->setValue(0);
        updateBalanceWidget_();
        updatefeeWidget_();
    }

    void TransferWidget::resetDialogFields() {
        depositButton_->setChecked(true);
        resetInputFields();
    }

    void TransferWidget::retranslateUi() {
        depositButton_->setText(tr("deposit-selector-text"));
        withdrawButton_->setText(tr("withdraw-selector-text"));
        assetComboBox_->setToolTip(tr("write-first-letters-to-search"));
        quantityLineEdit_->setPlaceholderText(tr("quantity-placeholder-text"));
        quantityLabel_->setText(tr("quantity-label"));
        balanceLabel_->setText(tr("balance-label"));
        feeLabel_->setText(tr("estimated-fee-label"));
        validateInput_();
    }

    // PRIVATE SLOTS
    void TransferWidget::onOperationTypeChanged_(QAbstractButton* abstractButton) {
        //QMessageBox::information(this, "onOperationTypeChanged_()", "the method TransferWidget::onOperationTypeChanged_(QAbstractButton* button) has been called");
        setOperationType_(getOperationType());
    }

    void TransferWidget::onAssetComboBoxChanged_(int index) {
        exchange::Ticket asset = assetComboBox_->currentData().value<exchange::Ticket>();
        setAsset(asset);
        validateInput_();
    }

    void TransferWidget::onQuantityLineEditChanged_(const QString& text) {
        if (updating_) return; // Avoid update loop
        if(portfolio_) {
            updating_ = true;
            bool quantityValid = false;
            double quantity = text.toDouble(&quantityValid);
            if(quantityValid && quantity > 0) {
                double balance = balanceLineEdit_->text().toDouble(); // Balance is always valid
                if(quantity > balance) {
                    slider_->setValue(100);
                } else {
                    double percentage = quantity / balance * 100;
                    slider_->setValue(percentage);
                }
            } else {
                slider_->setValue(0);
            }
            updating_ = false;
        }
    }

    void TransferWidget::onSliderValueChanged_(int value) {
        if (updating_) return; // Avoid update loop
        if(portfolio_) {
            updating_ = true;
            double balance = balanceLineEdit_->text().toDouble(); // Balance is always valid
            double quantity = balance * value / 100;
            if(quantity > 0) {
                quantityLineEdit_->setText(MyQtUtils::stringfy(quantity, asset_));
            }
            else {
                quantityLineEdit_->clear();
            }
            updating_ = false;
        }
    }

    void TransferWidget::onBalanceLineEditChanged_(const QString& text) {
        // Get the necesary input values
        double balance = text.toDouble(); // Balance is always valid
        bool quantityValid = false;
        double quantity = quantityLineEdit_->text().toDouble(&quantityValid);
        if(quantityValid) {
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

    void TransferWidget::validateInput_() {
        // The validity flags for the input fields
        bool quantityValid = false;
        // Get the input values
        double quantity = quantityLineEdit_->text().toDouble(&quantityValid);
        double balance = balanceLineEdit_->text().toDouble(); // Balance is always valid
        // The quantity must be a positive number to be valid
        bool quantityIsNegative = quantity < 0;
        // Check if the balance is sufficient
        bool insufficientBalance = (balance < quantity);
        // Set the style sheet to the quantity field
        if(!quantityValid && !quantityLineEdit_->text().isEmpty()) {
            quantityLineEdit_->setStyleSheet(WarningStyleSheet);
            quantityLineEdit_->setToolTip(tr("invalid-quantity-tooltip")); // Invalid quantity: you must enter a valid number
        } else if (quantityIsNegative) {
            quantityLineEdit_->setStyleSheet(WarningStyleSheet);
            quantityLineEdit_->setToolTip(tr("invalid-negative-quantity-tooltip")); // Invalid quantity: the quantity must be a positive number
        } else if (insufficientBalance) {
            quantityLineEdit_->setStyleSheet(DangerStyleSheet);
            if(getOperationType() == exchange::OperationType::Deposit) {
                quantityLineEdit_->setToolTip(tr("insufficient-deposit-balance-tooltip")); // Reduce the quantity: the quantity must be lower or equal to your balance
            } else {
                quantityLineEdit_->setToolTip(tr("insufficient-withdraw-balance-tooltip")); // Insufficient balance: the quantity must be lower or equal to your balance
            }
        } else {
            quantityLineEdit_->setStyleSheet("");
            quantityLineEdit_->setToolTip("");
        }
        // Enable the order button if the input is valid
        bool valid = quantityValid && !insufficientBalance && !quantityIsNegative;
        emit inputValidated(valid);
    }

    void TransferWidget::init_() {
        // Ensure only one widget can have focus at a time
        setFocusPolicy(Qt::StrongFocus);
        // Deposit/Withdraw selector
        depositButton_ = new QPushButton(tr("deposit-selector-text"), this);
        depositButton_->setCheckable(true);
        depositButton_->setMinimumHeight(30);
        depositButton_->setFixedWidth(77);
        depositButton_->setContentsMargins(0, 0, 0, 0);
        depositButton_->setFocusPolicy(Qt::NoFocus);
        withdrawButton_ = new QPushButton(tr("withdraw-selector-text"), this);
        withdrawButton_->setCheckable(true);
        withdrawButton_->setMinimumHeight(30);
        withdrawButton_->setFixedWidth(77);
        withdrawButton_->setContentsMargins(0, 0, 0, 0);
        withdrawButton_->setFocusPolicy(Qt::NoFocus);
        buttonGroup_ = new QButtonGroup(this);
        buttonGroup_->addButton(depositButton_, 0);
        buttonGroup_->addButton(withdrawButton_, 1);
        buttonGroup_->setExclusive(true);
        // Layout
        QHBoxLayout* selectorLayout = new QHBoxLayout();
        selectorLayout->addWidget(depositButton_);
        selectorLayout->addWidget(withdrawButton_);
        selectorLayout->setSpacing(0);
        selectorLayout->setContentsMargins(0, 0, 0, 0);

        // Asset
        assetComboBox_ = new QComboBox(this);
        for(auto it : exchange::AvailableTickets) {
            if(it.first != exchange::Ticket::Unknown) {
                assetComboBox_->addItem(exchange::getTicketName(it.first).c_str(), QVariant::fromValue(it.first));
            }
        }
        assetComboBox_->setMinimumHeight(30);
        assetComboBox_->setFixedWidth(126);
        assetComboBox_->setCurrentIndex(0);
        // Layout
        QHBoxLayout* assetLayout = new QHBoxLayout();
        assetLayout->setSpacing(0);
        assetLayout->setContentsMargins(0, 0, 0, 0);
        assetLayout->addWidget(assetComboBox_);
        assetLayout->addStretch();
        
        // Quantity
        quantityLineEdit_ = new QLineEdit();
        quantityLineEdit_->setAlignment(Qt::AlignRight);
        quantityLineEdit_->setPlaceholderText(tr("quantity-placeholder-text"));
        quantityLineEdit_->setMinimumHeight(24);
        quantityLineEdit_->setFixedWidth(126);
        quantityLineEdit_->setClearButtonEnabled(true);
        QFont font = quantityLineEdit_->font();
        font.setPointSize(12);
        font.setStretch(QFont::Condensed);
        quantityLineEdit_->setFont(font);
        quantityLabel_ = new QLabel(tr("quantity-label"));
        quantityLabel_->setMinimumHeight(24);
        quantityLabel_->setFixedWidth(140);
        quantityLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        quantityTickerLabel_ = MyQtUtils::createTickerLabel(exchange::Ticket::BTC, 24);
        // Layout
        QHBoxLayout* quantityLayout = new QHBoxLayout();
        quantityLayout->addWidget(quantityLineEdit_);
        quantityLayout->addWidget(quantityTickerLabel_);

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

        // Layouts
        QVBoxLayout* selectorLabelLayout = new QVBoxLayout();
        selectorLabelLayout->addLayout(selectorLayout);
        selectorLabelLayout->addWidget(quantityLabel_);

        QVBoxLayout* assetQuantityLayout = new QVBoxLayout();
        assetQuantityLayout->addLayout(assetLayout);
        assetQuantityLayout->addLayout(quantityLayout);
        
        QHBoxLayout* balanceLayout = new QHBoxLayout();
        balanceLayout->addWidget(balanceLabel_);
        balanceLayout->addWidget(balanceLineEdit_);
        balanceLayout->addWidget(balanceTickerLabel_);

        QHBoxLayout* feeLayout = new QHBoxLayout();
        feeLayout->addWidget(feeLabel_);
        feeLayout->addWidget(feeLineEdit_);
        feeLayout->addWidget(feeTickerLabel_);

        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->addLayout(selectorLabelLayout);
        topLayout->addLayout(assetQuantityLayout);

        QVBoxLayout* mainLayout = new QVBoxLayout();
        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(slider_);
        mainLayout->addLayout(balanceLayout);
        mainLayout->addLayout(feeLayout);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        
        // Set the layout to the main widget
        setLayout(mainLayout);

        connect(buttonGroup_, &QButtonGroup::buttonClicked, this, &TransferWidget::onOperationTypeChanged_);
        connect(assetComboBox_, &QComboBox::currentIndexChanged, this, &TransferWidget::onAssetComboBoxChanged_);
        connect(quantityLineEdit_, &QLineEdit::textChanged, this, &TransferWidget::onQuantityLineEditChanged_);
        connect(quantityLineEdit_, &QLineEdit::textChanged, this, &TransferWidget::validateInput_);
        connect(quantityLineEdit_, &QLineEdit::returnPressed, [this] () {
            quantityLineEdit_->setText(balanceLineEdit_->text());
        });
        connect(slider_, &QSlider::valueChanged, this, &TransferWidget::onSliderValueChanged_);
        connect(balanceLineEdit_, &QLineEdit::textChanged, this, &TransferWidget::onBalanceLineEditChanged_);
        
        // Set the default operation type
        depositButton_->setChecked(true);

        // Set the default operation type
        updateBalanceWidget_();
        updatefeeWidget_();
        // Execute the initial validation
        validateInput_();
    }

    void TransferWidget::setOperationType_(exchange::OperationType operationType) {
        //QMessageBox::information(this, "setOperationType_()", "the method TransferWidget::setOperationType_() has been called");
        resetInputFields();
        updateBalanceWidget_();
        emit operationTypeChanged(operationType);
    }

    void TransferWidget::updateBalanceWidget_() {
        if(portfolio_) {
            const auto& generalBalance = portfolio_->getGeneralBalance();
            if(getOperationType() == exchange::OperationType::Deposit) {
                balanceLineEdit_->setText(MyQtUtils::stringfy(generalBalance.getWalletQuantity(asset_), asset_));
            } else {
                balanceLineEdit_->setText(MyQtUtils::stringfy(generalBalance.getBalanceQuantity(asset_), asset_));
            }
            balanceTickerLabel_->setText(QString::fromStdString(exchange::getTicketName(asset_)));
        }
    }

    void TransferWidget::updatefeeWidget_() {
        if(portfolio_) {
            float multiplier = simulated_? portfolio_->getMultiplier() : 1.0;
            float increment = simulated_? portfolio_->getIncrement(asset_) : 0.0;
            if(getOperationType() == exchange::OperationType::Deposit) {
                exchange::Fee fee = exchange::calculateDepositFee(asset_, portfolio_->getReferenceCurrency(), portfolio_->getTickersInformation(), multiplier, increment);
                feeLineEdit_->setText(MyQtUtils::stringfy(fee.quantity, asset_));
            } else {
                exchange::Fee fee = exchange::calculateWithdrawFee(asset_, portfolio_->getReferenceCurrency(), portfolio_->getTickersInformation(), multiplier, increment);
                feeLineEdit_->setText(MyQtUtils::stringfy(fee.quantity, asset_));
            }
        }
    }


} // namespace hmi
