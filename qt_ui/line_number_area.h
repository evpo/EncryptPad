#pragma once

#include <QWidget> // Required for inheritance

class PlainTextEdit;

class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    explicit LineNumberArea(PlainTextEdit *parent = nullptr);

    // Disable copying
    LineNumberArea(const LineNumberArea &) = delete;
    LineNumberArea &operator=(const LineNumberArea &) = delete;

    QSize sizeHint() const override;


protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool getShowRelativeNumbers() const;
    PlainTextEdit *m_codeEditParent;
};
