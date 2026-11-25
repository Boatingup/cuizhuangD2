#ifndef WIDGET_STACK_H
#define WIDGET_STACK_H

#include <QStack>
#include <QWidget>

class WidgetStack : public QStack<QWidget*>
{
private:
    WidgetStack();

public:
    static WidgetStack *getInstance(){
        if(widget_stack == nullptr){
            widget_stack = new WidgetStack();
        }
        return widget_stack;
    }

public:
    static WidgetStack* widget_stack;


};

#endif // WIDGET_STACK_H
