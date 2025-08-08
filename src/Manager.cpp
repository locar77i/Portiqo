// Headers
#include "Manager.h"

// Stl headers
#include <iostream>
#include <chrono>

// Qt headers
#include <QtWidgets/QMessageBox>



namespace exchange {

// Implementation of the Manager class
Manager::Manager()
    : QObject()
    , controller_() {
    
}

void Manager::init() {
    controller_.setTraceSignal([this](std::string msg) {
        emit trace(QString(msg.c_str()));
    });
    controller_.setInfoSignal([this](std::string msg) {
        emit info(QString(msg.c_str()));
    });
    controller_.setWarningSignal([this](std::string msg) {
        emit warning(QString(msg.c_str()));
    });
    controller_.setErrorSignal([this](std::string msg) {
        emit error(QString(msg.c_str()));
    });
    controller_.init();
}


// PUBLIC SLOTS
void Manager::exec() {
    // Do something
}

void Manager::createPortfolio(const QString& name, exchange::Ticket referenceCurrency, double capital) {
    try {
        PortfolioData data;
        controller_.createPortfolio(name.toStdString(), referenceCurrency, capital, data);
        emit portfolioCreated(data, false);
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

void Manager::openPortfolio(const QString& filename) {
    try {
        PortfolioData data;
        controller_.openPortfolio(filename.toStdString(), data);
        emit portfolioCreated(data, true);
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

void Manager::savePortfolio(const QString& name, bool feedback) {
    try {
        unsigned int numOperations;
        if(controller_.savePortfolio(name.toStdString(), numOperations)) {
            // "Portfolio '%1' saved (Total: %2 operations)"
            emit info(tr("portfolio-saved-info").arg(name).arg(numOperations));
            if(feedback) {
                emit portfolioSaved(name, numOperations);
            }
        }
        else {
            // "No changes to save on portfolio '%1'"
            emit warning(tr("no-changes-to-save-on-portfolio").arg(name));
        }
    } catch (const std::exception& e) {
        emit warning(e.what());
    }    
}

void Manager::savePortfolioAs(const QString& name, const QString& filename, bool feedback) {
    try {
        unsigned int numOperations = 0;
        controller_.savePortfolioAs(name.toStdString(), filename.toStdString(), numOperations);
        // "Portfolio '%1' saved into file '%2' (Total: %3 operations)"
        emit info(tr("portfolio-saved-as-info").arg(name).arg(numOperations));
        if(feedback) {
            emit portfolioSavedAs(name, filename, numOperations);
        }
        
    } catch (const std::exception& e) {
        emit warning(e.what());
    }    
}

void Manager::closePortfolio(const QString& name, bool force) {
    try {
        bool ok = controller_.closePortfolio(name.toStdString(), force);
        if(!ok) {
            // "Portfolio '%1' has unsaved changes"
            emit warning(tr("portfolio-has-unsaved-changes").arg(name));
        }
        else {
            // "Portfolio '%1' closed"
            emit info(tr("portfolio-closed").arg(name));
        }
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

void Manager::lockPortfolio(const QString& name, bool locked) {
    try {
        controller_.lockPortfolio(name.toStdString(), locked);
        if(locked) {
            // "The portfolio '%1' is now locked"
            emit info(tr("portfolio-locked").arg(name));
        } else {
            // "The portfolio '%1' is now unlocked"
            emit info(tr("portfolio-unlocked").arg(name));
        }
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

void Manager::order(exchange::OperationType operationType, const QString& name, const OrderData& data) {
    try {
        //DELETEME: This is a test to simulate a large transfer execution
        //std::this_thread::sleep_for(std::chrono::seconds(10));
        PortfolioUpdate updateData;
        controller_.order(operationType, name.toStdString(), data, updateData);
        emit portfolioOperationAdded(name, updateData);
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

void Manager::simulateOrder(exchange::OperationType operationType, const QString& name, const DummyOrder& data) {
    try {
        //DELETEME: This is a test to simulate a large transfer execution
        //std::this_thread::sleep_for(std::chrono::seconds(10));
        PortfolioUpdate updateData;
        controller_.simulateOrder(operationType, name.toStdString(), data, updateData);
        emit portfolioOperationAdded(name, updateData);
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

void Manager::transfer(exchange::OperationType operationType, const QString& name, const TransferData& transfer) {
    try {
        //DELETEME: This is a test to simulate a large transfer execution
        //std::this_thread::sleep_for(std::chrono::seconds(40));
        PortfolioUpdate updateData;
        controller_.transfer(operationType, name.toStdString(), transfer, updateData);
        // "Portfolio '%1' - %2 operation: %3"
        emit info(tr("portfolio-operation-info").arg(name).arg(QString::fromStdString(getOperationName(operationType))).arg(QString::fromStdString(transfer.description())));
        emit portfolioOperationAdded(name, updateData);
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

void Manager::simulateTransfer(exchange::OperationType operationType, const QString& name, const DummyTransfer& transfer) {
    try {
        //DELETEME: This is a test to simulate a large transfer execution
        //std::this_thread::sleep_for(std::chrono::seconds(10));
        PortfolioUpdate updateData;
        controller_.simulateTransfer(operationType, name.toStdString(), transfer, updateData);
        emit info(tr("portfolio-operation-info").arg(name).arg(QString::fromStdString(getOperationName(operationType))).arg(QString::fromStdString(transfer.description())));
        emit portfolioOperationAdded(name, updateData);
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

void Manager::removeLastPortfolioOperation(const QString& name) {
    try {
        //DELETEME: This is a test to simulate a large transfer execution
        //std::this_thread::sleep_for(std::chrono::seconds(40));
        PortfolioRemove data;
        controller_.removeLastPortfolioOperation(name.toStdString(), data);
        emit portfolioOperationRemoved(name, data);
    } catch (const std::exception& e) {
        emit warning(e.what());
    }
}

} // namespace exchange
