#ifndef HMI_PORTFOLIOTREE_H
#define HMI_PORTFOLIOTREE_H

// Qt headers
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QMenu>
#include <QtGui/QAction>

// headers
#include "Types.h"

// hmi headers
#include "hmi/PortfolioWindow.h"


namespace hmi {

    // Create class to swhow all porfolios opened in the application as a QTreeWidget with the following columns:
    // Name: the name of the portfolio
    // Filename: the filename of the portfolio
    // Operations: the number of operations in the portfolio
    // Profit: the total profit of the portfolio
    // Unmaterialized: the unmaterialized profit of the portfolio
    class PortfolioTree : public QTreeWidget {
        Q_OBJECT

    public:
        PortfolioTree(QWidget *parent = nullptr);

        void retranslateUi() {
            setTranslatableText_();
            for (int ii = 0; ii < topLevelItemCount(); ++ii) {
                setTranslatableText_(topLevelItem(ii));
            }
        }

    signals:
        void portfolioSelected(const QString& name);
        void savePortfolio(const QString& name);
        void savePortfolioAs(const QString& name);
        void closePortfolioWindow(const QString& name);

    public:
        void add(const PortfolioWindow* portfolio);
        void updateView(const QString& name);
        void removePortfolioItem(const QString& name);

        void contextMenuEvent(QContextMenuEvent* event) override;

    private slots:
        void savePortfolio_();
        void savePortfolioAs_();
        void closePortfolioWindow_();

    private:
        void init_();
        void createContextMenu_();
        void updateView_(QTreeWidgetItem* item, const PortfolioWindow& portfolio);
        void enablePortfolioActions_(bool enabled, bool portfolioIsModified = false);
        QTreeWidgetItem* findPortfolio_(const QString& name);
        QString getCurrentItemName_();
        void setTotalProfit_(QTreeWidgetItem* item, double totalProfit, exchange::Ticket referenceCurrency);

        void setTranslatableText_();
        void setTranslatableText_(QTreeWidgetItem* item);
        
    private:
        QMenu* contextMenu_;

        QAction *savePortfolioAction;
        QAction *savePortfolioAsAction;
        QAction* closePortfolioWindowAction;
    };

} // namespace hmi

#endif // HMI_PORTFOLIOTREE_H
