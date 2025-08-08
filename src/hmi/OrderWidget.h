#ifndef HMI_ORDERWIDGET_H
#define HMI_ORDERWIDGET_H

#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QScreen>
#include <QtWidgets/QMessageBox>
#include <QtCore/QMetaObject>

// headers
#include "Types.h"

// exchange headers
#include "exchange/Exchange.h"

// hmi headers
#include "hmi/PortfolioWindow.h"
#include "hmi/MyQtUtils.h"


namespace hmi
{

class OrderWidget : public QWidget {
    Q_OBJECT

public:
    OrderWidget(bool simulate, QWidget* parent = nullptr)
        : QWidget(parent)
        , portfolio_(nullptr)
        , market_(exchange::Market::BTC_EUR)
        , asset_(exchange::Ticket::BTC)
        , currency_(exchange::Ticket::EUR)
        , simulated_(simulate)
        , updating_(false) {
        init_();
        setMarket(exchange::Market::BTC_EUR);
        setOperationType(exchange::OperationType::Buy);
    }
    ~OrderWidget() {}

signals:
    void operationTypeChanged(exchange::OperationType operationType);
    void inputValidated(bool valid);

public:
    void updateView(const PortfolioWindow* portfolio);
    inline void updateView() {
        updateView(portfolio_);
    }

    void setMarket(exchange::Market market);
    void setOperationType(exchange::OperationType operationType);
    void setForBuy(double total) {
        totalLineEdit_->setText(QString::number(total, 'f', 2));
    }
    void setForSell(double quantity) {
        quantityLineEdit_->setText(QString::number(quantity, 'f', 8));
    }

    const PortfolioWindow* getPortfolio() const {
        return portfolio_;
    }
    exchange::OperationType getOperationType() const;
    exchange::OrderData getOrderData() const;

    void resetInputFields();

    void retranslateUi();

private slots:
    void onOperationTypeChanged_(QAbstractButton* button);
    void onPriceLineEditChanged_(const QString& text);
    void onPriceLineEditReturnPressed_();
    void onQuantityLineEditChanged_(const QString& text);
    void onTotalLineEditChanged_(const QString& text);
    void onSliderValueChanged_(int value);
    void onBalanceLineEditChanged_(const QString& text);
    void validateInput_();

private:
    void init_();
    void setOperationType_(exchange::OperationType operationType);
    void setLastAssetPrice_(bool updating);
    void updateBalanceWidget_();
    void updatefeeWidget_();
    
private:
    const PortfolioWindow* portfolio_;
    exchange::Market market_;
    exchange::Ticket asset_;
    exchange::Ticket currency_;
    bool simulated_;

    QPushButton* buyButton_;
    QPushButton* sellButton_;
    QButtonGroup* buttonGroup_;
    QLineEdit* priceLineEdit_;
    QLabel* priceTickerLabel_;
    QLineEdit* quantityLineEdit_;
    QLabel* quantityTickerLabel_;
    QLineEdit* totalLineEdit_;
    QLabel* totalTickerLabel_;
    QSlider* slider_;
    QLabel* balanceLabel_;
    QLineEdit* balanceLineEdit_;
    QLabel* balanceTickerLabel_;
    QLabel* feeLabel_;
    QLineEdit* feeLineEdit_;
    QLabel* feeTickerLabel_;

    bool updating_;
};

} // namespace hmi

#endif // HMI_ORDERWIDGET_H
