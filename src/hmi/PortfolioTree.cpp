// Class header file for the PortfolioTree class
#include "PortfolioTree.h"

// Qt headers
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtGui/QContextMenuEvent>

// hmi headers
#include "MyQtUtils.h"


namespace hmi {

    PortfolioTree::PortfolioTree(QWidget *parent)
        : QTreeWidget(parent) {
        init_();
    }

    void PortfolioTree::add(const PortfolioWindow* portfolio) {
        if(portfolio) {
            // Add a new row to the portfolio tree
            QTreeWidgetItem* item = new QTreeWidgetItem(this);
            // Configure the 1st column
            item->setIcon(0, QIcon(":/icons/Compressed-File-32x32.png"));
            item->setText(0, portfolio->getName());
            item->setData(0, Qt::UserRole, QVariant::fromValue(portfolio));
            // Configure the 2nd column
            item->setTextAlignment(1, Qt::AlignRight);
            QFont font = item->font(1);
            font.setPointSize(13);
            font.setStretch(QFont::Condensed);
            item->setFont(1, font);
            // Create the first child for the number of operations
            QTreeWidgetItem* child = new QTreeWidgetItem(item);
            child->setIcon(0, QIcon(":/icons/Ruler-32x32.png"));
            // Set the static information of the item
            setTranslatableText_(item);
            // Set the dynamic information of the itemui
            updateView_(item, *portfolio);
        }
    }

    void PortfolioTree::updateView(const QString& name) {
        QTreeWidgetItem* item = findPortfolio_(name);
        if(item) {
            PortfolioWindow* portfolio = item->data(0, Qt::UserRole).value<PortfolioWindow*>();
            if(portfolio) {
                updateView_(item, *portfolio);
            }
        }
    }

    void PortfolioTree::removePortfolioItem(const QString& name) {
        // Find the portfolio in the tree
        QTreeWidgetItem* item = findPortfolio_(name);
        if (item) { // remove the item from the tree
            delete item;
        }
    }

    void PortfolioTree::contextMenuEvent(QContextMenuEvent* event) {
        // Get the selected item
        QTreeWidgetItem* item = itemAt(event->pos());
        if (item) {
            // Enable/disable the portfolio actions
            PortfolioWindow* portfolio = item->data(0, Qt::UserRole).value<PortfolioWindow*>();
            if (portfolio) {
                enablePortfolioActions_(true, portfolio->isModified());
            }
            // Show the context menu
            contextMenu_->exec(event->globalPos());
        }
        
    }

    void PortfolioTree::savePortfolio_() {
        QString name = getCurrentItemName_();
        if (!name.isEmpty()) {
            emit savePortfolio(name);
        }
    }

    void PortfolioTree::savePortfolioAs_() {
        QString name = getCurrentItemName_();
        if (!name.isEmpty()) {
            emit savePortfolioAs(name);
        }
    }

    void PortfolioTree::closePortfolioWindow_() {
        QString name = getCurrentItemName_();
        if (!name.isEmpty()) {
            emit closePortfolioWindow(name);
        }
    }
 
    void PortfolioTree::init_() {
        // Create the layout of the portfolio tree
        setColumnCount(2);
        setHeaderLabels({tr("name-title"), tr("profit-title")});
        // Set the width of the columns
        setColumnWidth(0, 140);
        setColumnWidth(1, 140);
        // Set the height of the rows
        setUniformRowHeights(true);
        // Set the selection behavior to select rows
        setSelectionBehavior(QAbstractItemView::SelectRows);
        // Set the selection mode to single selection
        setSelectionMode(QAbstractItemView::SingleSelection);
        // Set rows color
        setAlternatingRowColors(false);
        // set the width of the tree
        setMinimumWidth(280);

        createContextMenu_();

        // Connections
        connect(this, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem* item, int column) {
            // Check if the item is a top-level item
            if (item->parent() == nullptr) {
                PortfolioWindow* portfolio = item->data(0, Qt::UserRole).value<PortfolioWindow*>();
                emit portfolioSelected(portfolio->getName());
            }
        });
    }

    void PortfolioTree::createContextMenu_() {
        // Create the context menu
        contextMenu_ = new QMenu(this);
        // Add the actions to the context menu
        savePortfolioAction = contextMenu_->addAction(QIcon(":/icons/Save-32x32.png"), tr("save-portfolio"));
        savePortfolioAsAction = contextMenu_->addAction(QIcon(":/icons/Save-New-32x32.png"), tr("save-portfolio-as-action"));
        contextMenu_->addSeparator();
        closePortfolioWindowAction = contextMenu_->addAction(QIcon(":/icons/Close-32x32.png"), tr("close-protfolio-action"));

        // Connections
        connect(savePortfolioAction, &QAction::triggered, this, &PortfolioTree::savePortfolio_);
        connect(savePortfolioAsAction, &QAction::triggered, this, &PortfolioTree::savePortfolioAs_);
        connect(closePortfolioWindowAction, &QAction::triggered, this, &PortfolioTree::closePortfolioWindow_);
    }

    void PortfolioTree::updateView_(QTreeWidgetItem* item, const PortfolioWindow& portfolio) {
        if (item) {
            auto overview = portfolio.getOverview();
            // Update the total profit
            setTotalProfit_(item, overview.materialized.gainLoss, overview.referenceCurrency);
            // Update the item text
            QFont font = item->font(0);
            font.setBold(portfolio.isModified());
            item->setFont(0, font);
            item->setText(0, portfolio.getName() + (portfolio.isModified()? "*": ""));
            if(!portfolio.getFilename().isEmpty()) {
                item->setToolTip(0, portfolio.getFilename());
            }
            // update the number of operations and increment it
            QTreeWidgetItem* child = item->child(0);
            if (child) {
                child->setText(1, QString::number(portfolio.getTotalOperations()));
            }
        }
    }

    void PortfolioTree::enablePortfolioActions_(bool enabled, bool portfolioIsModified) {
        // Enable/disable the save portfolio action
        savePortfolioAction->setEnabled(enabled && portfolioIsModified);
        savePortfolioAsAction->setEnabled(enabled);
    }

    
    QTreeWidgetItem* PortfolioTree::findPortfolio_(const QString& name) {
        for (int ii = 0; ii < topLevelItemCount(); ++ii) {
            PortfolioWindow* portfolio = topLevelItem(ii)->data(0, Qt::UserRole).value<PortfolioWindow*>();
            if(portfolio && portfolio->getName() == name) {
                return topLevelItem(ii);
            }
        }
        return nullptr;
    }

    QString PortfolioTree::getCurrentItemName_() {
        QTreeWidgetItem* item = currentItem();
        if (item) {
            PortfolioWindow* portfolio = item->data(0, Qt::UserRole).value<PortfolioWindow*>();
            if (portfolio) {
                return portfolio->getName();
            }
        }
        return "";
    }
    
    void PortfolioTree::setTotalProfit_(QTreeWidgetItem *item, double totalProfit, exchange::Ticket referenceCurrency) {
        item->setText(1, MyQtUtils::stringfy(totalProfit, referenceCurrency, true));
        if (totalProfit > 0.1) {
            item->setIcon(1, QIcon(":/icons/Green-Up-Arrow-32x32.png"));
        }
        else if (totalProfit < -0.1) {
            item->setIcon(1, QIcon(":/icons/Red-Down-Arrow-32x32.png"));
        }
        else { // totalProfit -> 0
            item->setIcon(1, QIcon(":/icons/Grey-Right-Arrow-32x32.png"));
        }
    }

    void PortfolioTree::setTranslatableText_() {
        setHeaderLabels({tr("name-title"), tr("profit-title")});
        savePortfolioAction->setText(tr("save-portfolio"));
        savePortfolioAsAction->setText(tr("save-portfolio-as-action"));
        closePortfolioWindowAction->setText(tr("close-protfolio-action"));
    }

    void PortfolioTree::setTranslatableText_(QTreeWidgetItem* item) {
        item->setToolTip(1, tr("gross-profit-tooltip")); // The gross profit of the investment - El beneficio bruto de la inversión
        QTreeWidgetItem* child = item->child(0);
        if (child) {
            child->setText(0, tr("operations-label"));
        }    
    } 
   

} // namespace hmi
