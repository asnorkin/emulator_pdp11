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
    enable_false(ui);
    ui->pushButton_3->setEnabled(false);
}

MainWidget::~MainWidget()
{
    delete ui;
}
//////////////////////////////////////////////////////
bool pause_pressed = false;
bool started = false;
bool finish = false;
const std::string final = "HALT ";
std::string file_name = "";

int num_of_rows = 0;
int breakpoint = -1;
pdp pdp_instance;

void free(pdp_status *instance) {
    delete[] (instance -> registers);
    delete instance;
}

void insert_row(Ui::MainWidget *ui) {
    ui->tableWidget->insertRow(num_of_rows);
    num_of_rows++;
}

void enable_false(Ui::MainWidget *ui) {
    ui->pushButton_4->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
}

void insert_elements_refresh(Ui::MainWidget *ui) {
    ui->pushButton_5->setEnabled(true);
    ui->lineEdit->setEnabled(true);
    ui->lineEdit->clear();
}

void MainWidget::on_pushButton_clicked() //start
{
    pause_pressed = false;
    ui->pushButton_3->setEnabled(true);
    if(started == false) {
        bool result = pdp_instance.load_program((char *)file_name.c_str());
        started = true;
        if(result != true) {
            exit(-1);
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
            insert_elements_refresh(ui);
            enable_false(ui);
        }
    }
}

void MainWidget::on_pushButton_4_clicked() // pause
{
    pause_pressed = true;
}

void MainWidget::on_pushButton_3_clicked() // reset
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
    ui->pushButton_3->setEnabled(false);
}

void MainWidget::on_pushButton_2_clicked() // step
{
    ui->pushButton_3->setEnabled(true);
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
        insert_row(ui);
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
    if(finish == true) {
        insert_elements_refresh(ui);
        enable_false(ui);
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
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_4->setEnabled(true);
}
