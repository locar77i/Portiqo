#ifndef HMI_TRANSFERWIDGET_H
#define HMI_TRANSFERWIDGET_H

// Qt headers
#include <QtWidgets/QDialog>
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


// headers
#include "Types.h"

// hmi headers
#include "hmi/PortfolioWindow.h"
#include "hmi/MyQtUtils.h"


namespace hmi {

class TransferWidget : public QWidget {
    Q_OBJECT

public:
    TransferWidget(bool simulate, QWidget* parent = nullptr)
        : QWidget(parent)
        , portfolio_(nullptr)
        , asset_(exchange::Ticket::EUR)
        , simulated_(simulate)
        , updating_(false) {
        init_();
        setAsset(exchange::Ticket::EUR);
        setOperationType(exchange::OperationType::Deposit);
    }

    ~TransferWidget() {}

signals:
    void operationTypeChanged(exchange::OperationType operationType);
    void inputValidated(bool valid);

public:
    void updateView(const PortfolioWindow* portfolio);
    inline void updateView() {
        updateView(portfolio_);
    }
    void setAsset(exchange::Ticket asset);
    void setOperationType(exchange::OperationType operationType);

    const PortfolioWindow* getPortfolio() const {
        return portfolio_;
    }
    exchange::OperationType getOperationType() const;
    exchange::TransferData getTransferData() const;

    void resetInputFields();
    void resetDialogFields();

    void retranslateUi();

private slots:
    void onOperationTypeChanged_(QAbstractButton* button);
    void onAssetComboBoxChanged_(int index);
    void onQuantityLineEditChanged_(const QString& text);
    void onSliderValueChanged_(int value);
    void onBalanceLineEditChanged_(const QString& text);
    void validateInput_();

private:
    void init_();
    void setOperationType_(exchange::OperationType operationType);
    void updateBalanceWidget_();
    void updatefeeWidget_();

private:
    const PortfolioWindow* portfolio_;
    exchange::Ticket asset_;
    bool simulated_;

    QPushButton* depositButton_;
    QPushButton* withdrawButton_;
    QButtonGroup* buttonGroup_;
    QComboBox* assetComboBox_;
    QLabel* quantityLabel_;
    QLineEdit* quantityLineEdit_;
    QLabel* quantityTickerLabel_;
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

#endif // HMI_TRANSFERWIDGET_H
