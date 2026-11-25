#include "can_param_sync.h"
#include "can_bus_thread.h"
#include "para_synchronize_thread.h"
#include "../Resources/address_set.h"
#include "tlog/tlog.h"

#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QtGlobal>
#include <cstring>

CanParamSync *CanParamSync::s_instance = nullptr;
bool CanParamSync::s_useCanTransport = true;

CanParamSync *CanParamSync::instance()
{
    if (s_instance == nullptr) {
        s_instance = new CanParamSync();
    }
    return s_instance;
}

CanParamSync::CanParamSync(QObject *parent)
    : QObject(parent)
{
}

bool CanParamSync::useCanTransport()
{
    return s_useCanTransport;
}

void CanParamSync::setUseCanTransport(bool enabled)
{
    s_useCanTransport = enabled;
}

bool CanParamSync::init()
{
#ifdef RUN_ON_ARM
    CanBusThread *bus = CanBusThread::instance();
    if (!bus->init()) {
        TLOG_WARN("CAN param sync init failed, fallback to MQTT");
        s_useCanTransport = false;
        return false;
    }

    connect(bus, &CanBusThread::frameReceived,
            this, &CanParamSync::onFrameReceived, Qt::QueuedConnection);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CanParamSync::onReceiveTimeout);
    timer->start(5000);

    TLOG_INFO("CAN param sync ready");
    return true;
#else
    s_useCanTransport = false;
    return false;
#endif
}

void CanParamSync::shutdown()
{
    CanBusThread::instance()->shutdown();
}

uint8_t CanParamSync::frameChecksum(const uint8_t *data)
{
    uint32_t sum = 0;
    for (int i = 0; i < 7; ++i) {
        sum += data[i];
    }
    return static_cast<uint8_t>(sum & 0xFF);
}

uint16_t CanParamSync::payloadCrc16(const QByteArray &payload)
{
    return static_cast<uint16_t>(qChecksum(payload.constData(), static_cast<uint>(payload.size())) & 0xFFFF);
}

uint32_t CanParamSync::frameBaseId(uint32_t msgId, uint8_t localId)
{
    return HMI_CAN_EFF_FLAG | msgId | (localId & 0xFF);
}

uint32_t CanParamSync::sessionKey(uint8_t srcId, uint16_t sessionId)
{
    return (static_cast<uint32_t>(srcId) << 16) | sessionId;
}

#ifdef RUN_ON_ARM
bool CanParamSync::sendRawFrame(uint32_t msgId, const uint8_t data[8])
{
    struct can_frame frame;
    std::memset(&frame, 0, sizeof(frame));
    frame.can_id = msgId;
    frame.can_dlc = 8;
    std::memcpy(frame.data, data, 8);
    return CanBusThread::instance()->sendFrame(frame);
}
#endif

bool CanParamSync::sendPayload(const QByteArray &payload, uint8_t dstMin, uint8_t dstMax)
{
#ifdef RUN_ON_ARM
    if (!CanBusThread::instance()->isReady()) {
        return false;
    }
    if (payload.isEmpty() || payload.size() > HMI_PARAM_SYNC_MAX_PAYLOAD) {
        TLOG_ERROR("CAN param sync invalid payload size: %d", payload.size());
        return false;
    }

    AddressSet *addr = AddressSet::getInstance();
    uint8_t localId = addr->get_local_address();
    uint16_t sessionId = m_nextSessionId++;
    if (m_nextSessionId == 0) {
        m_nextSessionId = 1;
    }

    uint8_t startData[8] = {0};
    startData[0] = static_cast<uint8_t>(sessionId & 0xFF);
    startData[1] = static_cast<uint8_t>((sessionId >> 8) & 0xFF);
    startData[2] = static_cast<uint8_t>(payload.size() & 0xFF);
    startData[3] = static_cast<uint8_t>((payload.size() >> 8) & 0xFF);
    startData[4] = dstMin;
    startData[5] = dstMax;
    startData[6] = localId;
    startData[7] = frameChecksum(startData);
    if (!sendRawFrame(frameBaseId(HMI_PARAM_SYNC_START_ID, localId), startData)) {
        return false;
    }

    uint16_t seq = 0;
    for (int offset = 0; offset < payload.size(); offset += HMI_PARAM_SYNC_DATA_BYTES) {
        uint8_t dataFrame[8] = {0};
        dataFrame[0] = static_cast<uint8_t>(sessionId & 0xFF);
        dataFrame[1] = static_cast<uint8_t>((sessionId >> 8) & 0xFF);
        dataFrame[2] = static_cast<uint8_t>(seq & 0xFF);
        dataFrame[3] = static_cast<uint8_t>((seq >> 8) & 0xFF);

        for (int i = 0; i < HMI_PARAM_SYNC_DATA_BYTES; ++i) {
            int idx = offset + i;
            if (idx < payload.size()) {
                dataFrame[4 + i] = static_cast<uint8_t>(payload.at(idx));
            }
        }
        dataFrame[7] = frameChecksum(dataFrame);
        if (!sendRawFrame(frameBaseId(HMI_PARAM_SYNC_DATA_ID, localId), dataFrame)) {
            return false;
        }
        usleep(1000);
        ++seq;
    }

    uint16_t crc = payloadCrc16(payload);
    uint8_t endData[8] = {0};
    endData[0] = static_cast<uint8_t>(sessionId & 0xFF);
    endData[1] = static_cast<uint8_t>((sessionId >> 8) & 0xFF);
    endData[2] = static_cast<uint8_t>(crc & 0xFF);
    endData[3] = static_cast<uint8_t>((crc >> 8) & 0xFF);
    endData[4] = static_cast<uint8_t>((seq >> 8) & 0xFF);
    endData[5] = static_cast<uint8_t>(seq & 0xFF);
    endData[7] = frameChecksum(endData);
    if (!sendRawFrame(frameBaseId(HMI_PARAM_SYNC_END_ID, localId), endData)) {
        return false;
    }

    TLOG_INFO("CAN param sync sent session=%u bytes=%d range=[%u,%u]",
              sessionId, payload.size(), dstMin, dstMax);
    return true;
#else
    Q_UNUSED(payload);
    Q_UNUSED(dstMin);
    Q_UNUSED(dstMax);
    return false;
#endif
}

void CanParamSync::onFrameReceived(const QByteArray &rawFrame)
{
#ifndef RUN_ON_ARM
    Q_UNUSED(rawFrame);
    return;
#else
    if (rawFrame.size() != static_cast<int>(sizeof(struct can_frame))) {
        return;
    }

    struct can_frame frame;
    std::memcpy(&frame, rawFrame.constData(), sizeof(frame));

    if (!(frame.can_id & HMI_CAN_EFF_FLAG)) {
        return;
    }

    const uint32_t extId = frame.can_id & CAN_EFF_MASK;
    const uint32_t baseId = extId & 0x1FFFF000UL;
    const uint8_t srcId = static_cast<uint8_t>(extId & 0xFFU);
    const uint8_t *data = frame.data;

    if (frameChecksum(data) != data[7]) {
        TLOG_WARN("CAN param sync checksum error from %u", srcId);
        return;
    }

    AddressSet *addr = AddressSet::getInstance();
    uint8_t localId = addr->get_local_address();
    if (srcId == localId) {
        return;
    }

    switch (baseId) {
    case HMI_PARAM_SYNC_START_ID:
        handleStartFrame(srcId, data);
        break;
    case HMI_PARAM_SYNC_DATA_ID:
        handleDataFrame(srcId, data);
        break;
    case HMI_PARAM_SYNC_END_ID:
        handleEndFrame(srcId, data);
        break;
    default:
        TLOG_WARN("CAN param sync unknown baseId=0x%X from=%u", baseId, srcId);
        break;
    }
#endif
}

#ifdef RUN_ON_ARM
void CanParamSync::handleStartFrame(uint8_t srcId, const uint8_t data[8])
{
    uint16_t sessionId = static_cast<uint16_t>(data[0] | (data[1] << 8));
    uint16_t totalLen = static_cast<uint16_t>(data[2] | (data[3] << 8));
    uint8_t dstMin = data[4];
    uint8_t dstMax = data[5];

    AddressSet *addr = AddressSet::getInstance();
    uint8_t localId = addr->get_local_address();
    if (localId < dstMin || localId > dstMax) {
        TLOG_WARN("CAN param sync START ignored local=%u range=[%u,%u] from=%u",
                  localId, dstMin, dstMax, srcId);
        return;
    }
    if (totalLen == 0 || totalLen > HMI_PARAM_SYNC_MAX_PAYLOAD) {
        return;
    }

    ReceiveSession session;
    session.sessionId = sessionId;
    session.totalLen = totalLen;
    session.dstMin = dstMin;
    session.dstMax = dstMax;
    session.srcId = srcId;
    session.buffer.clear();
    session.buffer.reserve(totalLen);
    session.nextSeq = 0;
    m_sessions.insert(sessionKey(srcId, sessionId), session);
    TLOG_INFO("CAN param sync recv START from %u session=%u len=%u range=[%u,%u] local=%u",
              srcId, sessionId, totalLen, dstMin, dstMax, localId);
}

void CanParamSync::handleDataFrame(uint8_t srcId, const uint8_t data[8])
{
    uint16_t sessionId = static_cast<uint16_t>(data[0] | (data[1] << 8));
    uint16_t seq = static_cast<uint16_t>(data[2] | (data[3] << 8));

    auto it = m_sessions.find(sessionKey(srcId, sessionId));
    if (it == m_sessions.end()) {
        return;
    }

    ReceiveSession &session = it.value();
    if (seq != session.nextSeq) {
        TLOG_WARN("CAN param sync seq mismatch session=%u expect=%u got=%u",
                  sessionId, session.nextSeq, seq);
        m_sessions.remove(sessionKey(srcId, sessionId));
        return;
    }

    for (int i = 0; i < HMI_PARAM_SYNC_DATA_BYTES; ++i) {
        if (session.buffer.size() >= session.totalLen) {
            break;
        }
        session.buffer.append(static_cast<char>(data[4 + i]));
    }
    ++session.nextSeq;
}

void CanParamSync::handleEndFrame(uint8_t srcId, const uint8_t data[8])
{
    uint16_t sessionId = static_cast<uint16_t>(data[0] | (data[1] << 8));
    uint16_t crc = static_cast<uint16_t>(data[2] | (data[3] << 8));

    auto it = m_sessions.find(sessionKey(srcId, sessionId));
    if (it == m_sessions.end()) {
        return;
    }

    ReceiveSession session = it.value();
    m_sessions.remove(sessionKey(srcId, sessionId));

    if (session.buffer.size() != session.totalLen) {
        TLOG_WARN("CAN param sync incomplete session=%u", sessionId);
        return;
    }
    if (payloadCrc16(session.buffer) != crc) {
        TLOG_WARN("CAN param sync crc error session=%u", sessionId);
        return;
    }

    dispatchPayload(session.buffer);
    TLOG_INFO("CAN param sync recv END from %u session=%u applied", srcId, sessionId);
}
#endif

void CanParamSync::onReceiveTimeout()
{
    if (m_sessions.isEmpty()) {
        return;
    }
    m_sessions.clear();
}

void CanParamSync::dispatchPayload(const QByteArray &payload)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        TLOG_ERROR("CAN param sync invalid json: %s", error.errorString().toUtf8().constData());
        return;
    }

    QJsonObject rootObj = doc.object();
    QJsonValue paramVal = rootObj.value(QStringLiteral("param"));

    ParaSynchronizeThread *pst = ParaSynchronizeThread::getInstance();
    if (paramVal.isObject()) {
        pst->parse_msg1(payload);
        return;
    }

    if (paramVal.isString()) {
        QJsonObject legacyObj;
        legacyObj.insert(QStringLiteral("param"), paramVal.toString());
        if (rootObj.contains(QStringLiteral("support_type"))) {
            legacyObj.insert(QStringLiteral("support_type"), rootObj.value(QStringLiteral("support_type")));
        }
        pst->parse_msg(QJsonDocument(legacyObj).toJson(QJsonDocument::Compact));
    }
}
