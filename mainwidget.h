#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QtGui>
#include <QtCore>



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

private:
    Ui::MainWidget *ui;
};

#endif // MAINWIDGET_H
