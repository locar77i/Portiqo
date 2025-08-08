#ifndef HMI_ORDERFORM_H
#define HMI_ORDERFORM_H

// Qt headers
#include <QtWidgets/QWidget>

// headers

// exchange headers

// hmi headers
#include "hmi/OrderWidget.h"


namespace hmi
{

class OrderForm : public QWidget {
    Q_OBJECT

public:
    OrderForm(QWidget* parent = nullptr)
        : QWidget(parent) {
        init_();
    }

    inline void updateView(const PortfolioWindow* portfolio) {
        orderWidget_->updateView(portfolio);
    }

    inline void setMarket(exchange::Market market) { 
        orderWidget_->setMarket(market);
    }

    void setForBuy(double total) {
        orderWidget_->setOperationType(exchange::OperationType::Buy);
        orderWidget_->updateView();
        orderWidget_->setForBuy(total);
    }
    void setForSell(double quantity) {
        orderWidget_->setOperationType(exchange::OperationType::Sell);
        orderWidget_->updateView();
        orderWidget_->setForSell(quantity);
    }

    void retranslateUi() {
        orderWidget_->retranslateUi();
        if(orderWidget_->getOperationType() == exchange::OperationType::Buy) {
            orderButton_->setText(tr("buy-button-text"));
        } else {
            orderButton_->setText(tr("sell-button-text"));
        }
    }

signals:
    void orderConfirmed(const PortfolioWindow* portfolio, exchange::OperationType operationType, const exchange::OrderData& data);

private:
    void init_() {
        // Create the order widget
        orderWidget_ = new OrderWidget(false, this); // Not simulate
        // Create the order Button
        orderButton_ = new QPushButton(tr("buy-button-text"));
        orderButton_->setMinimumHeight(30);
        orderButton_->setEnabled(false);
        orderButton_->setFocusPolicy(Qt::NoFocus);
        // Create the order form layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(orderWidget_);
        mainLayout->addWidget(orderButton_);
        setLayout(mainLayout);

        connect(orderWidget_, &OrderWidget::operationTypeChanged, [this] (exchange::OperationType operationType) {
            if (operationType == exchange::OperationType::Buy) {
                orderButton_->setText(tr("buy-button-text"));
            } else {
                orderButton_->setText(tr("sell-button-text"));
            }
        });
        connect(orderWidget_, &OrderWidget::inputValidated, [this] (bool valid) {
            orderButton_->setEnabled(valid);
        });
        connect(orderButton_, &QPushButton::clicked, [this] () {
            if (QMessageBox::question(this, tr("order-confirm-title"), tr("order-confirm-question")) == QMessageBox::Yes) {
                emit orderConfirmed(orderWidget_->getPortfolio(), orderWidget_->getOperationType(), orderWidget_->getOrderData());
                orderWidget_->resetInputFields();
            }
        });
    }

private:
    OrderWidget* orderWidget_;
    QPushButton* orderButton_;
};

} // namespace hmi

#endif // HMI_ORDERFORM_H
