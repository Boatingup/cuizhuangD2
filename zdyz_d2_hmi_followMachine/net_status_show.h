#ifndef NET_STATUS_SHOW_H
#define NET_STATUS_SHOW_H

#include <QWidget>
#include "threads/comm_acs_socket_thread.h"
#include "widget_stack.h"


#include <QLabel>
#include <QHBoxLayout>
#include <QVector>
#include <QPainter>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QScrollBar>
#include <QVector>

#define MAX_ROW_NUM 14

class NodeLabel : public QLabel {
    Q_OBJECT
public:
    explicit NodeLabel(int id, int route, QWidget *parent = nullptr) : QLabel(parent), m_id(id) {
        QLabel *text = new QLabel(this);
        text->setText(QString::number(id));
        text->setStyleSheet("color: white;background-color: transparent;qproperty-alignment: AlignCenter;border-radius: 15px;");
        if(route == id) m_route = true;
        if(m_route){
            text->setGeometry(0,30,47,15);

            setAlignment(Qt::AlignCenter);
            setFixedSize(50, 46); // 设置节点大小
            setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/net_state/route.png")));
        }else{
            text->setGeometry(0,22,47,15);

            setAlignment(Qt::AlignCenter);
            setFixedSize(50, 40); // 设置节点大小
            setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/net_state/offline.png")));
        }
    }

    // 模拟节点状态切换
    void setFault(bool isFault) {
        if(m_route){
            if (isFault) {
                setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/net_state/route.png")));
            } else {
                setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/net_state/route.png")));
            }
        }else{
            if (isFault) {
                setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/net_state/offline.png")));
            } else {
                setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/net_state/oneline.png")));
            }
        }

    }

signals:
    void clicked(int id);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        emit clicked(m_id);
        QLabel::mousePressEvent(event);
    }

private:
    int m_id;
    bool m_route{false};
};

class ControllerRow : public QWidget
{
    Q_OBJECT
public:
    explicit ControllerRow(const QString &rowName, int start_no, int end_no, int route_no, QWidget *parent = nullptr) : QWidget(parent) {
        setFixedHeight(46); // 设定每行高度
        setStyleSheet("background-color: transparent;");
        QHBoxLayout *nodesLayout = new QHBoxLayout(this);
        nodesLayout->setContentsMargins(0, 0, 0, 0);
        nodesLayout->setSpacing(5); // 节点之间的水平距离

        for (int i = 0; i < MAX_ROW_NUM; ++i) {
            if(end_no < start_no+i) break;
            NodeLabel *node = new NodeLabel(start_no+i, route_no);
            m_nodes.append(node);
            nodesLayout->addWidget(node);
            connect(node, &NodeLabel::clicked, this, [=](){
                emit nodeClicked(rowName, i);
            });
        }
        nodesLayout->addStretch();
    }

    void updateNodeStatus(int nodeId, bool isOk) {
        if (nodeId > 0 && nodeId <= m_nodes.size()) {
            m_nodes[nodeId - 1]->setFault(!isOk);
        }
    }

    const QVector<NodeLabel*>& getAllNodes() const {
            return m_nodes;
        }

signals:
    void nodeClicked(const QString &rowName, int nodeId);

private:
    QVector<NodeLabel*> m_nodes;
};


namespace Ui {
class net_status_show;
}

class net_status_show : public QWidget
{
    Q_OBJECT

public:
    explicit net_status_show(QWidget *parent = 0);
    ~net_status_show();

protected:
    bool event(QEvent *event);

public:
    int m_start_no;
    int m_end_no;
    int m_route_no;

    QVector<QVector<NodeLabel*>> m_allNodes;
private:
    Ui::net_status_show *ui;

    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
    Parameters* para;

    QScrollArea *scrollArea;

    void setStatus(int support_no, bool status) {
        if((support_no < m_start_no) || (support_no > m_end_no))
            return ;
        int row, list;

        row = (support_no - m_start_no) / MAX_ROW_NUM;
        list = (support_no - m_start_no) % MAX_ROW_NUM;
        m_allNodes[row][list]->setFault(status);
    }

    void setupUI() {
            scrollArea = new QScrollArea(this);
            scrollArea->setGeometry(5,55,790,430);
            scrollArea->setWidgetResizable(true);
            scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent;}");
            scrollArea->setStyleSheet("background-color: transparent;");
            scrollArea->viewport()->setAutoFillBackground(false);
            scrollArea->viewport()->setStyleSheet("background: transparent;");

            QWidget *contentWidget = new QWidget();
            contentWidget->setStyleSheet("background-color: transparent;");

            QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
            contentLayout->setAlignment(Qt::AlignTop); // 从顶部开始排列
            contentLayout->setSpacing(20); // 行间距

            int dev_max = m_end_no - m_start_no + 1;
            int offset = dev_max%14;
            for (int i = 0; i < (dev_max/14)+(offset==0?0:1); ++i) {
                ControllerRow *row = new ControllerRow(QString::number(i), m_start_no+i*14, m_end_no, m_route_no);
                contentLayout->addWidget(row);
                m_allNodes.append(row->getAllNodes());

                QLabel *label_line = new QLabel(contentWidget);
                label_line->setStyleSheet("background-color: transparent;");
                label_line->setPixmap(QPixmap(QString::fromUtf8(":/new/login/image/net_state/connect_line.png")));
                if((offset != 0) && (i > (dev_max/14 - 1))){
                    label_line->setGeometry(40,10+66*i,55*(offset-1),20);
                }else{
                    label_line->setGeometry(40,10+66*i,711,20);
                }
                label_line->lower();
            }




            scrollArea->setWidget(contentWidget);
            scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }

    private slots:
        void onNodeClicked(const QString &rowName, int nodeId) {

        }

public slots:
        void cb_get_dev_status(int dev_no){
            if(dev_no != 0){
                setStatus(dev_no, !comm_acs_thread->cmdManager->m_dev_status[dev_no-1]);
            }else{
                for(int i=0; i<255; i++){
                    setStatus(i, !comm_acs_thread->cmdManager->m_dev_status[i]);
                }
            }
        }
};

#endif // NET_STATUS_SHOW_H
