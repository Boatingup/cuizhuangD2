#ifndef PORT_VISUALIZATION_H
#define PORT_VISUALIZATION_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QVector>

#include "Resources/dbg_print.h"
#include "threads/comm_acs_socket_thread.h"
namespace Ui {
class port_visualization;
}

typedef enum
{
    INIT_TYPE = 0,
    CONNECTING_TYPE = 1,
    CONNECTED_TYPE = 2,
    DISCONNECT_TYPE = 3,
    OFFLIMIT_TYPE = 4,
    MAX_STATUS_TYPE
}STATUS_E;

typedef struct{
    QLabel *label;
    QString label_focus;
    QString label_unfocus;

    QPushButton *button;
    QString button_focus;
    QString button_unfocus;

    QVector<QFrame *> line;
    void (*set_focus)(QString port_name);
    void (*set_status)(STATUS_E type);
}SET_STRATEGY_S;

class port_visualization : public QWidget
{
    Q_OBJECT

public:
    explicit port_visualization(QWidget *parent = nullptr);
    ~port_visualization();

    WidgetStack* widget_stack;
    CommAcsSocketThread* comm_acs_thread;
    void draw();
    void update_label(int index);
    void set_status(SET_STRATEGY_S icon, STATUS_E type);
    void update_status(QString fault_name, QString fault_status);
    Parameters* para;
    QList <QList <int>> list_index;
private:
    Ui::port_visualization *ui;
    void load_label_vector(QVector<QVector<SET_STRATEGY_S>> &label_vector);
    int row;//行
    int column;//列

    QVector<QVector<SET_STRATEGY_S>> m_label_vector;
    QMultiMap<QString, QPair<QString, STATUS_E>> m_setting_fault_map;
    QMap<QString, QPair<STATUS_E, SET_STRATEGY_S>> m_fault_map;
protected:
    bool event(QEvent *event);

};

#endif // PORT_VISUALIZATION_H
