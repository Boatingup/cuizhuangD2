#-------------------------------------------------
#
# Project created by QtCreator 2021-08-25T10:03:05
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG += exceptions
#CONFIG += debug
#DEFINES += OPEN_ALL_DEBUG
#DEFINES += OPEN_DEBUG

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

#QMAKE_CXXFLAGS_WARN_ON += -Wno-error=unused-variable
#warning_filter += -isystem.*open62541\.c

QMAKE_CXXFLAGS += -Wno-deprecated-declarations
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CFLAGS += -Wno-deprecated-declarations
QMAKE_CFLAGS += -Wno-unused-parameter
QMAKE_CFLAGS += -Wno-unused-variable

# RUN_ON_WINDOWS  RUN_ON_LINUX  RUN_ON_ARM
DEFINES += RUN_ON_ARM
TARGET = HMI
TEMPLATE = app

target.path=/userdata/bin

INSTALLS += target

SOURCES += main.cpp\
    Resources/system_state.cpp \
    Resources/sensor.cpp \
    Resources/address_set.cpp \
    window/mainpage.cpp \
    Resources/parameters.cpp \
    Resources/auto_follow_config.cpp \
    parameter_config.cpp \
    Resources/command.cpp \
    stoplockpage.cpp \
    Resources/action_register.cpp \
    manual_address.cpp \
    widget_stack.cpp \
    net_phone_page.cpp \
    threads/para_synchronize_thread.cpp \
    threads/can_bus_thread.cpp \
    threads/can_param_sync.cpp \
    system_config_page.cpp \
    Resources/error_code.cpp \
    Resources/gui_setting.cpp \
    show_version.cpp \
    actionconfigure.cpp \
    action_set.cpp \
    threads/update_thread.cpp \
    dialog_timer_setup.cpp \
    Resources/mlabel.cpp \
    dialog_warrant.cpp \
    Resources/processlogger.cpp \
    dialog_usercheckandlogin.cpp \
    dialog_help.cpp \
    dialog_info.cpp \
    threads/comm_acs_socket_thread.cpp \
    update_config_page.cpp \
    auxiliarymenu.cpp \
    #threads/para_synchronize_file.cpp \
    config_backup_restore.cpp \
    syncparafile.cpp \
    fault_display.cpp \
    fault_diagnosis_diaplsy.cpp \
    #driver_chn_config.cpp \
    #driver_chn_config_child.cpp \
    #driver_chn_set.cpp \
    generic_config.cpp \
    port_visualization.cpp \
    controlled_page.cpp \
    window/menupage.cpp \
    window/leftpane.cpp \
    passwd_set.cpp \
    login_in.cpp \
    update_process.cpp \
    combination_switch.cpp \
    pumpstation_monitor.cpp \
    net_status_show.cpp

HEADERS  += main.h \
    Resources/mqtt_message.h \
    Resources/system_state.h \
    Resources/sensor.h \
    Resources/address_set.h \
    Resources/mytypes.h \
    Resources/key_value.h \
    Resources/configure.h \
    log_head.h \
    window/menupage.h \
    window/mainpage.h \
    window/leftpane.h \
    Resources/parameters.h \
    Resources/auto_follow_config.h \
    parameter_config.h \
    Resources/command.h \
    stoplockpage.h \
    Resources/action_register.h \
    manual_address.h \
    widget_stack.h \
    net_phone_page.h \
    threads/para_synchronize_thread.h \
    threads/can_bus_thread.h \
    threads/can_param_sync.h \
    system_config_page.h \
    Resources/error_code.h \
    Resources/guiconfig.h \
    Resources/gui_setting.h \
    show_version.h \
    actionconfigure.h \
    action_set.h \
    threads/update_thread.h \
    dialog_timer_setup.h \
    Resources/mlabel.h \
    dialog_warrant.h \
    Resources/processlogger.h \
    dialog_usercheckandlogin.h \
    dialog_help.h \
    dialog_info.h \
    main.h \
    threads/comm_acs_socket_thread.h \
    update_config_page.h \
    auxiliarymenu.h \
    #threads/para_synchronize_file.h \
    syncparafile.h \
    config_backup_restore.h \
    fault_display.h \
    Resources/dbg_print.h \
    fault_diagnosis_diaplsy.h \
    #driver_chn_config.h \
    #driver_chn_config_child.h \
    #driver_chn_set.h \
    generic_config.h \
    Resources/libssh2_sftp.h \
    Resources/libssh2.h \
    Resources/libssh2_publickey.h \
    port_visualization.h \
    controlled_page.h \
    passwd_set.h \
    login_in.h \
    update_process.h \
    combination_switch.h \
    pumpstation_monitor.h \
    net_status_show.h

FORMS    += window/mainpage.ui \
    window/menupage.ui \
    window/leftpane.ui \
    parameter_config.ui \
    stoplockpage.ui \
    manual_address.ui \
    net_phone_page.ui \
    system_config_page.ui \
    show_version.ui \
    actionconfigure.ui \
    action_set.ui \
    dialog_timer_setup.ui \
    dialog_warrant.ui \
    dialog_usercheckandlogin.ui \
    dialog_help.ui \
    dialog_info.ui \
    update_config_page.ui \
    auxiliarymenu.ui \
    syncparafile.ui \
    fault_display.ui \
    fault_diagnosis_diaplsy.ui \
    #driver_chn_config.ui \
    driver_chn_config_child.ui \
    driver_chn_set.ui \
    generic_config.ui \
    port_visualization.ui \
    controlled_page.ui \
    passwd_set.ui \
    login_in.ui \
    update_process.ui \
    combination_switch.ui \
    pumpstation_monitor.ui \
    net_status_show.ui

RESOURCES += \
    window/image.qrc \
    window/qss.qrc \
    window/resources.qrc \
    Resources/follow_action_map.qrc

OTHER_FILES += \
    window/menupage.qss \
    window/mainpage.qss \
    window/leftpane.qss

DISTFILES += \
    window/stoplockpage.qss \
    window/update_page.qss \
    window/update_progress_page.qss \
    window/Controlled_action_page.qss

# arm-linux system compile env
if(contains(DEFINES, RUN_ON_ARM))
{
    INCLUDEPATH += $$PWD/aarch_env/include
}

#unix:!macx: LIBS += -L$$PWD/aarch_env/lib/glog/ -lglog
unix:!macx: LIBS += -L$$PWD/aarch_env/lib/zlog/ -lzlog

#unix:!macx: LIBS += -L$$PWD/aarch_env/lib/sqlite3/ -lsqlite3


unix:!macx: LIBS += -L/home/vmuser/workspace/work/build-qtmtt/lib -lQt5Mqtt

INCLUDEPATH += /home/vmuser/workspace/work/build-qtmtt/include/QtMqtt
DEPENDPATH += /home/vmuser/workspace/work/build-qtmtt/include/QtMqtt

INCLUDEPATH += /home/vmuser/workspace/work/build-qtmtt/include

unix:!macx: LIBS += -L$$PWD/aarch_env/lib/zlog/ -lzlog

INCLUDEPATH += $$PWD/aarch_env/include/zlog
DEPENDPATH += $$PWD/aarch_env/include/zlog

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/aarch_env/lib/tlog/release/ -ltlog
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/aarch_env/lib/tlog/debug/ -ltlog
else:unix: LIBS += -L$$PWD/aarch_env/lib/tlog -ltlog

INCLUDEPATH += $$PWD/aarch_env/include/tlog
DEPENDPATH += $$PWD/aarch_env/include/tlog
