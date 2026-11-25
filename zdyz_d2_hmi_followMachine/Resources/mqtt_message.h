#ifndef MQTT_MESSAGE_H
#define MQTT_MESSAGE_H


#define MQTT_ADDRESS "0.0.0.0"
//#define MQTT_ADDRESS "10.12.132.6"

#define MQTT_PORT 1883

#define MQTT_WILL_TOPIC "/client/hmi/online"
#define TOPIC_KEY "key"
#define TOPIC_SENSOR "sensor"
#define TOPIC_STATE "state"
#define TOPIC_ACTION "action"
#define TOPIC_SELECT "select"
#define TOPIC_STOP "stop"
#define TOPIC_LOCK "lock"
#define TOPIC_CONFIG "config"
#define TOPIC_FALUT "fault"
#define TOPIC_UPDATE "update"
#define TOPIC_PARAMETER_SET "parameter_set"
#define TOPIC_PARAMETER_EXTERN_SET "parameter_extern"
#define TOPIC_PARAMETER_EXTERN_V2 "parameter_extern_v2"
#define TOPIC_SCREEN "screen"
#define TOPIC_REBOOT "system_reboot"
#define TOPIC_BROAD_CAST "broad_cast"
#define TOPIC_PORT_STATUS "portstatus"
#define TOPIC_MQTT_PROXY_STATUS "mqtt_proxy_status"
#define TOPIC_WILL_MSG "will_msg"

#define TOPIC_EXTERN_MSG "extern_msg"

#define MSG_PARAM_CATEGORY "category"
#define MSG_PARAM_SUBBLOCK "subblock"
#define MSG_PARAM_ELEMENT "element"
#define MSG_PARAM_VALUE "value"
#define MSG_PARAM_BASIC "basic"






















#endif // MQTT_MESSAGE_H
