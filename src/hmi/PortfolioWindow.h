#ifndef HMI_PORTFOLIOWINDOW_H
#define HMI_PORTFOLIOWINDOW_H


// Qt headers
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QDockWidget>
#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtCore/QTimer>

// hmi headers

// exchange headers
#include "exchange/Portfolio.h"
#include "exchange/Exchange.h"


namespace hmi {


    // Subclass of QTimer to add a counter (seconds) to the timeout signal that will be set to 0 on start and will be incremented on each timeout signal
    class CounterTimer : public QTimer {
        Q_OBJECT

    public:
        CounterTimer(QObject* parent = nullptr) : QTimer(parent) {
            connect(this, &QTimer::timeout, this, &CounterTimer::onTimeout);
        }

    public slots:
        void start(int msec = 1000) {
            period_ = msec;
            cycles_ = 0;
            QTimer::start(msec);
        }

    private slots:
        void onTimeout() {
            ++cycles_;
            emit timeout(cycles_*period_/1000);
        }

    signals:
        void timeout(unsigned int seconds);

    private:
        unsigned int period_;
        unsigned int cycles_;
    };


    // Create a class that represents the main part of the portfolio window. It will be a table that contain the list of operations with the following columns:
    // Date: the date of the operation
    // Operation: the type of operation (buy, sell, deposit, withdraw)
    // Ticket: the asset involved in the operation
    // Quantity: the quantity of the asset
    // Description: a description of the operation
    class PortfolioMain : public QTableWidget {
        Q_OBJECT

    public:
        explicit PortfolioMain(QWidget *parent = nullptr);
        ~PortfolioMain();

        void setOperation(unsigned int row, std::shared_ptr<exchange::Operation> operation);

        void retranslateUi();
    
    private:
        void init_();
        void selectRow_(const std::chrono::system_clock::time_point& date);

        void setTranslatableText_();
        void setTranslatableText_(unsigned int row);
    
    private:
        // Create a layout for the main part of the portfolio window
        QVBoxLayout* mainLayout_;
    };
    
    
    
    // Create a class that represents the sidebar part of the portfolio window
    class PortfolioBalance : public QTableWidget {
        Q_OBJECT

    public:
        explicit PortfolioBalance(exchange::Ticket referenceCurrency, QWidget *parent = nullptr)
            : QTableWidget(parent)
            , referenceCurrency_(referenceCurrency) {
            init_();
        }

        ~PortfolioBalance() {}

    public:
        void setBalance(unsigned int row, const exchange::Balance& balance, double averagePrice, double price, exchange::Ticket referenceCurrency);

        void retranslateUi();

    signals:
        void showBalanceInformation(const exchange::Balance& balance);
        void buyAssetUsing(exchange::Ticket asset, double multiplier);
        void buyAssetUsingTheProfit(exchange::Ticket asset);
        void sellAssetUsing(exchange::Ticket asset, double multiplier);
        void sellAssetToLeaveTheProfit(exchange::Ticket asset);

    private:
        // The init_() method will create the layout of the portfolio sidebar
        void init_();
        void configureActionsFor_(exchange::OperationType type);

        void setTranslatableText_();

    private:
        exchange::Ticket referenceCurrency_;

        QVBoxLayout* mainLayout_;
        QMenu* contextMenu_;
        QAction* buyUsingTheHalfAction_;
        QAction* buyUsingTheThirdAction_;
        QAction* buyUsingTheFourthAction_;
        QAction* buyUsingTheFifthAction_;
        QAction* buyUsingTheProfitAction_;
        QAction* buyUsingAllAction_;
        QAction* sellTheHalfAction_;
        QAction* sellTheThirdAction_;
        QAction* sellTheFourthAction_;
        QAction* sellTheFifthAction_;
        QAction* sellToLeaveTheProfit;
        QAction* sellAllAction_;
    };


    // Create a class to show in detail the entries vector of the class exchange::Balance. It will be a table with the following columns:
    // Date: the date of the operation
    // Type: the type of operation (buy, sell, depotsit, withdraw)
    // Balance: the balance of the asset
    class PortfolioBalanceDetails : public QTableWidget {
        Q_OBJECT

    public:
        explicit PortfolioBalanceDetails(QWidget *parent = nullptr);
        ~PortfolioBalanceDetails();

    signals:
        void calculateDetailsFor(const std::vector<std::string>& ids);

    public:
        void setBalanceDetails(exchange::Ticket ticket, const std::vector<exchange::BalanceEntry>& entries);

        void retranslateUi();

        void focusOutEvent(QFocusEvent *event);
        
    private:
        void init_();
        QString getText_(exchange::OperationType type, double quantity, exchange::Ticket ticket) const;
        QColor getForegroundColor_(exchange::OperationType type, double quantity) const;

        void setTranslatableText_();
        void setTranslatableText_(unsigned int row);

    private:
        QVBoxLayout* mainLayout_;
        QMenu* contextMenu_;
        QAction* calculateDetailsForAction_;
    };




    // Create a class that represents the bottom part of the portfolio window
    class PortfolioStats : public QWidget {
        Q_OBJECT

    public:
        explicit PortfolioStats(exchange::Ticket referenceCurrency, QWidget *parent = nullptr)
            : QWidget(parent)
            , referenceCurrency_(referenceCurrency){
            init_();
        }

        ~PortfolioStats() {}

    public:
        void updateView(const exchange::StatsOverview& stats);

        void retranslateUi();

    private:
        // The init_() method will create the layout of the portfolio bottom
        void init_();

        void setTranslatableText_();

    private:
        exchange::Ticket referenceCurrency_;

        // Create a layout for the bottom part of the portfolio window
        QVBoxLayout* mainLayout_;

        // Show the materialized profit and significant data
        QLabel* costLabel_;
        QLineEdit* costEdit_;
        QLabel* worthLabel_;
        QLineEdit* worthEdit_;
        QLabel* gainLossLabel_;
        QLineEdit* gainLossEdit_;
        QLabel* netProfitLabel_;
        QLineEdit* netProfitEdit_;
        QLabel* taxesLabel_;
        QLineEdit* taxesEdit_;
        QLabel* volumeLabel_;
        QLineEdit* volumeEdit_;

        // Show the unmaterialized profit and significant data
        QLabel* investmentCostLabel_;
        QLineEdit* unmaterializedCostEdit_;
        QLabel* unmaterializedWorthLabel_;
        QLineEdit* unmaterializedWorthEdit_;
        QLabel* unmaterializedGainLossLabel_;
        QLineEdit* unmaterializedGainLossEdit_;
        QLabel* unmaterializedNetProfitLabel_;
        QLineEdit* unmaterializedNetProfitEdit_;
        QLabel* unmaterializedTaxesLabel_;
        QLineEdit* unmaterializedTaxesEdit_;
        QLabel* unmaterializedVolumeLabel_;
        QLineEdit* unmaterializedVolumeEdit_;
    };


    // Class LabeledSlider: inherits from QWidget
    class LabeledSlider : public QWidget {
        Q_OBJECT

    public:
        explicit LabeledSlider(QWidget *parent = nullptr) 
            : QWidget(parent), label_(nullptr), slider_(nullptr), valueLabel_(nullptr) {
            init_();
        }
    
        ~LabeledSlider() {}

    signals:
        void valueChanged(int value);

    public:
        void setText(const QString& text) {
            label_->setText(text);
        }

        void setToolTip(const QString& text) {
            label_->setToolTip(text);
            slider_->setToolTip(text);
            valueLabel_->setToolTip(text);
        }

        void setRange(int min, int max) {
            slider_->setRange(min, max);
        }

        void setValue(int value) {
            slider_->setValue(value);
            valueLabel_->setText(QString::number(value)+ "%");
        }
        int value() const {
            return slider_->value();
        }

        void setEnabled(bool enabled) {
            slider_->setEnabled(enabled);
            valueLabel_->setEnabled(enabled);
        }
        bool isEnabled() const {
            return slider_->isEnabled();
        }

    private:
        // The init_() method will create the layout of the labeled slider
        void init_() {
            // Create the label
            label_ = new QLabel(this);
            label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            label_->setMaximumWidth(100);
            // Create the slider
            slider_ = new QSlider(Qt::Horizontal, this);
            slider_->setRange(-8, 20);
            slider_->setValue(0);
            slider_->setTickInterval(1);
            slider_->setSingleStep(1);
            slider_->setPageStep(1);
            //slider_->setFixedWidth(100);
            slider_->setVisible(true);
            // Create the value label
            valueLabel_ = new QLabel(this);
            valueLabel_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            valueLabel_->setFixedWidth(30);
            // Create the main layout
            QHBoxLayout* mainLayout = new QHBoxLayout(this);
            mainLayout->setContentsMargins(5, 0, 5, 0);
            mainLayout->addWidget(label_);
            mainLayout->addWidget(slider_);
            mainLayout->addWidget(valueLabel_);
            mainLayout->setStretch(0, 0);
            mainLayout->setStretch(2, 0);
            setLayout(mainLayout);
            // Connect the slider signal to the lambda function that will update the value label
            connect(slider_, &QSlider::valueChanged, [this](int value) {
                valueLabel_->setText(QString::number(value)+ "%");
                emit valueChanged(value);
            });
        }

    private:
        QString labelText_;
        QLabel* label_;
        QSlider* slider_;
        QLabel* valueLabel_;
    };


    // Class PortfolioWindow: inherits from QMainWindow
    class PortfolioWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit PortfolioWindow(const exchange::PortfolioData& data, exchange::Market market, QWidget *parent = nullptr);
        ~PortfolioWindow();

        const QString& getName() const { return name_; }
        const QString& getFilename() const { return filename_; }
        void setFilename(const QString& filename) { filename_ = filename; }

        const exchange::GeneralBalance& getGeneralBalance() const { return balance_; }

        bool isModified() const {
            return modified_;
        }
        void setModified(bool modified);

        bool isLocked() const {
            return locked_;
        }

        void setReadOnly(bool readOnly);
        bool isReadOnly() const { return readOnly_; }

        void updateView(const exchange::PortfolioUpdate& data);
        void updateView(const exchange::PortfolioRemove& data);

        void setMarket(exchange::Market market);
        void setTickersInformation(const exchange::TickersInformation& tickersInformation);
        const exchange::TickersInformation& getTickersInformation() const {
            return tickersInformation_;
        }

        unsigned int getTotalOperations() const {
            return mainWidget_->rowCount();
        }

        inline exchange::Ticket getReferenceCurrency() const {
            return referenceCurrency_;
        }

        inline float getMultiplier() const {
            return multiplier_;
        }

        inline float getIncrement(exchange::Ticket asset) const {
            auto it = increments_.find(asset);
            if(it != increments_.end()) {
                return it->second;
            }
            return 0;
        }

        inline double getLastPrice() const {
            return tickersInformation_.getLastPrice(market_);
        }

        double getSimulatedLastPrice(exchange::Market market) const;
        double getSimulatedLastPrice() const {
            return getSimulatedLastPrice(market_);
        }

        exchange::StatsOverview getOverview() const {
            return stats_.getOverview(tickersInformation_.get(), multiplier_, increments_);
        }

        const exchange::StatsData& getStats() const {
            return stats_;
        }
    
        void retranslateUi();

    signals:
        void savePortfolio(const QString& name);
        void savePortfolioAs(const QString& name);
        void lockPortfolio(const QString &name, bool locked);
        void orderAsset();
        void transferAsset();
        void removeLastPortfolioOperation(const PortfolioWindow* window);
        void readOnlyTimeout(const PortfolioWindow* window, unsigned int seconds);

        void buyAssetUsing(const PortfolioWindow* window, exchange::Ticket asset, double percentage);
        void buyAssetUsingTheProfit(const PortfolioWindow* window, exchange::Ticket asset);
        void sellAssetUsing(const PortfolioWindow* window, exchange::Ticket asset, double percentage);
        void sellAssetToLeaveTheProfit(const PortfolioWindow* window, exchange::Ticket asset);

        void calculateDetailsFor(const std::vector<std::string>& ids);

        void info(const QString &text);
        void warning(const QString &text);
        void error(const QString &text);
        void trace(const QString &text);

        void closed(PortfolioWindow* window);

    private slots:
        // Slots to handle the actions of the toolbar
        void savePortfolioTriggered_() { emit savePortfolio(name_); }
        void savePortfolioAsTriggered_() { emit savePortfolioAs(name_); }
        void lockPortfolioToggled_(bool checked);
        void removeLastPortfolioOperationTriggered_();

        void showBalanceInformation_(const exchange::Balance& balance);

    protected:
        void closeEvent(QCloseEvent *event) override;

    private:
        // The init_() method will create the layout of the portfolio window
        void init_();
        void addOperations_(const std::vector<std::shared_ptr<exchange::Operation>>& operations);
        void addOperation_(std::shared_ptr<exchange::Operation> operation);
        void removeOperation_(std::chrono::system_clock::time_point timePoint);
        void setBalance_(const exchange::GeneralBalance& generalBalance);
        void updateWindowTitle_();
        void enablePortfolioActions_(bool portfolioIsModified);
        void updateSimulatedLastPrice_();
        void updatePriceModifiers_();

        inline void setMultiplier_(float multiplier) {
            multiplier_ = multiplier;
            statsWidget_->updateView(
                stats_.getOverview(tickersInformation_.get(), multiplier_, increments_)
            );
            setBalance_(balance_);
        }

        inline void setIncrement_(exchange::Ticket asset, float increment) {
            increments_[asset] = increment;
            statsWidget_->updateView(
                stats_.getOverview(tickersInformation_.get(), multiplier_, increments_)
            );
            setBalance_(balance_);
        }

        inline void resetModifiers_() {
            multiplier_ = 1.0;
            increments_.clear();
            statsWidget_->updateView(
                stats_.getOverview(tickersInformation_.get(), multiplier_, increments_)
            );
            setBalance_(balance_);
        }

        void setTranslatableText_();

    private:
        QString name_;
        QString filename_;
        exchange::Ticket referenceCurrency_;
        exchange::GeneralBalance balance_;
        mutable bool modified_;
        bool locked_;
        bool readOnly_;

        exchange::Market market_;
        exchange::Ticket asset_;
        exchange::Ticket currency_;

        exchange::StatsData stats_;
        exchange::TickersInformation tickersInformation_;
        float multiplier_;
        std::map<exchange::Ticket, float> increments_;

        QToolBar* toolbar_;
        QAction *savePortfolioAction;
        QAction *savePortfolioAsAction;
        QAction *showDocksAction;
        QAction *lockPortfolioAction;
        QAction *orderAssetAction;
        QAction *transferAssetAction;
        QAction *removeLastPortfolioOperationAction;

        QLabel* simulatedLastLabel_;
        QLineEdit *simulatedLastLineEdit_;
        QToolBar* priceToolbar_;
        LabeledSlider* marketModifier_;
        LabeledSlider* assetModifier_;

        QVBoxLayout* mainLayout_;
        PortfolioMain* mainWidget_;
        PortfolioStats* statsWidget_;
        PortfolioBalance* portfolioBalance_;
        PortfolioBalanceDetails* portfolioBalanceDetails_;

        QDockWidget* balanceDock_;
        QDockWidget* balanceDetailsDock_;

        QStatusBar* statusBar_;
        CounterTimer statusBarTimer_;
    };

}

#endif // HMI_PORTFOLIOWINDOW_H
