#include "can_bus_thread.h"
#include "can_param_sync.h"
#include "tlog/tlog.h"

#ifdef RUN_ON_ARM
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#endif

CanBusThread *CanBusThread::s_instance = nullptr;

CanBusThread *CanBusThread::instance()
{
    if (s_instance == nullptr) {
        s_instance = new CanBusThread();
    }
    return s_instance;
}

CanBusThread::CanBusThread()
{
}

bool CanBusThread::init()
{
#ifdef RUN_ON_ARM
    if (m_ready) {
        return true;
    }

    int fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (fd < 0) {
        TLOG_ERROR("can socket create failed: %s", strerror(errno));
        return false;
    }

    struct ifreq ifr;
    std::memset(&ifr, 0, sizeof(ifr));
    std::strncpy(ifr.ifr_name, "can0", IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        TLOG_ERROR("can ioctl failed: %s", strerror(errno));
        close(fd);
        return false;
    }

    struct sockaddr_can addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
        TLOG_ERROR("can bind failed: %s", strerror(errno));
        close(fd);
        return false;
    }

    struct can_filter filters[3];
    const canid_t idMask = CAN_EFF_MASK & ~0xFFU;
    filters[0].can_id = HMI_CAN_EFF_FLAG | HMI_PARAM_SYNC_START_ID;
    filters[0].can_mask = idMask;
    filters[1].can_id = HMI_CAN_EFF_FLAG | HMI_PARAM_SYNC_DATA_ID;
    filters[1].can_mask = idMask;
    filters[2].can_id = HMI_CAN_EFF_FLAG | HMI_PARAM_SYNC_END_ID;
    filters[2].can_mask = idMask;
    setsockopt(fd, SOL_CAN_RAW, CAN_RAW_FILTER, filters, sizeof(filters));

    int sndbuf = 65536;
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

    m_fd = fd;
    m_stop = false;
    m_ready = true;
    start();
    TLOG_INFO("HMI can0 thread started");
    return true;
#else
    TLOG_WARN("CAN bus thread disabled on non-ARM build");
    return false;
#endif
}

void CanBusThread::shutdown()
{
#ifdef RUN_ON_ARM
    m_stop = true;
    if (isRunning()) {
        wait(3000);
    }
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
    m_ready = false;
#endif
}

#ifdef RUN_ON_ARM
bool CanBusThread::sendFrame(const struct can_frame &frame)
{
    if (!m_ready || m_fd < 0) {
        return false;
    }

    QMutexLocker locker(&m_writeMutex);
    for (int attempt = 0; attempt < 5; ++attempt) {
        ssize_t ret = write(m_fd, &frame, sizeof(frame));
        if (ret == static_cast<ssize_t>(sizeof(frame))) {
            return true;
        }
        if (errno == ENOBUFS) {
            usleep(2000);
            continue;
        }
        TLOG_ERROR("can write failed: %s", strerror(errno));
        return false;
    }
    TLOG_ERROR("can write failed: No buffer space available");
    return false;
}
#endif

void CanBusThread::run()
{
#ifdef RUN_ON_ARM
    struct can_frame frame;
    while (!m_stop) {
        ssize_t nbytes = read(m_fd, &frame, sizeof(frame));
        if (nbytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (m_stop) {
                break;
            }
            TLOG_ERROR("can read failed: %s", strerror(errno));
            msleep(10);
            continue;
        }
        if (nbytes == static_cast<ssize_t>(sizeof(frame))) {
            emit frameReceived(QByteArray(reinterpret_cast<const char *>(&frame),
                                          static_cast<int>(sizeof(frame))));
        }
    }
#endif
}
