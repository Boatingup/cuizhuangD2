#ifndef AUTO_FOLLOW_CONFIG_H
#define AUTO_FOLLOW_CONFIG_H

#include <QJsonObject>
#include <QStringList>
#include <QMutex>

#include "parameters.h"

#define AUTO_FOLLOW_CONFIG_PATH HMI_FILES_PATH_BASE "/auto_follow_config"
#define FOLLOW_ACTION_MAP_PATH  HMI_FILES_PATH_BASE "/follow_action_map.json"
#define FOLLOW_ACTION_MAP_QRC   ":/config/follow_action_map.json"

/**
 * Persist/load follow_config from MQTT topic parameter_extern_v2.
 * File format: JSON (same structure as MQTT payload's follow_config object).
 * UI labels via descriptor(); Stages fields shown as "段序号+中文名".
 */
class AutoFollowConfig
{
private:
    AutoFollowConfig();

public:
    static AutoFollowConfig* getInstance();

    /** Overwrite file with follow_config object; keep in-memory cache. */
    bool saveFollowConfig(const QJsonObject &followConfig);

    /** Load from disk into cache; return false if file missing/invalid. */
    bool loadFromFile();

    bool hasConfig() const;
    QJsonObject followConfig() const;

    /**
     * Build "Name:Value" rows for leftPane dual-column display.
     * Top-level: Chinese name; Stages: "段序号+中文名".
     */
    QStringList buildDisplayRows() const;

    /** Field key -> Chinese display name. */
    static QString descriptor(const QString &key);

private:
    void loadValueMap();
    QString jsonValueText(const QJsonValue &v) const;
    QString translateValue(const QString &fieldKey, const QJsonValue &v) const;

private:
    static AutoFollowConfig *instance;
    mutable QMutex mutex;
    QJsonObject m_followConfig;
    QJsonObject m_valueMap;
    bool m_loaded;
};

#endif // AUTO_FOLLOW_CONFIG_H
