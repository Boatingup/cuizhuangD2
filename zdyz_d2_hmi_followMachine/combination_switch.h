#ifndef COMBINATION_SWITCH_H
#define COMBINATION_SWITCH_H

#include <QWidget>

namespace Ui {
class combination_switch;
}

class combination_switch : public QWidget
{
    Q_OBJECT

public:
    explicit combination_switch(QWidget *parent = 0);
    ~combination_switch();

private:
    Ui::combination_switch *ui;
};

#endif // COMBINATION_SWITCH_H
