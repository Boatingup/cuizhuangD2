#ifndef KEYBOARDREADTHREAD_H
#define KEYBOARDREADTHREAD_H

#include <QThread>
#include <QWidget>
#include <QFile>


class KeyboardReadThread : public QThread
{
    Q_OBJECT
public:
    explicit KeyboardReadThread(QObject *parent = nullptr);
    
signals:
    
public slots:

protected:
    void run();

public:
    int parse(char* data, int lenght);

public:
    static unsigned char myxor(void *data, int length);
private:
    int check_key_state(unsigned char* previous, unsigned char* current, int length);
public:
    void setFousedWindow(QWidget* win);
private:
    QWidget* window;

    int logfile;
    
};

#endif // KEYBOARDREADTHREAD_H
