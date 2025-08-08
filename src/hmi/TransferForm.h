#ifndef HMI_TRANSFERFORM_H
#define HMI_TRANSFERFORM_H

// Qt headers


// headers

// hmi headers
#include "hmi/TransferWidget.h"


namespace hmi {


class TransferForm : public QWidget {
    Q_OBJECT

public:
    TransferForm(QWidget* parent = nullptr)
        : QWidget(parent) {
        init_();
    }

    inline void updateView(const PortfolioWindow* portfolio) {
        transferWidget_->updateView(portfolio);
    }
    inline void setAsset(exchange::Ticket asset) {
        transferWidget_->setAsset(asset);
    }

    void retranslateUi() {
        transferWidget_->retranslateUi();
        if(transferWidget_->getOperationType() == exchange::OperationType::Deposit) {
            transferButton_->setText(tr("deposit-button-text"));
        } else {
            transferButton_->setText(tr("withdraw-button-text"));
        }
    }

signals:
    void transferConfirmed(const PortfolioWindow* portfolio, exchange::OperationType operationType, const exchange::TransferData& data);

private:
    void init_() {
        // Create the transfer widget
        transferWidget_ = new TransferWidget(false, this); // Not simulate
        // Create the transfer Button
        transferButton_ = new QPushButton(tr("deposit-button-text"));
        transferButton_->setMinimumHeight(30);
        transferButton_->setEnabled(false);
        transferButton_->setFocusPolicy(Qt::NoFocus);
        // Create the transfer form layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(transferWidget_);
        mainLayout->addWidget(transferButton_);
        setLayout(mainLayout);

        connect(transferWidget_, &TransferWidget::operationTypeChanged, [this] (exchange::OperationType operationType) {
            if (operationType == exchange::OperationType::Deposit) {
                transferButton_->setText(tr("deposit-button-text"));
            } else {
                transferButton_->setText(tr("withdraw-button-text"));
            }
        });
        connect(transferWidget_, &TransferWidget::inputValidated, [this] (bool valid) {
            transferButton_->setEnabled(valid);
        });
        connect(transferButton_, &QPushButton::clicked, [this] () {
            if (QMessageBox::question(this, tr("transfer-confirm-title"), tr("transfer-confirm-question")) == QMessageBox::Yes) {
                emit transferConfirmed(transferWidget_->getPortfolio(), transferWidget_->getOperationType(), transferWidget_->getTransferData());
                transferWidget_->resetInputFields();
            }
        });
    }

private:
    TransferWidget* transferWidget_;
    QPushButton* transferButton_;
};

} // namespace hmi

#endif // HMI_TRANSFERFORM_H
