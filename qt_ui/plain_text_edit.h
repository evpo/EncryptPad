#ifndef PLAIN_TEXT_EDIT_H
#define PLAIN_TEXT_EDIT_H

#include <QPlainTextEdit>

class PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    typedef QPlainTextEdit super;
    explicit PlainTextEdit(QWidget *parent = 0);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dragMoveEvent(QDragMoveEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
signals:
    void urlDropped(QUrl url);
    void leaveControl();
    void enterControl();

public slots:

};

#endif // PLAIN_TEXT_EDIT_H
