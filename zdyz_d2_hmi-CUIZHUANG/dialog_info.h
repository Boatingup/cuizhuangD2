#ifndef DIALOG_INFO_H
#define DIALOG_INFO_H

#include <QDialog>
#include <QLabel>
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"
#include "Resources/key_value.h"

namespace Ui {
class Dialog_info;
}

class Dialog_info : public QDialog
{
    Q_OBJECT

public:
    //参数名 : 标题   内容   提示信息  自动关闭时间
    explicit Dialog_info(QString title,QString content,QString info,float time,QWidget *parent = nullptr);
    ~Dialog_info();
    int GetStylePath();

    void setFontNumber(QLabel *label,int number);
    void setFontColor(QLabel *label,QColor color);
    void doRecord();

private:
    Ui::Dialog_info *ui;
private:
    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
    QString m_title;
    QString m_content;
    QString m_info;
    float m_delay;
    QTimer *timer;
    //QList <Template_sheet> list_template;
protected:
    bool event(QEvent *event);
public slots:
    void timer_cb(void);
};

#endif // DIALOG_INFO_H
