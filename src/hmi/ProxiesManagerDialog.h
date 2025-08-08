#ifndef HMI_SERVICEMANAGERDIALOG_H
#define HMI_SERVICEMANAGERDIALOG_H


// Qt headers
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMenu>
#include <QtGui/QAction>
#include <QtGui/QIcon>

// headers
#include "exchange/Exchange.h"



namespace hmi {

    class ProxiesManager : public QWidget {

        Q_OBJECT

    public:
        explicit ProxiesManager(exchange::Service service, QWidget *parent = nullptr)
            : QWidget(parent)
            , service_(service) {
            init_();
        }

        ~ProxiesManager() {}

    public:
        exchange::Service getServiceType() const {
            return service_;
        }

        void retranslateUi() {
            setTranslatableText_();
        }

        void executeProxy(exchange::Market market, unsigned int period);
        void activateProxy(exchange::Market market, unsigned int period);
        void deactivateProxy(exchange::Market market);
        void updateProxyStatus(exchange::Market market, exchange::ProxyStatus status);
        void updateProxyStatistics(exchange::Market market, exchange::WorkStats workStats);

    signals:
        void createProxy(exchange::Service service, exchange::Market market, unsigned int period);
        void slowdownProxy(exchange::Service service, exchange::Market market, unsigned int period);
        void pauseProxy(exchange::Service service, exchange::Market market);
        void resumeProxy(exchange::Service service, exchange::Market market);
        void destroyProxy(exchange::Service service, exchange::Market market);

    private slots:

    private:
        void init_();
        int findOrCreateRow_(exchange::Market market);
        void resetInputFields_();
        void setTranslatableText_();
        void setMarketComboBoxItemVisibility_(exchange::Market market, bool visible);
        QString getStatusLabel_(exchange::ProxyStatus status);
        QIcon getStatusIcon_(exchange::ProxyStatus status);
        QString getResponseLabel_(exchange::Response response);
        QIcon getResponseIcon_(exchange::Response response);
        
    private:
        exchange::Service service_;
        // Market combobox
        QLabel* marketLabel_;
        QComboBox* marketComboBox_;
        // Period combobox
        QLabel* periodLabel_;
        QComboBox* periodComboBox_;
        // Create button
        QPushButton* createButton_;
        // Active proxies table
        QTableWidget* activeProxies_;
        // Context menu
        QMenu* contextMenu_;
        QAction* slowdownProxyAction_;
        QAction* pauseProxyAction_;
        QAction* resumeProxyAction_;
        QAction* destroyProxyAction_;
        // Market and period must be used as ack
        exchange::Market market_;
        unsigned int period_;
    };


    class ProxiesManagerDialog : public QDialog {

        Q_OBJECT

    public:
        explicit ProxiesManagerDialog(exchange::ProxyType type, QWidget *parent = nullptr)
            : QDialog(parent)
            , type_(type) {
            init_();
        }

        ~ProxiesManagerDialog() {}

    public:
        inline exchange::ProxyType getProxyType() const {
            return type_;
        }

        void retranslateUi() {
            for(int i = 0; i < tabWidget_->count(); ++i) {
                ProxiesManager* proxiesManager = dynamic_cast<ProxiesManager*>(tabWidget_->widget(i));
                if(proxiesManager) {
                    proxiesManager->retranslateUi();
                }
            }
            setTranslatableText_();
        }

        void show() {
            resetInputFields_();
            QDialog::show();
        }

        void executeProxy(exchange::Service service, exchange::Market market, unsigned int period);
        void activateProxy(exchange::Service service, exchange::Market market, unsigned int period);
        void deactivateProxy(exchange::Service service, exchange::Market market);
        void updateProxyStatus(exchange::Service service, exchange::Market market, exchange::ProxyStatus status);
        void updateProxyStatistics(exchange::Service service, exchange::Market market, exchange::WorkStats workStats);

    signals:
        void createProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market, unsigned int period);
        void slowdownProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market, unsigned int period);
        void pauseProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market);
        void resumeProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market);
        void destroyProxy(exchange::Service service, exchange::ProxyType type, exchange::Market market);

    private:
        void init_();
        void resetInputFields_();
        void setTranslatableText_();
        ProxiesManager* getProxiesManager_(exchange::Service service);
        
    private:
        exchange::ProxyType type_;
        QTabWidget* tabWidget_;

    };


} // namespace hmi



#endif // HMI_SERVICEMANAGERDIALOG_H
