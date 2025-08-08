# Define the project type and Qt modules
TEMPLATE = app
CONFIG += qt console
QT += core gui widgets

# Define the project name and target
TARGET = Portfolio
DESTDIR = bin

# Define the source files
SOURCES += \
    src/main.cpp \
    src/Types.cpp \
    src/Utils.cpp \
    src/PortfolioData.cpp \
    src/Controller.cpp \
    src/Manager.cpp \
    src/hmi/MyQtUtils.cpp \
    src/hmi/NewDialog.cpp \
    src/hmi/OrderWidget.cpp \
    src/hmi/TransferWidget.cpp \
    src/hmi/ProxiesManagerDialog.cpp \
    src/hmi/PortfolioTree.cpp \
    src/hmi/PortfolioWindow.cpp \
    src/hmi/SystemLog.cpp \
    src/hmi/MainWindow.cpp \
    src/exchange/Portfolio.cpp \
    src/exchange/CommonTypes.cpp \
    src/exchange/Exchange.cpp \
    src/exchange/Kraken.cpp \
    src/exchange/Binance.cpp \
    src/exchange/Coinbase.cpp \
    src/exchange/ProxyTypes.cpp \
    src/exchange/KrakenProxy.cpp

# Define the header files
HEADERS += \
    src/ULIDGenerator.h \
    src/Types.h \
    src/Utils.h \
    src/PortfolioData.h \
    src/Controller.h \
    src/Manager.h \
    src/hmi/MyQtUtils.h \
    src/hmi/NewDialog.h \
    src/hmi/OrderWidget.h \
    src/hmi/OrderForm.h \
    src/hmi/OrderDialog.h \
    src/hmi/TransferWidget.h \
    src/hmi/TransferForm.h \
    src/hmi/TransferDialog.h \
    src/hmi/ProxiesManagerDialog.h \
    src/hmi/PortfolioTree.h \
    src/hmi/PortfolioWindow.h \
    src/hmi/SystemLog.h \
    src/hmi/MainWindow.h \
    src/exchange/Portfolio.h \
    src/exchange/CommonTypes.h \
    src/exchange/Exchange.h \
    src/exchange/OrderBook.h \
    src/exchange/RecentTrades.h \
    src/exchange/Kraken.h \
    src/exchange/Binance.h \
    src/exchange/Coinbase.h \
    src/exchange/ProxyTypes.h \
    src/exchange/ProxyStatistics.h \
    src/exchange/ProxiesManager.h

# Define the UI files (if any)
FORMS +=

# Define the resources files (if any)
RESOURCES += \
    Portfolio.qrc

# Define additional include directories
INCLUDEPATH += src

# Define additional libraries (if any)
LIBS += -Lpath/to/your/libs -lyourlib

# Define the output directory for the compiled binaries
DESTDIR = bin

# Define the build directory
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui
