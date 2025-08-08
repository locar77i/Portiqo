#ifndef HMI_MYQTUTILS_H
#define HMI_MYQTUTILS_H


// Qt  headers
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QComboBox>
#include <QtCore/QTranslator>
#include <QtCore/QTimer>

// hmi headers

// headers
#include "Types.h"

// exchange headers


// Color definitions
#define BUYCOLOR_BG       "#f3fff1"
#define SELLCOLOR_BG      "#ffdfdf"
#define DEPOSITCOLOR_BG   "#e8f6ff"
#define WITHDRAWCOLOR_BG  "#fffde0"

#define BUYCOLOR_FG       "#108200"
#define SELLCOLOR_FG      "#8d0000"
#define DEPOSITCOLOR_FG   "#005183"
#define WITHDRAWCOLOR_FG  "#9c9400"

#define INFORMATIONCOLOR  "#88d0fd"
#define WARNINGCOLOR      "#fdf788"
#define WARNINGCOLOR_BG   "#fffee6"
#define WARNINGCOLOR_FG   "#9c9400"
#define DANGERCOLOR       "#fd8888"
#define DANGERCOLOR_BG    "#fee7e7"
#define DANGERCOLOR_FG    "#8d0000"
#define ERRORCOLOR_FG     "#ff0000"
#define ERRORCOLOR_BG     "#ff0000"

#define DEBUGCOLOR        "#b1b1b1"



namespace hmi {

    const char BuyStyleSheet[] = "background-color: #f3fff1; color: #108200";
    const char SellStyleSheet[] = "background-color: #ffdfdf; color: #8d0000";
    const char DepositStyleSheet[] = "background-color: #e8f6ff; color: #005183";
    const char WithdrawStyleSheet[] = "background-color: #fffde0; color: #9c9400";

    const char GainStyleSheet[] = "background-color: #f3fff1; color: #108200";
    const char LossStyleSheet[] = "background-color: #ffdfdf; color: #8d0000";

    const char WarningStyleSheet[] = "background-color: #fffee6; color: #9c9400";
    const char DangerStyleSheet[] = "background-color: #fee7e7; color: #8d0000";
    const char ErrorStyleSheet[] = "background-color: #ff0000; color: #ffffff";


    /* Predefined Qt colors
        Qt::white,
        Qt::black,
        Qt::red,
        Qt::darkRed,
        Qt::green,
        Qt::darkGreen,
        Qt::blue,
        Qt::darkBlue,
        Qt::cyan,
        Qt::darkCyan,
        Qt::magenta,
        Qt::darkMagenta,
        Qt::yellow,
        Qt::darkYellow,
        Qt::gray,
        Qt::darkGray,
        Qt::lightGray
    */


    QString formatTimePoint(std::chrono::system_clock::time_point date, std::string format = "%d/%m/%Y %H:%M:%S");

    void loadTranslation(QApplication &app, QTranslator &translator, const QString &languageCode);


    class MyQtUtils {
    public:
        static const char* getStyleSheet(exchange::OperationType type);
        static QColor getBackgroundColor(exchange::OperationType type);
        static QColor getForegroundColor(exchange::OperationType type);
        static QLabel* createTickerLabel(exchange::Ticket ticket, unsigned int width);

        static QString stringfy(double value, exchange::Ticket currency, bool symbol = false, int ndecimals = -1);
        static QString stringfyHR(double value, exchange::Ticket currency, bool symbol = false, int ndecimals = -1);
        static QString stringfyLN(double value, exchange::Ticket currency, bool symbol);
        static QString stringfyPrice(double value, exchange::Ticket asset, exchange::Ticket currency);
        static QString stringfyPriceHR(double value, exchange::Ticket asset, exchange::Ticket currency);
    };


    class TimePointTableWidgetItem : public QTableWidgetItem {
    public:
        TimePointTableWidgetItem(const QString &text, std::chrono::system_clock::time_point data)
            : QTableWidgetItem(text), timePoint(data) {}

        std::chrono::system_clock::time_point getTimePoint() const { return timePoint; }

        bool operator<(const QTableWidgetItem &other) const override {
            const TimePointTableWidgetItem *otherItem = dynamic_cast<const TimePointTableWidgetItem*>(&other);
            if (otherItem) {
                return timePoint < otherItem->timePoint;
            }
            return QTableWidgetItem::operator<(other);
        }

    private:
        std::chrono::system_clock::time_point timePoint;
    };
    
} // namespace hmi


#endif // HMI_MYQTUTILS_H
