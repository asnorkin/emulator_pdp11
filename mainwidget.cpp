#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "pdp.h"
#include <string>
#include <cstring>
#include <QtGui>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QMessageBox>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    QGraphicsScene* scene = new QGraphicsScene(10, 10, 261, 261, ui->graphicsView);
    QString qstr = QString::fromStdString("girl.png");
    QImage image(qstr);

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    //ui->graphicsView->setScene(scene);
    //ui->graphicsView->show();
}

MainWidget::~MainWidget()
{
    delete ui;
}
//////////////////////////////////////////////////////

bool pause_pressed = false;
bool started = false;
bool finish = false;
const std::string final = "HALT";
std::string file_name = "";
pdp pdp_instance;

void free(pdp_status *instance) {
    delete[] (instance -> registers);
    delete instance;
}

void MainWidget::on_pushButton_clicked() //start
{
    pause_pressed = false;
    if(started == false) {
        bool result = pdp_instance.load_program((char *)file_name.c_str());
        started = true;
        if(result != true) {
            exit(-1); //TODO: Some other realization!
        }
    }

    pdp_status *instruction_content = NULL;
    std::string compare;
    if(finish != true) {
        do {
            if(pause_pressed == true) break;
            instruction_content = pdp_instance.run_next_instruction();
            compare = instruction_content -> disasm_command;
            if(!final.compare(compare)) {
                finish = true;
            }
            ui->listWidget->addItem( (instruction_content -> disasm_command).c_str());
            ui->label_9->setText(QString::number(instruction_content -> registers[0]));
            ui->label_10->setText(QString::number(instruction_content -> registers[1]));
            ui->label_11->setText(QString::number(instruction_content -> registers[2]));
            ui->label_12->setText(QString::number(instruction_content -> registers[3]));
            ui->label_13->setText(QString::number(instruction_content -> registers[4]));
            ui->label_14->setText(QString::number(instruction_content -> registers[5]));
            ui->label_15->setText(QString::number(instruction_content -> registers[6]));
            ui->label_16->setText(QString::number(instruction_content -> registers[7]));
            free(instruction_content);
        } while(final.compare(compare));
        if((pause_pressed != true) && (finish == true)) {
            ui->pushButton_5->setEnabled(true);
            ui->lineEdit->setEnabled(true);
        }
    }
}

void MainWidget::on_pushButton_4_clicked() // pause
{
    pause_pressed = true;
}

void MainWidget::on_pushButton_3_clicked() //stop/reset
{
    ui->listWidget->clear();
    ui->graphicsView->setBackgroundBrush(Qt::white);
    ui->label_9->clear();
    ui->label_10->clear();
    ui->label_11->clear();
    ui->label_12->clear();
    ui->label_13->clear();
    ui->label_14->clear();
    ui->label_15->clear();
    ui->label_16->clear();
    started = false;
    pause_pressed = false;
    finish = false;
}

void MainWidget::on_pushButton_2_clicked() // step
{
    pdp_status *instruction_content = NULL;
    if(started == false) {
        bool result = pdp_instance.load_program((char *)file_name.c_str());
        started = true;
        if(result != true) {
            exit(-1); //TODO: Some other realization!
        }
    }

    if(finish != true) {
        instruction_content = pdp_instance.run_next_instruction();
        if(!final.compare(instruction_content -> disasm_command)) {
            finish = true;
        }
        ui->listWidget->addItem( (instruction_content -> disasm_command).c_str());
        ui->label_9->setText(QString::number(instruction_content -> registers[0]));
        ui->label_10->setText(QString::number(instruction_content -> registers[1]));
        ui->label_11->setText(QString::number(instruction_content -> registers[2]));
        ui->label_12->setText(QString::number(instruction_content -> registers[3]));
        ui->label_13->setText(QString::number(instruction_content -> registers[4]));
        ui->label_14->setText(QString::number(instruction_content -> registers[5]));
        ui->label_15->setText(QString::number(instruction_content -> registers[6]));
        ui->label_16->setText(QString::number(instruction_content -> registers[7]));
        free(instruction_content);
    }
    if((pause_pressed != true) && (finish == true)) {
        ui->pushButton_5->setEnabled(true);
        ui->lineEdit->setEnabled(true);
    }
}

void MainWidget::on_pushButton_5_clicked()
{
    QString line_str = ui->lineEdit->text();
    if(line_str.isNull())
        {
            QMessageBox::information(this,"Image Viewer","Error Displaying image");
            return;
        }
    file_name = line_str.toStdString();
    ui->pushButton_5->setEnabled(false);
    ui->lineEdit->setEnabled(false);
}
