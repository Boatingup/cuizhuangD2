#include "auto_follow_config.h"

#include <QFile>
#include <QDir>
#include <QMap>
#include <QJsonDocument>
#include <QJsonArray>
#include <unistd.h>
#include <errno.h>

#include "tlog/tlog.h"

AutoFollowConfig* AutoFollowConfig::instance = nullptr;

AutoFollowConfig::AutoFollowConfig()
    : m_loaded(false)
{
    loadValueMap();
    loadFromFile();
}

AutoFollowConfig* AutoFollowConfig::getInstance()
{
    if (instance == nullptr) {
        instance = new AutoFollowConfig();
    }
    return instance;
}

QString AutoFollowConfig::descriptor(const QString &key)
{
    static QMap<QString, QString> map;
    if (map.isEmpty()) {
        map.insert("Type", QString::fromUtf8("模式类型"));
        map.insert("Direction", QString::fromUtf8("运行方向"));
        map.insert("Length", QString::fromUtf8("煤机身长(架)"));
        map.insert("InfraredFilter", QString::fromUtf8("红外防抖架数"));
        map.insert("StopTime", QString::fromUtf8("停止时长(秒)"));
        map.insert("SortNum", QString::fromUtf8("段序号"));
        map.insert("StartSupport", QString::fromUtf8("起始支架"));
        map.insert("EndSupport", QString::fromUtf8("结束支架"));
        map.insert("ActionSortNum", QString::fromUtf8("动作排序号"));
        map.insert("ActionNum", QString::fromUtf8("动作编号"));
        map.insert("TriggerPoint", QString::fromUtf8("触发门槛架号"));
        map.insert("CtrlDistance", QString::fromUtf8("控制距离"));
        map.insert("CtrlRange", QString::fromUtf8("连续动作架数"));
        map.insert("CtrlDirection", QString::fromUtf8("动作执行方向"));
        map.insert("RangeMin", QString::fromUtf8("有效架号下限"));
        map.insert("RangeMax", QString::fromUtf8("有效架号上限"));
        map.insert("NoData", QString::fromUtf8("无数据"));
    }
    return map.value(key, key);
}

void AutoFollowConfig::loadValueMap()
{
    QByteArray data;
    QFile diskFile(QString(FOLLOW_ACTION_MAP_PATH));
    if (diskFile.exists() && diskFile.open(QFile::ReadOnly)) {
        data = diskFile.readAll();
        diskFile.close();
    } else {
        QFile qrcFile(QString(FOLLOW_ACTION_MAP_QRC));
        if (qrcFile.open(QFile::ReadOnly)) {
            data = qrcFile.readAll();
            qrcFile.close();
        }
    }

    if (data.isEmpty()) {
        TLOG_WARN("follow_action_map: no map file loaded");
        m_valueMap = QJsonObject();
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        TLOG_WARN("follow_action_map: parse error %s",
                  err.errorString().toUtf8().data());
        m_valueMap = QJsonObject();
        return;
    }

    m_valueMap = doc.object();
    TLOG_INFO("follow_action_map: loaded ActionNum=%d Type=%d Direction=%d CtrlDirection=%d",
              m_valueMap.value("ActionNum").toObject().size(),
              m_valueMap.value("Type").toObject().size(),
              m_valueMap.value("Direction").toObject().size(),
              m_valueMap.value("CtrlDirection").toObject().size());
}

QString AutoFollowConfig::jsonValueText(const QJsonValue &v) const
{
    if (v.isDouble())
        return QString::number(static_cast<qint64>(v.toDouble()));
    if (v.isString())
        return v.toString();
    if (v.isBool())
        return v.toBool() ? "1" : "0";
    return v.toVariant().toString();
}

QString AutoFollowConfig::translateValue(const QString &fieldKey, const QJsonValue &v) const
{
    QString numText = jsonValueText(v);
    QJsonObject group = m_valueMap.value(fieldKey).toObject();
    if (group.contains(numText)) {
        QString name = group.value(numText).toString();
        if (!name.isEmpty())
            return name;
    }
    return numText;
}

bool AutoFollowConfig::saveFollowConfig(const QJsonObject &followConfig)
{
    if (followConfig.isEmpty()) {
        TLOG_WARN("auto_follow_config: empty follow_config, skip save");
        return false;
    }

    QDir dir(HMI_FILES_PATH_BASE);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            TLOG_WARN("auto_follow_config: mkdir %s failed", HMI_FILES_PATH_BASE);
            return false;
        }
    }

    QString path = QString(AUTO_FOLLOW_CONFIG_PATH);
    QString path_back = path + "_back";

    if (access(path_back.toUtf8().data(), F_OK) == 0) {
        unlink(path_back.toUtf8().data());
    }
    if (access(path.toUtf8().data(), F_OK) == 0) {
        QFile::copy(path, path_back);
    }

    QJsonDocument doc(followConfig);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    QFile file(path);
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        TLOG_WARN("auto_follow_config: open %s failed errno:%d",
                  path.toUtf8().data(), errno);
        return false;
    }
    qint64 written = file.write(data);
    file.flush();
    if (file.handle() >= 0) {
        fsync(file.handle());
    }
    file.close();

    if (written != data.size()) {
        TLOG_WARN("auto_follow_config: write incomplete");
        if (access(path_back.toUtf8().data(), F_OK) == 0) {
            unlink(path.toUtf8().data());
            QFile::copy(path_back, path);
        }
        return false;
    }

    system("sync");

    {
        QMutexLocker locker(&mutex);
        m_followConfig = followConfig;
        m_loaded = true;
    }

    TLOG_INFO("auto_follow_config: saved to %s, stages=%d",
              path.toUtf8().data(),
              followConfig.value("Stages").toArray().size());
    return true;
}

bool AutoFollowConfig::loadFromFile()
{
    QString path = QString(AUTO_FOLLOW_CONFIG_PATH);
    QFile file(path);
    if (!file.exists()) {
        TLOG_INFO("auto_follow_config: file not found %s", path.toUtf8().data());
        QMutexLocker locker(&mutex);
        m_followConfig = QJsonObject();
        m_loaded = false;
        return false;
    }

    if (!file.open(QFile::ReadOnly)) {
        TLOG_WARN("auto_follow_config: open %s failed", path.toUtf8().data());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        TLOG_WARN("auto_follow_config: parse error %s",
                  err.errorString().toUtf8().data());
        return false;
    }

    QMutexLocker locker(&mutex);
    m_followConfig = doc.object();
    m_loaded = true;
    TLOG_INFO("auto_follow_config: loaded stages=%d",
              m_followConfig.value("Stages").toArray().size());
    return true;
}

bool AutoFollowConfig::hasConfig() const
{
    QMutexLocker locker(&mutex);
    return m_loaded && !m_followConfig.isEmpty();
}

QJsonObject AutoFollowConfig::followConfig() const
{
    QMutexLocker locker(&mutex);
    return m_followConfig;
}

QStringList AutoFollowConfig::buildDisplayRows() const
{
    QStringList rows;
    QJsonObject cfg;
    {
        QMutexLocker locker(&mutex);
        if (!m_loaded || m_followConfig.isEmpty()) {
            rows << descriptor("NoData") + QString(":--");
            return rows;
        }
        cfg = m_followConfig;
    }

    auto appendScalar = [&](const QString &key) {
        if (!cfg.contains(key))
            return;
        QJsonValue v = cfg.value(key);
        QString text;
        if (key == "Type" || key == "Direction")
            text = translateValue(key, v);
        else
            text = jsonValueText(v);
        rows << descriptor(key) + QString(":") + text;
    };

    appendScalar("Type");
    appendScalar("Direction");
    appendScalar("Length");
    appendScalar("InfraredFilter");
    appendScalar("StopTime");

    // Stages: display as "段序号+中文名:值", e.g. "1起始支架:6"
    auto appendStageField = [&](int stageSort, const QString &key, const QJsonValue &v) {
        QString text;
        if (key == "ActionNum" || key == "CtrlDirection")
            text = translateValue(key, v);
        else
            text = jsonValueText(v);
        rows << QString::number(stageSort) + descriptor(key) + QString(":") + text;
    };

    QJsonArray stages = cfg.value("Stages").toArray();
    for (int i = 0; i < stages.size(); ++i) {
        QJsonObject stage = stages.at(i).toObject();
        int sortNum = stage.value("SortNum").toInt(i + 1);

        if (stage.contains("SortNum"))
            appendStageField(sortNum, "SortNum", stage.value("SortNum"));
        if (stage.contains("StartSupport"))
            appendStageField(sortNum, "StartSupport", stage.value("StartSupport"));
        if (stage.contains("EndSupport"))
            appendStageField(sortNum, "EndSupport", stage.value("EndSupport"));

        QJsonArray actions = stage.value("Actions").toArray();
        for (int j = 0; j < actions.size(); ++j) {
            QJsonObject act = actions.at(j).toObject();
            if (act.contains("SortNum"))
                appendStageField(sortNum, "ActionSortNum", act.value("SortNum"));
            if (act.contains("ActionNum"))
                appendStageField(sortNum, "ActionNum", act.value("ActionNum"));
            if (act.contains("TriggerPoint"))
                appendStageField(sortNum, "TriggerPoint", act.value("TriggerPoint"));
            if (act.contains("CtrlDistance"))
                appendStageField(sortNum, "CtrlDistance", act.value("CtrlDistance"));
            if (act.contains("CtrlRange"))
                appendStageField(sortNum, "CtrlRange", act.value("CtrlRange"));
            if (act.contains("CtrlDirection"))
                appendStageField(sortNum, "CtrlDirection", act.value("CtrlDirection"));
            if (act.contains("RangeMin"))
                appendStageField(sortNum, "RangeMin", act.value("RangeMin"));
            if (act.contains("RangeMax"))
                appendStageField(sortNum, "RangeMax", act.value("RangeMax"));
        }
    }

    if (rows.isEmpty()) {
        rows << descriptor("NoData") + QString(":--");
    }
    return rows;
}
