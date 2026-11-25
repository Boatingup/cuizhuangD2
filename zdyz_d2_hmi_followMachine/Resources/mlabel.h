#ifndef MLABEL_H
#define MLABEL_H

#include <QLabel>

class MLabel : public QLabel
{
    Q_OBJECT
public:
    MLabel(QWidget* parent = NULL);
    void set_font_size(int size);
    void set_speed(int speed);

public slots:
    void setText(const QString&);

protected:
    virtual void paintEvent(QPaintEvent* e);
    virtual void timerEvent(QTimerEvent* e);
    virtual void resizeEvent(QResizeEvent* e);

    void upateLabelRollingState();
private:
    int left;
    int timerId;
    int fontSize;
};

#endif // MLABEL_H
