#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "pdp.h"
#include <string>
#include <cstring>
#include <QtGui>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QPainter>

bool pause_pressed = false;
bool started = false;
bool finish = false;

const std::string final = "HALT ";
std::string file_name = "";
int row;

bool buffer_ready = false;
unsigned char *buffer_ = NULL;

int num_of_rows = 0;
int breakpoint = -1;
pdp pdp_instance;

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    pictureSet = false;

    viewWidth = ui->graphicsView->width();
    viewHeight = ui->graphicsView->height();

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);

    ui->pushButton_3->setEnabled(false);
    ui->pushButton_7->setEnabled(false);
    ui->pushButton_8->setEnabled(false);
}

MainWidget::~MainWidget()
{
    delete ui;
    delete scene;
}

void MainWidget::paintEvent(QPaintEvent *event)
{
    if(buffer_ready == true) {
        //////
        struct stat buf;
        stat("pause.rgba", &buf);

        unsigned char* buffer = (unsigned char *)calloc(buf.st_size, sizeof(unsigned char));

        int fd = open("pause.rgba", O_RDONLY);
        pread(fd, buffer, buf.st_size, 0);
        //close(fd);

        QImage img(buffer, 256, 256, QImage::Format_RGBA8888);

        //////
        scene->clear();
        scene->addPixmap(QPixmap::fromImage(img));
        ui->graphicsView->update();
    } else {
        scene->clear();
        ui->graphicsView->update();
    }
}

//////////////////////////////////////////////////////


void free(pdp_status *instance) {
    delete[] (instance -> registers);
    delete instance;
}

void insert_row(Ui::MainWidget *ui) {
    ui->tableWidget->insertRow(num_of_rows);
    num_of_rows++;
}

void set_text_in_labels(Ui::MainWidget *ui, pdp_status *instruction_content) {
    ui->label_9->setText(QString::number(instruction_content -> registers[0]));
    ui->label_10->setText(QString::number(instruction_content -> registers[1]));
    ui->label_11->setText(QString::number(instruction_content -> registers[2]));
    ui->label_12->setText(QString::number(instruction_content -> registers[3]));
    ui->label_13->setText(QString::number(instruction_content -> registers[4]));
    ui->label_14->setText(QString::number(instruction_content -> registers[5]));
    ui->label_15->setText(QString::number(instruction_content -> registers[6]));
    ui->label_16->setText(QString::number(instruction_content -> registers[7]));
    ui->label_24->setText(QString::number(instruction_content -> N_flag));
    ui->label_25->setText(QString::number(instruction_content -> Z_flag));
    ui->label_26->setText(QString::number(instruction_content -> V_flag));
    ui->label_27->setText(QString::number(instruction_content -> C_flag));
}

void enable_false(Ui::MainWidget *ui) {
    ui->pushButton_4->setEnabled(false);
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
}

void clear_labels(Ui::MainWidget *ui) {
    ui->label_9->clear();
    ui->label_10->clear();
    ui->label_11->clear();
    ui->label_12->clear();
    ui->label_13->clear();
    ui->label_14->clear();
    ui->label_15->clear();
    ui->label_16->clear();
    ui->label_24->clear();
    ui->label_25->clear();
    ui->label_26->clear();
    ui->label_27->clear();
}

void reset_common(Ui::MainWidget *ui) {
    clear_labels(ui);
    started = false;
    pause_pressed = false;
    finish = false;
    ui->pushButton_3->setEnabled(false);
    pdp_instance.reset();
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

            if(buffer_ready == false) {
                buffer_ = instruction_content -> VRAM;
                buffer_ready = true;
            } else {
                //free(instruction_content -> VRAM);
            }

            if(!final.compare(compare)) {
                finish = true;
            }

            set_text_in_labels(ui, instruction_content);
            insert_row(ui);
            QTableWidgetItem *newItem_1 = new QTableWidgetItem();
            QTableWidgetItem *newItem_2 = new QTableWidgetItem();
            newItem_1->setText(QString::fromStdString(instruction_content -> disasm_command));
            newItem_2->setText(QString::number(instruction_content -> registers[7]));
            ui->tableWidget->setItem(num_of_rows-1, 0, newItem_1);
            ui->tableWidget->setItem(num_of_rows-1, 1, newItem_2);
            //free(instruction_content -> registers);
            free(instruction_content -> RAM);
            free(instruction_content);
        } while(final.compare(compare));
        if((pause_pressed != true) && (finish == true)) {
            insert_elements_refresh(ui);
            enable_false(ui);
            ui->pushButton_7->setEnabled(true);
        }
    }
}

void MainWidget::on_pushButton_4_clicked() // pause
{
    pause_pressed = true;
}

void MainWidget::on_pushButton_3_clicked() // reset
{
    reset_common(ui);
}

void MainWidget::on_pushButton_2_clicked() // step
{
    ui->pushButton_3->setEnabled(true);
    pdp_status *instruction_content = NULL;
    if(started == false) {
        bool result = pdp_instance.load_program((char *)file_name.c_str());
        started = true;
        if(result != true) {
            exit(-1);
        }
    }

    if(finish != true) {
        instruction_content = pdp_instance.run_next_instruction();
        if(!final.compare(instruction_content -> disasm_command)) {
            finish = true;
        }

        if(buffer_ready == false) {
            buffer_ = instruction_content -> VRAM;
            buffer_ready = true;
        } else {
            //free(instruction_content -> VRAM);
        }

        insert_row(ui);
        set_text_in_labels(ui, instruction_content);
        QTableWidgetItem *newItem_1 = new QTableWidgetItem();
        QTableWidgetItem *newItem_2 = new QTableWidgetItem();
        newItem_1->setText(QString::fromStdString(instruction_content -> disasm_command));
        newItem_2->setText(QString::number(instruction_content -> registers[7]));
        ui->tableWidget->setItem(num_of_rows-1, 0, newItem_1);
        ui->tableWidget->setItem(num_of_rows-1, 1, newItem_2);
        free(instruction_content);
    }
    if(finish == true) {
        insert_elements_refresh(ui);
        enable_false(ui);
        ui->pushButton_7->setEnabled(true);
    }
}

void MainWidget::on_pushButton_5_clicked()
{
    QString line_str = ui->lineEdit->text();
    if(line_str.isNull()){
        QMessageBox::information(this,"Image Viewer","Error Displaying image");
        return;
    }
    ui->lineEdit->clear();
    file_name.clear();
    file_name = line_str.toStdString();
    ui->pushButton_5->setEnabled(false);
    ui->lineEdit->setEnabled(false);
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_4->setEnabled(true);
}

void MainWidget::on_pushButton_6_clicked()
{
    reset_common(ui);

    int i = num_of_rows;
    while(i > 0) {
        i--;
        ui->tableWidget->removeRow(0);
    }
    ui->pushButton_7->setEnabled(false);
    buffer_ready = false;
    //free(buffer_);
    file_name.clear();
    num_of_rows = 0;
    breakpoint = -1;
}

void MainWidget::on_pushButton_7_clicked()
{
    ui->pushButton_5->setEnabled(false);
    ui->lineEdit->setEnabled(false);
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_4->setEnabled(true);
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
        int i = 0;
        do {
            if(pause_pressed == true) break;
            instruction_content = pdp_instance.run_next_instruction();
            compare = instruction_content -> disasm_command;
            if(!final.compare(compare)) {
                finish = true;
            }
            set_text_in_labels(ui, instruction_content);
            if(breakpoint != -1) {
                if(breakpoint == (int)(instruction_content -> registers[7])) {
                    finish = true;
                    started = false;
                    free(instruction_content);
                    row = i;
                    ui->tableWidget->item(i, 0)->setBackgroundColor("red");
                    ui->tableWidget->item(i, 1)->setBackgroundColor("red");
                    break;
                }
            }
            i++;
            free(instruction_content);
        } while(final.compare(compare));
        if((pause_pressed != true) && (finish == true)) {
            insert_elements_refresh(ui);
            enable_false(ui);
            ui->pushButton_7->setEnabled(true);
        }
    }
}

void MainWidget::on_tableWidget_cellDoubleClicked(int row, int column)
{
    breakpoint = (ui->tableWidget->item(row, column)->text()).toInt();
    ui->pushButton_8->setEnabled(true);
}

void MainWidget::on_pushButton_8_clicked()
{
    breakpoint = -1;
    ui->pushButton_8->setEnabled(false);
    ui->tableWidget->item(row, 0)->setBackgroundColor("white");
    ui->tableWidget->item(row, 1)->setBackgroundColor("white");
    row = -1;
}
