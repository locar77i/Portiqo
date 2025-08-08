#ifndef HMI_MAINWINDOW_H
#define HMI_MAINWINDOW_H

// Qt headers
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QStatusBar>
#include <QtGui/QAction>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>

// hmi headers
#include "hmi/NewDialog.h"
#include "hmi/PortfolioWindow.h"
#include "hmi/OrderForm.h"
#include "hmi/OrderDialog.h"
#include "hmi/TransferForm.h"
#include "hmi/TransferDialog.h"
#include "hmi/PortfolioTree.h"
#include "hmi/ProxiesManagerDialog.h"
#include "hmi/SystemLog.h"

// headers

// exchange headers
#include "exchange/Portfolio.h"
#include "exchange/Exchange.h"


namespace hmi
{
    // Class MainWindow: inherits from QMainWindow
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

        void loadTranslation(const QString &languageCode);

        void init() {
        }
    
        void retranslateUi();

        void setStyle(const QString &style) { qApp->setStyle(style); }

    signals:
        void createPortfolio(const QString &name, exchange::Ticket referenceCurrency, double capital);
        void openPortfolio(const QString &filename);
        void savePortfolio(const QString &name, bool feedback);
        void savePortfolioAs(const QString &name, const QString &filename, bool feedback);
        void lockPortfolio(const QString &name, bool locked);
        void orderSimulated(exchange::OperationType operationType, const QString &name, const exchange::DummyOrder &data);
        void orderConfirmed(exchange::OperationType operationType, const QString &name, const exchange::OrderData &data);
        void transferSimulated(exchange::OperationType operationType, const QString &name, const exchange::DummyTransfer &data);
        void transferConfirmed(exchange::OperationType operationType, const QString &name, const exchange::TransferData &data);
        void removeLastPortfolioOperation(const QString &name);
        void closePortfolio(const QString &name, bool force);
        void ping();
        void createProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market, unsigned int period);
        void slowdownProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market, unsigned int period);
        void pauseProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market);
        void resumeProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market);
        void destroyProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market);
        void finalize();

    public slots:
        void portfolioCreated(const exchange::PortfolioData &data, bool openFromFile);

        void portfolioOperationAdded(const QString& name, const exchange::PortfolioUpdate& update);
        void portfolioOperationRemoved(const QString& name, const exchange::PortfolioRemove& data);
        void portfolioSaved(const QString& name, unsigned int numOperations);
        void portfolioSavedAs(const QString& name, const QString& filename, unsigned int numOperations);

        void updateTickersInformationStats(exchange::Service service, exchange::ProxyType type, exchange::WorkStats workStats);
        void updateSystemStatusStats(exchange::Service service, exchange::ProxyType type, exchange::WorkStats workStats);
        void updateProxyStatus(exchange::Service service, exchange::ProxyType type, exchange::Market market, exchange::ProxyStatus status);
        void updateProxyStatistics(exchange::Service service, exchange::ProxyType type, exchange::Market market, exchange::WorkStats workStats);
        void updateSystemStatus(exchange::SystemStatus systemStatus);
        void updateTickersInformation(exchange::Service service, exchange::TickersInformation tickersInformation);
        void updateOhlcDataSummary(exchange::Market market, exchange::OhlcDataSummary ohlcDataSummary);

        void activateProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market, unsigned int period);
        void deactivateProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market);

        void showInfo(const QString &text) { systemLog_->info(text); }
        void showWarning(const QString &text) { systemLog_->warning(text); }
        void showError(const QString &text) { systemLog_->error(text); }
        void showTrace(const QString &text) { systemLog_->trace(text); }

    private slots:
        // Slots to handle the actions of the toolbar
        void newPortfolioTriggered_();
        void openPortfolioTriggered_();
        void savePortfolioTriggered_();
        void savePortfolioAsTriggered_();
        void saveAllPortfoliosTriggered_();
        // View slots
        void showPortfolioTreeTriggered_();
        void showSystemLogTriggered_();
        void showOrderFormTriggered_();
        void showTransferFormTriggered_();
        // Help slots
        void aboutTriggered_();

        // Slots to handle the actions of the portfolio tree
        void savePortfolio_(const QString &name);
        void savePortfolioAs_(const QString &name);
        void orderAsset_() { orderDialog_->showDialog(); }
        void transferAsset_() { transferDialog_->showDialog(); }
        void orderSimulated_(const PortfolioWindow* portfolio, exchange::OperationType operationType, const exchange::DummyOrder &data) {
            const_cast<PortfolioWindow*>(portfolio)->setReadOnly(true);
            updateStatusFromActivePortfolio_();
            emit orderSimulated(operationType, portfolio->getName(), data);
        }
        void orderConfirmed_(const PortfolioWindow* portfolio, exchange::OperationType operationType, const exchange::OrderData &data) {
            const_cast<PortfolioWindow*>(portfolio)->setReadOnly(true);
            updateStatusFromActivePortfolio_();
            emit orderConfirmed(operationType, portfolio->getName(), data);
        }
        void transferSimulated_(const PortfolioWindow* portfolio, exchange::OperationType operationType, const exchange::DummyTransfer &data) {
            const_cast<PortfolioWindow*>(portfolio)->setReadOnly(true);
            updateStatusFromActivePortfolio_();
            emit transferSimulated(operationType, portfolio->getName(), data);
        }
        void transferConfirmed_(const PortfolioWindow* portfolio, exchange::OperationType operationType, const exchange::TransferData &data) {
            const_cast<PortfolioWindow*>(portfolio)->setReadOnly(true);
            updateStatusFromActivePortfolio_();
            emit transferConfirmed(operationType, portfolio->getName(), data);
        }
        void removeLastPortfolioOperation_(const PortfolioWindow* portfolio) {
            const_cast<PortfolioWindow*>(portfolio)->setReadOnly(true);
            updateStatusFromActivePortfolio_();
            emit removeLastPortfolioOperation(portfolio->getName());
        }
        void readOnlyTimeout_(const PortfolioWindow* portfolio, unsigned int seconds);

        void closePortfolioWindow_(const QString &name);

        void portfolioWindowActivated_(QMdiSubWindow *window);
        void portfolioSelected_(const QString &name);

        void portfolioWindowClosed_(PortfolioWindow* portfolioWindow);

        void onMarketComboBoxChanged_(int index);

        void updateTickerInformation_();

    protected:
        void closeEvent(QCloseEvent *event) override;

    private:
        // Initialize
        void init_();
        void createActions_();
        void createMenubar_();
        void createToolbars_();

        PortfolioWindow* getActivePortfolio_();
        const PortfolioWindow* getActivePortfolio_() const { return getActivePortfolio_(); }
        PortfolioWindow* getPortfolioWindow_(const QString &name);
        QMdiSubWindow* getPortfolioSubWindow_(const QString &name);

        void save_(PortfolioWindow* portfolio, bool feedback = true);
        void saveAs_(PortfolioWindow* portfolio, bool feedback = true);

        void enablePortfolioActions_(const PortfolioWindow* portfolio) const;

        void setPortFolioFilename_(PortfolioWindow* portfolio, const QString& filename);

        void setPortfolioModified_(PortfolioWindow* portfolio, bool modified);

        void updateStatusFromActivePortfolio_();

        void setMarket_(exchange::Market market);

        void setTranslatableText_();

    private:
        // Kraken proxy information
        exchange::TickersInformation tickersInformation_;

        // Create a menu bar
        QMenu *fileMenu_;
        QMenu *viewMenu_;
        QMenu *helpMenu_;

        QToolBar *toolbar_;
        // File actions
        QAction *newPortfolioAction;
        QAction *openPortfolioAction;
        QAction *savePortfolioAction;
        QAction *savePortfolioAsAction;
        QAction *saveAllPortfoliosAction;
        QAction *exitAction;
        // View actions
        QAction *showPortfolioTreeAction;
        QAction *showSystemLogAction;
        QAction *showOrderFormAction;
        QAction *showTransferFormAction;
        // Help actions
        QAction *enLanguageAction;
        QAction *esLanguageAction;
        // Availables styles in Qt 6.8.1: Fusion, Windows, WindowsVista, Windows11
        QAction *fusionStyleAction;
        QAction *windowsStyleAction;
        QAction *windowsVistaStyleAction;
        QAction *windows11StyleAction;
        // About actions
        QAction *aboutAction;
        QAction *aboutQtAction;

        QToolBar *exchangeToolbar_;
        // Exchange actions
        QAction *pingAction_;
        // The market selector
        QComboBox *marketComboBox_;
        // Labels to show the TickeInformation values
        QLabel* marketLabel_;
        QLabel* lastLabel_;
        QLabel* tradesLabel_;
        QLabel* lowLabel_;
        QLabel* highLabel_;
        QLabel* volumeLabel_;
        // Line edits to show the TickeInformation values
        QLineEdit* lastLineEdit_;
        QLineEdit* tradesLineEdit_;
        QLineEdit* lowLineEdit_;
        QLineEdit* highLineEdit_;
        QLineEdit* volumeLineEdit_;
        QLineEdit* totalVolumeLineEdit_;

        // The new dialog instance
        NewDialog *newDialog_;

        PortfolioTree *portfolioTree_;
        OrderDialog *orderDialog_;
        TransferDialog *transferDialog_;
        OrderForm *orderForm_;
        TransferForm *transferForm_;
        SystemLog *systemLog_;

        QMdiArea *mdiArea;

        QDockWidget *portfolioTreeDockWidget_;
        QDockWidget *systemLogDockWidget_;
        QDockWidget *orderDockWidget_;
        QDockWidget *transferDockWidget_;

        // The StatusBar of the main window
        QStatusBar *statusBar_;
        QLabel *frequencyLabel_;
        QLineEdit *frequencyLineEdit_;

        // Proxies control
        QAction *executeProxiesAction_;

        QAction *showTickersInformationDialogAction_;
        QAction *showRecentTradesDialogAction_;
        QAction *showOrderBooksDialogAction_;
        
        ProxiesManagerDialog *tickersInformationDialog_;
        ProxiesManagerDialog *recentTradesDialog_;
        ProxiesManagerDialog *orderBooksDialog_;

        // The translator
        QTranslator translator_;
    };

} // namespace hmi

#endif // HMI_MAINWINDOW_H
