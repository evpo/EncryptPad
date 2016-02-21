#include "find_dialog.h"
#include "ui_find_dialog.h"
#include "common_definitions.h"

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setFindWhat(const QString &text)
{
    ui->uiFind->setText(text);
}

void FindDialog::on_uiFindNext_clicked()
{
    findNext(ui->uiFind->text(), ui->uiDown->isChecked(), ui->uiMatchCase->isChecked(), ui->uiWholeWord->isChecked());
}

void FindDialog::on_uiFind_textChanged(const QString &text)
{
    ui->uiFindNext->setEnabled(!text.isEmpty());
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
void FindDialog::on_FindDialog_finished(int result)
{
    ui->uiFind->setFocus();
}
#pragma GCC diagnostic pop
