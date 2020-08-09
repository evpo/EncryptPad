#pragma once
#include <QEvent>
#include <QObject>
#include <QPlainTextEdit>
#include <QUrl>

class EpadTextEdit : public QPlainTextEdit
{
    Q_OBJECT

    public:
    explicit EpadTextEdit(QWidget *parent = nullptr);

    protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

    signals:
    void urlDropped(QUrl);
    void leaveControl();
};
