#ifndef HMI_SYSTEMLOG_H
#define HMI_SYSTEMLOG_H

#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtGui/QColor>


namespace hmi {

    // Create a class named SystemLog that inherits from QTableWidget to store traces. It will have the following columns:
    // Date: the date of the trace
    // The type of trace: information, warning, etc
    // The text of the trace
    class SystemLog : public QTableWidget
    {
        Q_OBJECT

    public:
        SystemLog(QWidget *parent = nullptr);

        void info(const QString &text);
        void warning(const QString &text);
        void error(const QString &text);
        void trace(const QString &text);

        void retranslateUi() {}

    private:
        // Define enums for the different types of traces
        enum class TraceType {
            Info,
            Warning,
            Error,
            Debug
        };
        QIcon getIcon_(TraceType type);
        QColor getColor_(TraceType type);
        QString toQString(TraceType type);
        void setTrace_(int row, TraceType type, const QString &text);
    };

} // namespace hmi

#endif // HMI_SYSTEMLOG_H
