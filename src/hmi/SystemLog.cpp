// Class header file for the SystemLog class
#include "SystemLog.h"

// Qt headers
#include <QtWidgets/QHeaderView>
#include <QtCore/QDateTime>

// headers
#include "Types.h"

// hmi headers
#include "hmi/MyQtUtils.h"


namespace hmi {
  
    SystemLog::SystemLog(QWidget *parent)
        : QTableWidget(parent)
        {
            setColumnCount(3);
            setHorizontalHeaderLabels({tr("date-title"), tr("type-title"), tr("description-title")});
            // Set a different color for the header
            horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #f0f0f0 }");
            // Set the horizaontal header height
            horizontalHeader()->setDefaultSectionSize(14);
            horizontalHeader()->setVisible(false);

            // Remove the vertical header
            verticalHeader()->setVisible(false);
            // Configure diferent stretch for each column
            horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
            horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
            // Disable editing
            setEditTriggers(QAbstractItemView::NoEditTriggers);
            // Set the selection behavior to select rows
            setSelectionBehavior(QAbstractItemView::SelectRows);
            // Set the selection mode to single selection
            setSelectionMode(QAbstractItemView::SingleSelection);
            // Set the row height
            verticalHeader()->setDefaultSectionSize(12);
        }

    void SystemLog::info(const QString &text)
    {
        QString date = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
        int row = rowCount();
        insertRow(row);
        setTrace_(row, TraceType::Info, text);
    }

    void SystemLog::warning(const QString &text)
    {
        QString date = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
        int row = rowCount();
        insertRow(row);
        setTrace_(row, TraceType::Warning, text);
    }

    void SystemLog::error(const QString &text)
    {
        QString date = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
        int row = rowCount();
        insertRow(row);
        setTrace_(row, TraceType::Error, text);
    }

    void SystemLog::trace(const QString &text)
    {
        QString date = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
        int row = rowCount();
        insertRow(row);
        setTrace_(row, TraceType::Debug, text);
    }

    QIcon SystemLog::getIcon_(TraceType type)
    {
        switch (type) {
            case TraceType::Info:
                return QIcon(":/icons/information.png");
            case TraceType::Warning:
                return QIcon(":/icons/warning.png");
            case TraceType::Error:
                return QIcon(":/icons/error.png");
            case TraceType::Debug:
            default:
                return QIcon(":/icons/debug.png");
        }
    }

    QColor SystemLog::getColor_(TraceType type)
    {
        switch (type) {
            case TraceType::Info:
                return QColor(INFORMATIONCOLOR);
            case TraceType::Warning:
                return QColor(WARNINGCOLOR);
            case TraceType::Error:
                return QColor(DANGERCOLOR);
            case TraceType::Debug:
            default:
                return QColor(DEBUGCOLOR);
        }
    }

    QString SystemLog::toQString(TraceType type)
    {
        switch (type) {
            case TraceType::Info:
                return "Info";
            case TraceType::Warning:
                return "Warning";
            case TraceType::Error:
                return "Error";
            case TraceType::Debug:
            default:
                return "Debug";
        }
    }

    void SystemLog::setTrace_(int row, TraceType type, const QString &text)
    {
        QTableWidgetItem* item;
        item = new QTableWidgetItem(QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss"));
        item->setBackground(QColor("#f0f0f0"));
        item->setForeground(QColor("#393939"));
        setItem(row, 0, item);
        item = new QTableWidgetItem(toQString(type));
        item->setIcon(getIcon_(type));
        item->setForeground(Qt::white);
        item->setBackground(getColor_(type));
        item->setFont(QFont("Arial", 10, QFont::Bold));
        item->setTextAlignment(Qt::AlignCenter);
        setItem(row, 1, item);
        item = new QTableWidgetItem(text);
        item->setForeground(QColor("#393939"));
        setItem(row, 2, item);
        // Scroll to the last row
        scrollToBottom();
    }

} // namespace hmi
