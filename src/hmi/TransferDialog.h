#ifndef HMI_TRANSFERDIALOG_H
#define HMI_TRANSFERDIALOG_H



// Qt headers
#include <QtWidgets/QDialog>

// hmi headers
#include "hmi/TransferWidget.h"


namespace hmi {


class TransferDialog : public QDialog {
    Q_OBJECT

public:
    TransferDialog(QWidget* parent = nullptr)
        : QDialog(parent) {
        init_();
    }

    void updateView(const PortfolioWindow* portfolio) {
        transferWidget_->updateView(portfolio);
    }
    void setAsset(exchange::Ticket asset) {
        transferWidget_->setAsset(asset);
    }

    void showDialog(exchange::OperationType operationType = exchange::OperationType::Deposit) {
        resetInputFields_();
        transferWidget_->setOperationType(operationType);
        transferWidget_->updateView();
        QDialog::show();
    }

    void retranslateUi() {
        transferWidget_->retranslateUi();
        dateLabel_->setText(tr("date-label"));
        useCurrentDateCheckBox_->setText(tr("use-current-date-checkbox"));
        if(transferWidget_->getOperationType() == exchange::OperationType::Deposit) {
            transferButton_->setText(tr("deposit-button-text"));
        } else {
            transferButton_->setText(tr("withdraw-button-text"));
        }
    }

signals:
    void transferSimulated(const PortfolioWindow* portfolio, exchange::OperationType operationType, const exchange::DummyTransfer& data);

private slots:

private:
    void init_() {
        // Create the transfer widget
        transferWidget_ = new TransferWidget(true, this); // Simulate
        // Set the checkbox to select if the date will be the current date or a custom date
        useCurrentDateCheckBox_ = new QCheckBox(tr("use-current-date-checkbox"), this);
        useCurrentDateCheckBox_->setChecked(true);
        // Create the date layout
        QHBoxLayout *dateLayout = new QHBoxLayout();
        dateLabel_ = new QLabel(tr("date-label"), this);
        dateEdit_ = new QDateTimeEdit(QDateTime::currentDateTime(), this);
        dateEdit_->setDisplayFormat("dd/MM/yyyy HH:mm");
        dateEdit_->setCalendarPopup(true);
        setDateVisible_(!useCurrentDateCheckBox_->isChecked());
        dateLayout->addWidget(dateLabel_);
        dateLayout->addWidget(dateEdit_);
        // Create the transfer Button
        transferButton_ = new QPushButton(tr("deposit-button-text"));
        transferButton_->setMinimumHeight(30);
        transferButton_->setEnabled(false);
        transferButton_->setFocusPolicy(Qt::NoFocus);
        // Create the transfer form layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setSizeConstraint(QLayout::SetFixedSize);
        //mainLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        mainLayout->addWidget(transferWidget_);
        mainLayout->addWidget(useCurrentDateCheckBox_);
        mainLayout->addLayout(dateLayout);
        mainLayout->addWidget(transferButton_);
        setLayout(mainLayout);
        adjustSize();
        centerDialog_();

        connect(transferWidget_, &TransferWidget::operationTypeChanged, [this] (exchange::OperationType operationType) {
            if (operationType == exchange::OperationType::Deposit) {
                transferButton_->setText(tr("deposit-button-text"));
            } else {
                transferButton_->setText(tr("withdraw-button-text"));
            }
        });
        connect(useCurrentDateCheckBox_, &QCheckBox::toggled, [this] (bool checked) {
            setDateVisible_(!checked);
            setFixedSize(sizeHint());
            //validateInput();
        });
        connect(transferWidget_, &TransferWidget::inputValidated, [this] (bool valid) {
            // Set the style sheet to the date field
            bool dateValid = false;
            if(useCurrentDateCheckBox_->isChecked()) {
                dateValid = true;
                useCurrentDateCheckBox_->setStyleSheet("");
                useCurrentDateCheckBox_->setToolTip("");
            } else {
                dateValid = dateEdit_->dateTime().isValid();
                dateEdit_->setStyleSheet(dateValid? "" : WarningStyleSheet);
                dateEdit_->setToolTip(dateValid? "" : tr("invalid-date-tooltip"));
            }
            transferButton_->setEnabled(valid && dateValid);
        });
        connect(transferButton_, &QPushButton::clicked, [this] () {
            if (QMessageBox::question(this, tr("transfer-confirm-title"), tr("transfer-confirm-question")) == QMessageBox::Yes) {
                const PortfolioWindow* portfolio = transferWidget_->getPortfolio();
                if(portfolio) {
                    exchange::DummyTransfer transfer;
                    transfer.data = transferWidget_->getTransferData();
                    transfer.multiplier = portfolio->getMultiplier();
                    transfer.increment = portfolio->getIncrement(transfer.data.asset);
                    transfer.autoDate = useCurrentDateCheckBox_->isChecked();
                    if(transfer.autoDate) { // Set the current date
                        transfer.timePoint = QDateTime::currentDateTime().toSecsSinceEpoch();
                    } else { // Set the date written by the user
                        transfer.timePoint = dateEdit_->dateTime().toSecsSinceEpoch();
                    }
                    accept();
                    emit transferSimulated(portfolio, transferWidget_->getOperationType(), transfer);
                }
            }
        });
    }

    void setDateVisible_(bool visible) {
        dateLabel_->setVisible(visible);
        dateEdit_->setVisible(visible);
        if(visible) {
            dateEdit_->setDateTime(QDateTime::currentDateTime());
        }
    }

    void resetInputFields_() {
        useCurrentDateCheckBox_->setChecked(true);
        dateEdit_->setDateTime(QDateTime::currentDateTime());
    }

    void centerDialog_() {
        QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }

    // Override the close event: after the dialog is closed by user, ask for confirmation
    void closeEvent(QCloseEvent* event) override {
        if (QMessageBox::question(this, tr("close-confirm-title"), tr("close-confirm-question")) == QMessageBox::Yes) {
            event->accept();
        } else {
            event->ignore();
        }
    }

private:
    TransferWidget* transferWidget_;
    QLabel* dateLabel_;
    QCheckBox* useCurrentDateCheckBox_;
    QDateTimeEdit* dateEdit_;
    QPushButton* transferButton_;
};


} // namespace hmi

#endif // HMI_TRANSFERDIALOG_H
