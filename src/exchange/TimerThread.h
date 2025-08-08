#ifndef EXCHANGE_TIMERTHREAD_H
#define EXCHANGE_TIMERTHREAD_H


// Stl  headers

// Qt headers
#include <QObject>
#include <QThread>
#include <QEventLoop>
#include <QTimer>



namespace exchange {


    class TimerThread : public QThread {

        Q_OBJECT

    public:
        TimerThread(unsigned int period, QObject *parent = nullptr) : QThread(parent), period_(period), exitRequested_(false) {
            timer_.moveToThread(this);
        }

        QTimer* timer() {
            return &timer_;
        }

    public:
        void requestExit(bool force = false) {
            exitRequested_ = true;
            if(force) {
                exit(0);
            }
        }

    protected:
        void run() override {
            QEventLoop eventLoop;
            connect(&timer_, &QTimer::timeout, [&]() {
                if (exitRequested_) {
                    eventLoop.quit();
                }
            });
            timer_.start(period_); // Trigger every second
            // Start the event loop
            eventLoop.exec();
        }

    private:
        unsigned int period_;
        bool exitRequested_;
        QTimer timer_;
    };


} // namespace exchange

#endif // EXCHANGE_TIMERTHREAD_H
