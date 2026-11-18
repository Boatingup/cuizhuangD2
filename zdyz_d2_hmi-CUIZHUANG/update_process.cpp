#include "update_process.h"
#include "ui_update_process.h"

#include <QEvent>
#include <QKeyEvent>

#include "Resources/key_value.h"

#define G_BYTES_UNIT 0x40000000
#define M_BYTES_UNIT 0x100000
#define K_BYTES_UNIT 0x400

update_process::update_process(QString data, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::update_process)
{
    ui->setupUi(this);
    comm_acs_thread = CommAcsSocketThread::getInstance();
    widget_stack = WidgetStack::getInstance();

    ui->progressBar_process->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    if(data != "none"){
        ui->progressBar_process->hide();
        ui->label_process->hide();
        ui->label_sign_process->hide();

        ui->label_sign_file->setText(data);
    }
}

update_process::~update_process()
{
    delete ui;
}

static QString format_update_size(double bytes)
{
    if (bytes < 0)
        return QStringLiteral("--");
    if (bytes < K_BYTES_UNIT * 100)
        return QString::number(static_cast<qint64>(bytes)) + QStringLiteral("bytes");
    if (bytes < M_BYTES_UNIT * 100)
        return QString::number(bytes / K_BYTES_UNIT, 'f', 1) + QStringLiteral("KB");
    return QString::number(bytes / M_BYTES_UNIT, 'f', 3) + QStringLiteral("MB");
}

void update_process::process_update(double cur, double total)
{
    /* FTP 常给不出总长度（total=-1），不能按 bytes 原样显示，也不能把进度条 max 设成 -1 */
    if (total <= 0) {
        ui->label_process->setText(format_update_size(cur) + QStringLiteral("/--"));
        ui->progressBar_process->setRange(0, 0);
        ui->progressBar_process->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        return;
    }

    ui->label_process->setText(format_update_size(cur) + QStringLiteral("/") + format_update_size(total));
    ui->progressBar_process->setRange(0, 1000);
    int percent = static_cast<int>(cur / total * 1000.0);
    if (percent < 0)
        percent = 0;
    if (percent > 1000)
        percent = 1000;
    ui->progressBar_process->setValue(percent);
    ui->progressBar_process->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void update_process::status_update(QString status)
{
    ui->label_update_status->setText(status);
}

void update_process::file_set(QString name)
{
    ui->label_file->setText(name);
}


bool update_process::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent*>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_CANCEL:{
            QJsonObject info_obj, reson_obj;
            reson_obj["reason"] = "cancel";
            info_obj["interrupt"] = reson_obj;
            //发送给本机
            QJsonDocument json_doc(info_obj);
            this->comm_acs_thread->send_socket_msg_to_acs(QString("update"), json_doc.toJson());

            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~update_process();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        }
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











