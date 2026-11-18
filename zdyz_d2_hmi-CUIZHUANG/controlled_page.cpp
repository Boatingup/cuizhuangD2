#include "controlled_page.h"
#include "ui_controlled_page.h"


#include <QImage>
#include <QPixmap>
#include <QEvent>
#include <QKeyEvent>
#include "./window/mainpage.h"
#include "./Resources/error_code.h"

extern bool is_large;

controlled_page::controlled_page(QWidget *mainpage, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::controlled_page)
{
    ui->setupUi(this);

    this->mainpage = mainpage;
    this->address_set = AddressSet::getInstance();
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->state = SystemState::getInstance();
    this->action_register = ActionRegister::getInstance();
    this->sensor = Sensor::getInstance();
    ui->label_action->setWordWrap(true);

    //comm_acs_thread->control_timer->start(comm_acs_thread->m_alter_control_time*100);
    comm_acs_thread->control_timer->stop();


    connect(this->mainpage,
            SIGNAL(signal_action_controlled_page(QString&, QString&)),
            this,
            SLOT(update_action_descriptor(QString&, QString&)));

    connect(this->mainpage,
            SIGNAL(signal_update_follow_direction(int)),
            this,
            SLOT(update_follow_direction(int)));

    connect(this->mainpage,
            SIGNAL(signal_update_follow_step(QString)),
            this,
            SLOT(update_follow_step(QString)));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(close_self_push()),
            this->mainpage,
            SLOT(exit_controlled_page()));

    connect(comm_acs_thread->cmdManager,
            SIGNAL(signal_update_sensor_data(unsigned int)),
            this,
            SLOT(update_sensor_data(unsigned int)));

    ui->label_controlled->setText(QString("%3").arg(this->address_set->get_local_address(), 3, 10, QLatin1Char('0')));
    ui->label_coal_mechine->setText(QString("%3").arg(comm_acs_thread->cmdManager->sensor->shearer_position, 3, 10, QLatin1Char('0')));
    movie = new QMovie();

    ui->label_main->setText(QString("%3").arg(sensor->master_no, 3, 10, QLatin1Char('0')));
    update_sensor_data(mainPage::View_All);
}

void controlled_page::update_sensor_data(unsigned int view){

    if(view & mainPage::View_Pressure){
        if(sensor->pressure[1] < 0.0f || sensor->pressure[1] > 60.0f){
            ui->label_pressure1->setText(QString("---"));
        }else{
            ui->label_pressure1->setText(QString("%1").arg(QString::number(static_cast<double>(sensor->pressure[1]), 'f', 1)));
        }

        if(sensor->pressure[2] < 0.0f || sensor->pressure[2] > 60.0f){
            ui->label_pressure2->setText(QString("---"));
        }else{
            ui->label_pressure2->setText(QString("%1").arg(QString::number(static_cast<double>(sensor->pressure[2]), 'f', 1)));
        }
    }

    if(view & mainPage::View_Stroke){
        if(sensor->stroke[1] < 0 || sensor->stroke[1] > 5000)
        {
            ui->label_travel1->setText(QString("---"));
        }
        else {
            ui->label_travel1->setText(QString("%1").arg(sensor->stroke[1]));
        }

        if(sensor->stroke[2] < 0 || sensor->stroke[2] > 5000)
        {
            ui->label_travel2->setText(QString("---"));
        }
        else {
            ui->label_travel2->setText(QString("%1").arg(sensor->stroke[2]));
        }
    }

    if(view & mainPage::View_Height){
        if(sensor->height == 0){
            ui->label_hight->setText(QString("---"));
        } else {
            ui->label_hight->setText(QString("%1").arg(QString::number(sensor->height)));
        }
    }

    if(view & mainPage::View_Shearer_Position ){
        ui->label_coal_mechine->setText(QString("%3").arg(sensor->shearer_position, 3, 10, QLatin1Char('0')));
    }

    if(view & mainPage::View_Dip_Angle){
        for(int index = 0; index < 5; index++){
            QString display = QString::number(this->sensor->dip_angle[index][0], 'f', 0) + "," +
                    QString::number(this->sensor->dip_angle[index][1], 'f', 0) + ","+
                    QString::number(this->sensor->dip_angle[index][2], 'f', 0);
            if(index == 0){
                ui->label_angle1->setText(display);
            } else if(index == 1){
                ui->label_angle2->setText(display);
            } else if(index == 2){
                ui->label_angle3->setText(display);
            } else if(index == 3){
                ui->label_angle4->setText(display);
            } else if(index == 4){
                ui->label_angle5->setText(display);
            }
        }
    }
}
void controlled_page::run_gif(QString file_name)
{
    if((file_name.isNull()) || (access(file_name.toUtf8().data(), F_OK) != 0)){
        movie->stop();
        ui->label_gif->hide();
        return ;
    }
    movie->setFileName(file_name);
    ui->label_gif->setMovie(movie); // 1. 设置要显示的 GIF 动画图片
    ui->label_gif->setScaledContents(true);
    ui->label_gif->show();
    movie->start();         // 2. 启动动画
}

controlled_page::~controlled_page()
{
    if(movie != nullptr)
    {
        movie->stop();
        delete movie;
        movie = nullptr;
    }
    delete ui;
}


void controlled_page::update_action_descriptor(QString& action_descriptor, QString &gif_info){
    is_large = true;
    ui->label_action->setText(action_descriptor);
    run_gif(gif_info);
}

// not used
void controlled_page::update_follow_direction(int dir){
    TLOG_INFO("in update_follow_direction");
    QString str_dir;
    if(dir == 0){
        TLOG_INFO("in update_follow_direction-----21");
        str_dir = "停止";
    }else if(dir == 1){
        TLOG_INFO("in update_follow_direction-----22");
        str_dir = "上行";
    }else if(dir == 2){
        TLOG_INFO("in update_follow_direction-----23");
        str_dir = "下行";
    }

    TLOG_INFO("in update_follow_direction-----2");
}


void controlled_page::update_follow_step(QString step){
    ui->label_stage->setText(step);
}

void controlled_page::update_sensor_info(){

}

bool controlled_page::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        this->comm_acs_thread->cmdManager->lock_ui_off();
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){
        case KeyValue::KEY_NUM_0:
        case KeyValue::KEY_NUM_1:
        case KeyValue::KEY_F1:
        case KeyValue::KEY_F2:
        case KeyValue::KEY_F3:
        case KeyValue::KEY_NUM_2:
        case KeyValue::KEY_NUM_3:
        case KeyValue::KEY_F4:
        case KeyValue::KEY_F5:
        case KeyValue::KEY_F6:
        case KeyValue::KEY_NUM_4:
        case KeyValue::KEY_NUM_5:
        case KeyValue::KEY_ARROW_PLUS:
        case KeyValue::KEY_ARROW_RIGHT:
        case KeyValue::KEY_ARROW_LEFT:
        case KeyValue::KEY_NUM_6:
        case KeyValue::KEY_NUM_7:
        case KeyValue::KEY_ARROW_MINUS:
        case KeyValue::KEY_ARROW_UP:
        case KeyValue::KEY_ARROW_DOWN:
        case KeyValue::KEY_PUSH_PULL:
        case KeyValue::KEY_NUM_9:
        //case KeyValue::KEY_FN:
        case KeyValue::KEY_OK:
        case KeyValue::KEY_CANCEL:
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
