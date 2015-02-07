#ifndef THEMEDCOMBOBOX_H
#define THEMEDCOMBOBOX_H

#include <QComboBox>

class ThemedComboBox : public QComboBox
{
    Q_OBJECT

    bool _hovered;
    bool _clicked;

public:
    explicit ThemedComboBox(QWidget *parent = 0);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);
    
signals:
    void currentIndexChanged(int);
};

#endif // THEMEDCOMBOBOX_H
