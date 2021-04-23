#include "find_dialog.h"
#include <QtGlobal>
#include "ui_find_dialog.h"
#include "common_definitions.h"

const char *kFindDialogWidth = QT_TRANSLATE_NOOP("FindDialog", "425");

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent, kDefaultWindowFlags),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
    setMinimumSize(sizeHint());
    adjustSize();
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::setFindWhat(const QString &text)
{
    ui->uiFind->setText(text);
}

void FindDialog::setFindFocus()
{
    ui->uiFind->setFocus();
}

void FindDialog::on_uiFindNext_clicked()
{
    findNext(ui->uiFind->text(), ui->uiDown->isChecked(), ui->uiMatchCase->isChecked(), ui->uiWholeWord->isChecked());
}

void FindDialog::on_uiFind_textChanged(const QString &text)
{
    ui->uiFindNext->setEnabled(!text.isEmpty());
}

void FindDialog::on_FindDialog_finished(int result)
{
    (void)result;
    setFindFocus();
}
