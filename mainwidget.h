#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QtGui>
#include <QtCore>
#include <QTableWidget>
#include <QGraphicsScene>
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
protected:
    virtual void paintEvent(QPaintEvent *event);

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

    void on_pushButton_9_clicked();

private:
    Ui::MainWidget *ui;
    QGraphicsScene *scene;
    bool pictureSet;
    int viewWidth;
    int viewHeight;
};

void enable_false(Ui::MainWidget *ui);
void insert_elements_refresh(Ui::MainWidget *ui);
void free(pdp_status *instance);


#endif // MAINWIDGET_H
