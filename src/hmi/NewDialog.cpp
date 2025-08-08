// Class header
#include "NewDialog.h"

// Qt headers
#include <QtCore/QRegularExpression>


// hmi headers
#include "hmi/MyQtUtils.h"


namespace hmi {

    NewDialog::NewDialog(QWidget *parent) : QDialog(parent) {
        init_();
    }

    void NewDialog::onCurrencyComboBoxChanged_(int index) {
        // Update the capital prefix
        exchange::Ticket currency = currencyComboBox_->currentData().value<exchange::Ticket>();
        capitalSpinBox_->setSuffix(QString(" ") + exchange::getTicketSymbol(currency).c_str());
    }

    void NewDialog::onCreateButtonClicked() {
        // Handle the creation of the portfolio
        QString name = nameEdit_->text();
        exchange::Ticket referenceCurrency = currencyComboBox_->currentData().value<exchange::Ticket>();
        double capital = capitalSpinBox_->value();
        emit createPortfolio(name, referenceCurrency, capital);
        accept();
    }

    void NewDialog::validateInput_() {
        // The create button must be enabled if the name is not empty
        bool nameValid = !nameEdit_->text().isEmpty();
        // The name only must contain letters and numbers, and must start with a letter
        QRegularExpression nameRegExp("^[a-zA-Z][a-zA-Z0-9]*$");
        nameValid = nameValid && nameRegExp.match(nameEdit_->text()).hasMatch();
        if(!nameValid && !nameEdit_->text().isEmpty()) {
            nameEdit_->setStyleSheet(WarningStyleSheet);
            nameEdit_->setToolTip(tr("invalid-portfolio-name-tooltip")); // Invalid name: you must enter a valid name (only letters and numbers, starting with a letter)
        } else {
            nameEdit_->setStyleSheet("");
            nameEdit_->setToolTip("");
        }
        createButton_->setEnabled(nameValid);
    }

    void NewDialog::init_() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Name field
        QHBoxLayout *nameLayout = new QHBoxLayout();
        nameLabel_ = new QLabel(this);
        nameEdit_ = new QLineEdit(this);
        nameLayout->addWidget(nameLabel_);
        nameLayout->addWidget(nameEdit_);
        mainLayout->addLayout(nameLayout);

        // Reference currency field
        QHBoxLayout *currencyLayout = new QHBoxLayout();
        currencyLabel_ = new QLabel(this);
        currencyComboBox_ = new QComboBox(this);
        for (exchange::Ticket ticket : exchange::CurrencyTickets) {
            currencyComboBox_->addItem(exchange::getTicketName(ticket).c_str(), QVariant::fromValue(ticket));
        }
        currencyComboBox_->setCurrentIndex(0);
        currencyLayout->addWidget(currencyLabel_);
        currencyLayout->addWidget(currencyComboBox_);
        mainLayout->addLayout(currencyLayout);

        // Capital field
        QHBoxLayout *capitalLayout = new QHBoxLayout();
        capitalLabel_ = new QLabel(this);
        capitalSpinBox_ = new QDoubleSpinBox(this);
        capitalSpinBox_->setAlignment(Qt::AlignRight);
        capitalSpinBox_->setSingleStep(100);
        capitalSpinBox_->setDecimals(0);
        capitalSpinBox_->setRange(100, 1000000); // Set appropriate range
        capitalSpinBox_->setSuffix(QString(" ") + exchange::getTicketSymbol(exchange::Ticket::EUR).c_str());
        capitalLayout->addWidget(capitalLabel_);
        capitalLayout->addWidget(capitalSpinBox_);
        mainLayout->addLayout(capitalLayout);

        // Create the buttons
        createButton_ = new QPushButton(this);
        cancelButton_ = new QPushButton(this);
        
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(createButton_);
        buttonLayout->addWidget(cancelButton_);
        mainLayout->addLayout(buttonLayout);

        connect(currencyComboBox_, &QComboBox::currentIndexChanged, this, &NewDialog::onCurrencyComboBoxChanged_);
        connect(nameEdit_, &QLineEdit::textChanged, this, &NewDialog::validateInput_);
        connect(createButton_, &QPushButton::clicked, this, &NewDialog::onCreateButtonClicked);
        connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);

        setTranslatableText_();
    }

    void NewDialog::resetInputFields_() {
        nameEdit_->clear();
        nameEdit_->setToolTip("");
        currencyComboBox_->setCurrentIndex(0);
        capitalSpinBox_->setValue(0);
    }

    void NewDialog::setTranslatableText_() {
        setWindowTitle(tr("create-portfolio-dialog-title"));
        nameLabel_->setText(tr("portfolio-name-label"));
        currencyLabel_->setText(tr("reference-currency-label"));
        capitalLabel_->setText(tr("initial-capital-label"));
        createButton_->setText(tr("create-button-text"));
        cancelButton_->setText(tr("cancel-button-text"));
    }

} // namespace hmi

