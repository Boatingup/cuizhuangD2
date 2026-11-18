#ifndef FAULT_DIAGNOSIS_DIAPLSY_H
#define FAULT_DIAGNOSIS_DIAPLSY_H

#include <QDialog>
#include "threads/comm_acs_socket_thread.h"

namespace Ui {
class fault_diagnosis_diaplsy;
}

class fault_diagnosis_diaplsy : public QDialog
{
    Q_OBJECT

public:
    explicit fault_diagnosis_diaplsy(QString fault_name, QString fault_dignosis, int fault_id, QWidget *parent = nullptr);
    ~fault_diagnosis_diaplsy();

    CommAcsSocketThread* comm_acs_thread;
    WidgetStack* widget_stack;
protected:
    bool event(QEvent *event);
private:
    Ui::fault_diagnosis_diaplsy *ui;

    bool m_aux_flag{false};
    int m_fault_id{0};
public:
    void hide_fn();
    void show_fn();
};

#endif // FAULT_DIAGNOSIS_DIAPLSY_H
