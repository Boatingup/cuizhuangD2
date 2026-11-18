#ifndef CAN_PARAM_SYNC_H
#define CAN_PARAM_SYNC_H

#include <QObject>
#include <QByteArray>
#include <QHash>
#include <cstdint>

#ifdef RUN_ON_ARM
#include <linux/can.h>
#endif

class CanParamSync : public QObject
{
    Q_OBJECT
public:
    static CanParamSync *instance();

    bool init();
    void shutdown();

    static bool useCanTransport();
    static void setUseCanTransport(bool enabled);

    bool sendPayload(const QByteArray &payload, uint8_t dstMin, uint8_t dstMax);

public slots:
    void onFrameReceived(const QByteArray &rawFrame);

private slots:
    void onReceiveTimeout();

private:
    explicit CanParamSync(QObject *parent = nullptr);

    struct ReceiveSession {
        uint16_t sessionId{0};
        uint16_t totalLen{0};
        uint8_t dstMin{0};
        uint8_t dstMax{0};
        uint8_t srcId{0};
        uint16_t expectedCrc{0};
        QByteArray buffer;
        uint16_t nextSeq{0};
    };

    static uint8_t frameChecksum(const uint8_t *data);
    static uint16_t payloadCrc16(const QByteArray &payload);
    static uint32_t frameBaseId(uint32_t msgId, uint8_t localId);
    static uint32_t sessionKey(uint8_t srcId, uint16_t sessionId);

#ifdef RUN_ON_ARM
    bool sendRawFrame(uint32_t msgId, const uint8_t data[8]);
    void handleStartFrame(uint8_t srcId, const uint8_t data[8]);
    void handleDataFrame(uint8_t srcId, const uint8_t data[8]);
    void handleEndFrame(uint8_t srcId, const uint8_t data[8]);
#endif
    void dispatchPayload(const QByteArray &payload);

    static CanParamSync *s_instance;
    static bool s_useCanTransport;

    QHash<uint32_t, ReceiveSession> m_sessions;
    uint16_t m_nextSessionId{1};
};

// HMI 跨架参数同步专用 CAN ID（ACS 当前未处理该段）
#define HMI_CAN_EFF_FLAG            0x80000000U
#define HMI_PARAM_SYNC_START_ID     0x10E00000UL
#define HMI_PARAM_SYNC_DATA_ID      0x10E01000UL
#define HMI_PARAM_SYNC_END_ID       0x10E02000UL

#define HMI_PARAM_SYNC_MAX_PAYLOAD  8192
#define HMI_PARAM_SYNC_DATA_BYTES   3

#endif // CAN_PARAM_SYNC_H
