#ifndef HMI_NEWDIALOG_H
#define HMI_NEWDIALOG_H


// Qt headers
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>

// headers
#include "Types.h"

// exchange headers


namespace hmi {

    // Create an input dialog to create a new portfolio. The dialog must have the following fields:
    // Name: the name of the portfolio
    // referenceCurrency: the reference currency of the portfolio
    // capital: the initial capital of the portfolio
    // The dialog must have a button to create the portfolio
    

    class NewDialog : public QDialog {
        Q_OBJECT

    public:
        explicit NewDialog(QWidget *parent = nullptr);
    
    public:
        void retranslateUi() {
            setTranslatableText_();
        }

        void show() {
            resetInputFields_();
            QDialog::show();
        }

    signals:
        void createPortfolio(const QString& name, exchange::Ticket referenceCurrency, double capital);

    private slots:
        void onCreateButtonClicked();
        void onCurrencyComboBoxChanged_(int index);
        void validateInput_();

    private:
        void init_();
        void resetInputFields_();
        void setTranslatableText_();
        
        
    private:
        QLabel* nameLabel_;
        QLineEdit* nameEdit_;
        QLabel* currencyLabel_;
        QComboBox* currencyComboBox_;
        QLabel* capitalLabel_;
        QDoubleSpinBox* capitalSpinBox_;
        QPushButton* createButton_;
        QPushButton* cancelButton_;
    };

} // namespace hmi

#endif // HMI_NEWDIALOG_H
