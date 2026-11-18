#ifndef GUI_SETTING_H
#define GUI_SETTING_H
#include<QSettings>
#include "guiconfig.h"


class gui_setting
{
private:
     gui_setting();
public:
    static gui_setting* getInstance(){
        if(mysettings == nullptr){
            mysettings = new gui_setting();
        }
        return mysettings;
    }
public:
    void getConfig();
    void write(QString first,QString second,QString value);
    void init();
private:
    static gui_setting* mysettings;
public:
    QSettings *settings_app = nullptr;


};

#endif // GUI_SETTING_H
