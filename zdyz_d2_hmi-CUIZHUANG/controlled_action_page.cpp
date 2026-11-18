#include "controlled_action_page.h"
#include "ui_controlled_action_page.h"
#include "./Resources/error_code.h"
#include <QImage>
#include <QPixmap>
#include <QEvent>
#include <QKeyEvent>

extern bool is_large;

ControlledActionPage::ControlledActionPage(QWidget *mainpage,int page, QString trip, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlledActionPage)
{
    ui->setupUi(this);
    this->mainpage = mainpage;
    this->address_set = AddressSet::getInstance();
    this->comm_acs_thread = CommAcsSocketThread::getInstance();
    this->state = SystemState::getInstance();
    this->action_register = ActionRegister::getInstance();

    QFile file(":/qss/Controlled_action_page.qss");
    if(!file.open(QIODevice::ReadOnly)){
        LOG(INFO) << "open qss file failed in " << __FILE__ << __LINE__;
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();
    }
    if(trip.isNull()){}
    //comm_acs_thread->control_timer->start(comm_acs_thread->main_control_time);
    comm_acs_thread->control_timer->start(comm_acs_thread->m_alter_control_time);
    connect(this->mainpage,
            SIGNAL(signal_action_controlled_page(QString&)),
            this,
            SLOT(update_action_descriptor(QString&)));

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

    ui->label_local_address->setText(QString::number(this->address_set->get_local_address()));
    QString value = QString("%1").arg(comm_acs_thread->cmdManager->sensor->shearer_position);
    ui->label_pos->setText(value);
    if (page == 0) {
        ui->label_state->setText("受控");
        ui->label_follow_direction->hide();
    }else if (page == 1) {
        ui->label_state->setText("跟机");
    }else if (page == 2) {
        ui->label_state->setText("推溜");
    }else if (page == 3) {
        ui->label_state->setText("自控");
    }
}

void ControlledActionPage::runGif()
{

    QString str_path = QCoreApplication::applicationDirPath();
    QString Current_Path = str_path;
    str_path += "/style/gif/111.gif";
    movie = new QMovie(str_path);
    //ui->label_address->setPixmap(QPixmap(":/new/menu_page/menu_item/address_sel.png"));
    ui->label_action_descriptor->setMovie(movie); // 1. 设置要显示的 GIF 动画图片
    movie->start();         // 2. 启动动画
}

ControlledActionPage::~ControlledActionPage()
{
    if(movie != nullptr)
    {
        movie->stop();
        delete movie;
        movie = nullptr;
    }
    delete ui;
}


void ControlledActionPage::update_action_descriptor(QString& action_descriptor){
    is_large = true;
    ui->label_action_descriptor->setText(action_descriptor);
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

// not used
void ControlledActionPage::update_follow_direction(int dir){
    LOG(INFO) << "in update_follow_direction";
    QString str_dir;
    if(dir == 0){
        LOG(INFO) << "in update_follow_direction-----21";
        str_dir = "停止";
    }else if(dir == 1){
        LOG(INFO) << "in update_follow_direction-----22";
        str_dir = "上行";
    }else if(dir == 2){
        LOG(INFO) << "in update_follow_direction-----23";
        str_dir = "下行";
    }

    ui->label_follow_direction->setText(str_dir);
    ui->label_follow_direction->setStyleSheet("background-color: rgb(250, 0, 0);font-size:60px;color:blue");
    LOG(INFO) << "in update_follow_direction-----2";
    this->comm_acs_thread->cmdManager->lock_ui_off();
}


void ControlledActionPage::update_follow_step(QString step){
    ui->label_follow_direction->setText(step);
    this->comm_acs_thread->cmdManager->lock_ui_off();
}

bool ControlledActionPage::event(QEvent *event)
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
