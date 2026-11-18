#ifndef CAN_BUS_THREAD_H
#define CAN_BUS_THREAD_H

#include <QThread>
#include <QMutex>
#include <QByteArray>

#ifdef RUN_ON_ARM
#include <linux/can.h>
#include <linux/can/raw.h>
#endif

class CanBusThread : public QThread
{
    Q_OBJECT
public:
    static CanBusThread *instance();

    bool init();
    void shutdown();
    bool isReady() const { return m_ready; }

#ifdef RUN_ON_ARM
    bool sendFrame(const struct can_frame &frame);
#endif

signals:
    void frameReceived(const QByteArray &rawFrame);

protected:
    void run() override;

private:
    CanBusThread();

    static CanBusThread *s_instance;
    int m_fd{-1};
    bool m_ready{false};
    bool m_stop{false};
    QMutex m_writeMutex;
};

#endif // CAN_BUS_THREAD_H
