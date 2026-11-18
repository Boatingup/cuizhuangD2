#include "combination_switch.h"
#include "ui_combination_switch.h"

#include <QFile>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QTableView>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QWidget>

combination_switch::combination_switch(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::combination_switch)
{
    ui->setupUi(this);

    ui->tableWidget->hide();

    QFile file(":/qss/combin.qss");
    if(!file.open(QIODevice::ReadOnly)){
    }else{
        QString ss = file.readAll();
        this->setStyleSheet(ss);
        file.close();
    }

    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->move(0, 40);
    tabWidget->setGeometry(5, 175, 790, 300);

    QTableView* tableView;

    QWidget *tab = new QWidget();
    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setMargin(0);
    hlayout->setSpacing(0);

    tableView = new QTableView();


    //设置左半侧tableview
    tableView->setShowGrid(false); //不显示格子线
    tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示垂直滚动条
    tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); //不显示水平滚动条
    tableView->verticalHeader()->setVisible(false); //垂头（也就是行号）不可见
    tableView->setAlternatingRowColors(true); //奇偶行不同颜色显示打开，要配合qss

    hlayout->addWidget(tableView);

    tab->setLayout(hlayout);
    tabWidget->addTab(tab, "");
    tabWidget->setStyleSheet("QTabBar::tab { height: 0px; }");


    QList<QString> tab_list;
    tab_list<<"前溜变频器";
    tab_list<<"后溜变频器";
    tab_list<<"皮带1溜变频器";
    tab_list<<"皮带2变频器";
    tab_list<<"泵站1变频器";
    tab_list<<"泵站2变频器";
    QStandardItemModel* model = new QStandardItemModel();
    model->setColumnCount(tab_list.size());
    for(int i=0; i<tab_list.size(); i++){
        model->setHeaderData(i, Qt::Horizontal, tab_list.at(i));
    }

    tableView->setModel(model);
    //设置宽度要在设置了model之后调用
    tableView->setColumnWidth(0, 240);
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    //tableView_left->horizontalHeader()->setVisible(false);

    tableView->verticalHeader()->setDefaultSectionSize(40);
    //表中有被选中的行时，表头不变色
    tableView->horizontalHeader()->setHighlightSections(false);

    //QFont font;
    model->setItem(0, 0, new QStandardItem("111")); //设置tablebiew中的文字内容
    model->setItem(0, 1, new QStandardItem("222"));

    model->setItem(1, 0, new QStandardItem("333")); //设置tablebiew中的文字内容
    model->setItem(1, 1, new QStandardItem("444"));

    model->item(0, 0)->setTextAlignment(Qt::AlignCenter); //设置文字水平和垂直都居中显示
    model->item(0, 1)->setTextAlignment(Qt::AlignCenter);

    tableView->setModel(model);


}

combination_switch::~combination_switch()
{
    delete ui;
}
