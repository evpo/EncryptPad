#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();

    void setFindWhat(const QString &text);

private:
    Ui::FindDialog *ui;

signals:
    void findNext(QString text, bool down, bool matchCase, bool wholeWord);
private slots:
    void on_uiFindNext_clicked();
    void on_uiFind_textChanged(const QString &text);
    void on_FindDialog_finished(int result);
};

#endif // FIND_DIALOG_H
