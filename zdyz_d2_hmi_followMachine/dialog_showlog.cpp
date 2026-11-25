#include "dialog_showlog.h"
#include "ui_dialog_showlog.h"
#include "Resources/key_value.h"
#include "threads/comm_acs_thread.h"

#include <QEvent>
#include <QKeyEvent>
#include "./Resources/error_code.h"


char s[1024] ={0};

Dialog_showLog::Dialog_showLog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_showLog)
{
    ui->setupUi(this);
    comm_acs_thread = CommAcsSocketThread::getInstance();
    this->widget_stack = WidgetStack::getInstance();

    int length = this->comm_acs_thread->cmdManager->last_msg.size();
    for(int i = 0;i < length;i++)
    {
        ui->textEdit_Log->append(this->comm_acs_thread->cmdManager->last_msg[i]);
    }
    QFile file(":/qss/run_error.qss");
    if(!file.open(QIODevice::ReadOnly)){
        LOG(INFO) << "open qss file failed in " << __FILE__ << __LINE__;
    }else{
        QString ss = file.readAll();
        LOG(INFO) << ss.toLocal8Bit().data();
        this->setStyleSheet(ss);
        file.close();
    }
    ui->textEdit_Log->moveCursor(QTextCursor::End);
}



bool Dialog_showLog::event(QEvent *event)
{
    QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
    switch(key_event->type())
    {
    case KeyValue::KEY_PRESSED:    // key pressed
        break;
    case KeyValue::KEY_RELEASED:  // key released
        switch(key_event->key()){

        case KeyValue::KEY_OK:
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_DOWN:
            ui->textEdit_Log->moveCursor(QTextCursor::Down);
            ui->textEdit_Log->moveCursor(QTextCursor::Down);
            ui->textEdit_Log->moveCursor(QTextCursor::Down);
            ui->textEdit_Log->moveCursor(QTextCursor::Down);
            this->comm_acs_thread->cmdManager->lock_ui_off();
            break;
        case KeyValue::KEY_ARROW_UP:
             ui->textEdit_Log->moveCursor(QTextCursor::Up);
             ui->textEdit_Log->moveCursor(QTextCursor::Up);
             ui->textEdit_Log->moveCursor(QTextCursor::Up);
             ui->textEdit_Log->moveCursor(QTextCursor::Up);
             this->comm_acs_thread->cmdManager->lock_ui_off();
             break;
        case KeyValue::KEY_CANCEL:
            this->comm_acs_thread->cmdManager->window = this->widget_stack->pop();
            this->close();
            this->~Dialog_showLog();
            this->comm_acs_thread->cmdManager->lock_ui_off();
            //this->comm_acs_thread->cmdManager->lock_ui_off();
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
int Dialog_showLog::getLog(const char * file_path, QString &displaystr)
{
    //ui->textEdit_Log->append("123");

    //QString displayString;
    QFile file(file_path);            //目标文件路径
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        DBG_INFO_OLD("Can't open the file!\n");
    }
    while(!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString str(line);
        DBG_INFO_OLD("%s\n", str.toUtf8().data());
        displaystr.append(str);
        //ui->textEdit_Log->append("123");
    }
    return 0;
}

/*
QString Dialog_showLog::getLog(const char * file_path)
{
    FILE *in;
    int count=0;
    int argvline;
    long int pos;
    //char s[10240];
    argvline = 100;
    in=fopen(file_path,"r");

    if(in==NULL){
     perror("fopen");
     exit(EXIT_FAILURE);
    }
    fseek(in,0,SEEK_END);
    pos=ftell(in);
    //从后往前读的技巧就在这儿
    while(pos){
     fseek(in,--pos,SEEK_SET);
     if(fgetc(in)=='\n'){
      if(count++==argvline) break;
     }
    }//end while(pos)

    //当文件内行数不足指定输出的行数时，默认输出文件所有内容
    //注意，这儿一定的先--count,因为之前的循环内多加了
    if(--count!=argvline){
     // printf("\ncount=%d,argvline=%d\n",count,argvline);
     //直接定位到文件头
     fseek(in,0,SEEK_SET);
    }
    while(fgets(s,sizeof(s),in)!=NULL){
       printf("%s",s);
       QString test(s);
       ui->textEdit_Log->append(test);
       qDebug() << test;
    }

    fclose(in);
    return 0;
}
*/

Dialog_showLog::~Dialog_showLog()
{
    delete ui;
}
