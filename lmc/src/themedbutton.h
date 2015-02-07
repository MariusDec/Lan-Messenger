
#ifndef THEMEDBUTTON_H
#define THEMEDBUTTON_H

#include <QToolButton>

class ThemedButton : public QToolButton
{
    Q_OBJECT

    int _id;        // button id
    int _menuId;    // for module menus

    bool _hovered = false;
    bool _clicked = false;
    bool _selected = false; // true when the menu is shown
    bool _iconFitSize = false;

public:
    enum ButtonStyle { Normal, LeftSide, MiddleSide, RightSide };
    ButtonStyle _style;

    ThemedButton(QWidget *parent = 0, int ID = -1, int menuID = -1);

    int menuId() { return _menuId; }
    int id() { return _id; }

    void setMenu(QMenu *menu);
    void addAction(QAction *action);
    void addActions(QList<QAction *> actions);

    void setIconFitSize (bool fit) { _iconFitSize = fit; }
    bool iconFitSize () { return _iconFitSize; }

    void setButtonStyle(const ButtonStyle &style) { _style = style; }
    ButtonStyle buttonStyle() { return _style; }

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);

signals:
    void clickedId(int);
};

#endif // THEMEDBUTTON_H
