// Header
#include "MyQtUtils.h"


namespace hmi {




    QString formatTimePoint(std::chrono::system_clock::time_point date, std::string format) {
        std::time_t date_time = std::chrono::system_clock::to_time_t(date);
        std::tm tm;
        localtime_s(&tm, &date_time);
        char dateStr[256];
        strftime(dateStr, sizeof(dateStr), format.c_str(), &tm);
        return QString(dateStr);
    }

    void loadTranslation(QApplication &app, QTranslator &translator, const QString &languageCode) {
        app.removeTranslator(&translator);
        if (translator.load(":/translations/Portfolio_" + languageCode + ".qm")) {
            app.installTranslator(&translator);
        }
    }

    // Static utility class 'MyQtUtils'
    // funtion that returns a diferent color for every OperationType
    const char* MyQtUtils::getStyleSheet(exchange::OperationType type) {
        switch (type) {
            case exchange::OperationType::Buy:
                return BuyStyleSheet;
            case exchange::OperationType::Sell:
                return SellStyleSheet;
            case exchange::OperationType::Deposit:
                return DepositStyleSheet;
            case exchange::OperationType::Withdraw:
                return WithdrawStyleSheet;
            default:
                return ErrorStyleSheet;
        }
    }

    QColor MyQtUtils::getBackgroundColor(exchange::OperationType type) {
        switch (type) {
            case exchange::OperationType::Buy:
                return QColor(BUYCOLOR_BG);
            case exchange::OperationType::Sell:
                return QColor(SELLCOLOR_BG);
            case exchange::OperationType::Deposit:
                return QColor(DEPOSITCOLOR_BG);
            case exchange::OperationType::Withdraw:
                return QColor(WITHDRAWCOLOR_BG);
            default:
                return QColor(ERRORCOLOR_BG);
        }
    }

    QColor MyQtUtils::getForegroundColor(exchange::OperationType type) {
        switch (type) {
            case exchange::OperationType::Buy:
                return QColor(BUYCOLOR_FG);
            case exchange::OperationType::Sell:
                return QColor(SELLCOLOR_FG);
            case exchange::OperationType::Deposit:
                return QColor(DEPOSITCOLOR_FG);
            case exchange::OperationType::Withdraw:
                return QColor(WITHDRAWCOLOR_FG);
            default:
                return QColor(ERRORCOLOR_FG);
        }
    }

    QLabel* MyQtUtils::createTickerLabel(exchange::Ticket ticket, unsigned int width) {
        QLabel* label = new QLabel(QString::fromStdString(exchange::getTicketName(ticket)));
        label->setFixedWidth(width);
        QFont labelFont = label->font();
        labelFont.setStretch(QFont::Condensed);
        label->setFont(labelFont);
        return label;
    }

    QString MyQtUtils::stringfy(double value, exchange::Ticket currency, bool symbol, int ndecimals) {
        unsigned int decimals = (ndecimals==-1? exchange::getDecimals(currency) : ndecimals);
        double val = exchange::roundDouble(value, decimals);
        QString number = QString::number(val, 'f', decimals);
        if(symbol) {
            return number + " " + QString::fromStdString(exchange::getTicketSymbol(currency));
        }
        return number;
    }

    QString MyQtUtils::stringfyHR(double value, exchange::Ticket currency, bool symbol, int ndecimals) {
        unsigned int decimals = (ndecimals==-1? exchange::getDecimals(currency) : ndecimals);
        double val = exchange::roundDouble(value, decimals);
        if(symbol) {
            return QLocale(QLocale::English).toString(val, 'f', decimals) + " " + QString::fromStdString(exchange::getTicketSymbol(currency));
        }
        return QLocale(QLocale::English).toString(val, 'f', decimals);
    }

 
    QString MyQtUtils::stringfyLN(double value, exchange::Ticket currency, bool symbol) {
        QString suffix;
        if (value >= 1e9) {
            value /= 1e9;
            suffix = "B";
        } else if (value >= 1e6) {
            value /= 1e6;
            suffix = "M";
        } else if (value >= 1e3) {
            value /= 1e3;
            suffix = "K";
        }
        if(symbol) {
            return QString::number(value, 'f', 1) + suffix + " " + QString::fromStdString(exchange::getTicketSymbol(currency));
        }
        return QString::number(value, 'f', 1) + suffix;
    }

    QString MyQtUtils::stringfyPriceHR(double value, exchange::Ticket asset, exchange::Ticket currency) {
        unsigned int decimals = exchange::getPriceDecimals(asset, currency);
        value = exchange::roundDouble(value, decimals);
        return QLocale(QLocale::English).toString(value, 'f', decimals) + " " + QString::fromStdString(exchange::getTicketSymbol(currency));
    }

    QString MyQtUtils::stringfyPrice(double value, exchange::Ticket asset, exchange::Ticket currency) {
        unsigned int decimals = exchange::getPriceDecimals(asset, currency);
        value = exchange::roundDouble(value, decimals);
        return QString::number(value, 'f', decimals);
    }

} // namespace hmi
