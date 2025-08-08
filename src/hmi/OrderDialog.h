#ifndef HMI_ORDERDIALOG_H
#define HMI_ORDERDIALOG_H

// Qt headers
#include <QtWidgets/QDialog>

// headers

// exchange headers

// hmi headers
#include "hmi/OrderWidget.h"


namespace hmi
{

class OrderDialog : public QDialog {
    Q_OBJECT

public:
    OrderDialog(QWidget* parent = nullptr)
        : QDialog(parent) {
        init_();
    }

    void updateView(const PortfolioWindow* portfolio) {
        orderWidget_->updateView(portfolio);
    }
    void setMarket(exchange::Market market) {
        orderWidget_->setMarket(market);
    }

    void showForBuy(double total) {
        showDialog(exchange::OperationType::Buy);
        orderWidget_->setForBuy(total);
    }
    void showForSell(double quantity) {
        showDialog(exchange::OperationType::Sell);
        orderWidget_->setForSell(quantity);
    }

    void showDialog(exchange::OperationType operationType = exchange::OperationType::Buy) {
        resetInputFields_();
        orderWidget_->setOperationType(operationType);
        orderWidget_->updateView();
        QDialog::show();
    }

    void retranslateUi() {
        orderWidget_->retranslateUi();
        dateLabel_->setText(tr("date-label"));
        useCurrentDateCheckBox_->setText(tr("use-current-date-checkbox"));
        if(orderWidget_->getOperationType() == exchange::OperationType::Buy) {
            orderButton_->setText(tr("buy-button-text"));
        } else {
            orderButton_->setText(tr("sell-button-text"));
        }
    }

signals:
    void orderSimulated(const PortfolioWindow* portfolio, exchange::OperationType operationType, const exchange::DummyOrder& data);

private slots:

private:
    void init_() {
        // Create the order widget
        orderWidget_ = new OrderWidget(true, this); // Simulate
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
        // Create the order Button
        orderButton_ = new QPushButton(tr("buy-button-text"));
        orderButton_->setMinimumHeight(30);
        orderButton_->setEnabled(false);
        orderButton_->setFocusPolicy(Qt::NoFocus);
        // Create the order form layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setSizeConstraint(QLayout::SetFixedSize);
        //mainLayout->setContentsMargins(0, 0, 0, 0); // Set margins to 0
        mainLayout->addWidget(orderWidget_);
        mainLayout->addWidget(useCurrentDateCheckBox_);
        mainLayout->addLayout(dateLayout);
        mainLayout->addWidget(orderButton_);
        setLayout(mainLayout);
        adjustSize();
        centerDialog_();

        connect(orderWidget_, &OrderWidget::operationTypeChanged, [this] (exchange::OperationType operationType) {
            if (operationType == exchange::OperationType::Buy) {
                orderButton_->setText(tr("buy-button-text"));
            } else {
                orderButton_->setText(tr("sell-button-text"));
            }
        });
        connect(useCurrentDateCheckBox_, &QCheckBox::toggled, [this] (bool checked) {
            setDateVisible_(!checked);
            setFixedSize(sizeHint());
            //validateInput();
        });
        connect(orderWidget_, &OrderWidget::inputValidated, [this] (bool valid) {
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
            orderButton_->setEnabled(valid && dateValid);
        });
        connect(orderButton_, &QPushButton::clicked, [this] () {
            if (QMessageBox::question(this, tr("order-confirm-title"), tr("order-confirm-question")) == QMessageBox::Yes) {
                const PortfolioWindow* portfolio = orderWidget_->getPortfolio();
                if(portfolio) {
                    exchange::DummyOrder order;
                    order.data = orderWidget_->getOrderData();
                    order.autoDate = useCurrentDateCheckBox_->isChecked();
                    order.multiplier = portfolio->getMultiplier();
                    order.increment = portfolio->getIncrement(order.data.getAsset());
                    if(order.autoDate) { // Set the current date
                        order.timePoint = QDateTime::currentDateTime().toSecsSinceEpoch();
                    } else { // Set the date written by the user
                        order.timePoint = dateEdit_->dateTime().toSecsSinceEpoch();
                    }
                    accept();
                    emit orderSimulated(portfolio, orderWidget_->getOperationType(), order);
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
    OrderWidget* orderWidget_;
    QLabel* dateLabel_;
    QCheckBox* useCurrentDateCheckBox_;
    QDateTimeEdit* dateEdit_;
    QPushButton* orderButton_;
};

} // namespace hmi

#endif // HMI_ORDERDIALOG_H
