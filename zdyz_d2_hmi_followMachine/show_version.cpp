#include "show_version.h"
#include "ui_show_version.h"
#include "./Resources/key_value.h"
#include "threads/comm_acs_thread.h"
#include <QEvent>
#include <QGridLayout>
#include <QKeyEvent>
#include <QScrollBar>
#include "unistd.h"


show_version::show_version(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::show_version)
{
    ui->setupUi(this);
    this->para = Parameters::getInstance();
    comm_acs_thread = CommAcsSocketThread::getInstance();
    widget_stack = WidgetStack::getInstance();
    scrollArea = ui->scrollArea;

    comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    fill_version_list();
}

QLabel *show_version::create_version_label(const QString &text)
{
    QLabel *label = new QLabel(text);
    QFont font;
    font.setFamily("YouSheBiaoTiHei");
    font.setPointSize(32);
    label->setFont(font);
    label->setStyleSheet("color: rgb(255, 255, 255);");
    label->setWordWrap(true);
    return label;
}

void show_version::fill_version_list()
{
    QWidget *content = ui->scrollAreaWidgetContents;
    QGridLayout *grid = new QGridLayout(content);
    grid->setContentsMargins(20, 10, 20, 10);
    grid->setHorizontalSpacing(40);
    grid->setVerticalSpacing(8);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 2);

    int row = 0;
    auto add_row = [&](const QString &key, const QString &value) {
        grid->addWidget(create_version_label(key), row, 0, Qt::AlignLeft | Qt::AlignVCenter);
        grid->addWidget(create_version_label(value), row, 1, Qt::AlignLeft | Qt::AlignVCenter);
        row++;
    };

    add_row(QString::fromUtf8("界面"), get_hmi_version());

    const QMap<QString, QString> &config_map = comm_acs_thread->cmdManager->m_config_info_map;
    for (QMap<QString, QString>::const_iterator it = config_map.constBegin(); it != config_map.constEnd(); ++it) {
        QString display_key = it.key();
        if (display_key.endsWith("_version")) {
            display_key.chop(QString("_version").length());
        }
        add_row(display_key, it.value());
    }

    QString pcb_version;
    if(access("/sys/class/i2c-dev/i2c-4", F_OK) == 0){
        pcb_version = "V1.0.1";
    } else {
        pcb_version = "V1.0.2";
    }
    add_row(QString::fromUtf8("硬件版本"), pcb_version);

    grid->setRowStretch(row, 1);
}

static int exec_sys_cmd(const char *cmd, char *result)
{
    char buf_ps[1024];
    char ps[1024]={0};
    FILE *ptr;
    strcpy(ps, cmd);
    if((ptr=popen(ps, "r"))!=nullptr)
    {
        while(fgets(buf_ps, 1024, ptr)!=nullptr)
        {
           strcat(result, buf_ps);
           if(strlen(result)>1024)
               break;
        }
        pclose(ptr);
        ptr = nullptr;
        return 0;
    }
    else
    {
        printf("popen %s error\n", ps);
        return 1;
    }
}

QString show_version::get_hmi_version()
{
    QString version;

    version = HMI_VERSION;
    return version;
}

QString show_version::get_kernel_version()
{
    QString version;
    char result[2048] = {0};
#if 0
    const char *cmd = "uname -r";
#else
    const char *cmd = "cat /proc/sys/kernel/version | awk '{print $8 \"/\" $4 \"/\" $5 \" \" $6}'";
#endif
    if(0 == exec_sys_cmd(cmd, result))
        version = result;

    return version;
}

show_version::~show_version()
{
    delete ui;
}

bool show_version::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->control_timer->stop();
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~show_version();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_RIGHT:
            this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_LEFT:
            this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if (this->scrollArea && this->scrollArea->verticalScrollBar()) {
                this->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
                this->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
                this->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
            this->comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if (this->scrollArea && this->scrollArea->verticalScrollBar()) {
                this->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
                this->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
                this->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_PLUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if (this->scrollArea && this->scrollArea->verticalScrollBar()) {
                this->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_MINUS:
            comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
            if (this->scrollArea && this->scrollArea->verticalScrollBar()) {
                this->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
            }
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        default:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
        break;
    default:
        break;
    }
    return QWidget::event(event);
}
