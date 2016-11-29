#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QtGui>
#include <QtCore>
#include <QTableWidget>
#include "pdp.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_pushButton_8_clicked();

private:
    Ui::MainWidget *ui;
};

void enable_false(Ui::MainWidget *ui);
void insert_elements_refresh(Ui::MainWidget *ui);
void free(pdp_status *instance);


#endif // MAINWIDGET_H
