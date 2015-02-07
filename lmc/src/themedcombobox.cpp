#include "themedcombobox.h"

#include <QPainter>
#include <QMouseEvent>
#include <thememanager.h>

ThemedComboBox::ThemedComboBox(QWidget *parent) :
    QComboBox(parent), _hovered(false), _clicked(false)
{
    connect(this, static_cast<void (QComboBox::*)(int)> (&QComboBox::currentIndexChanged), this, &ThemedComboBox::currentIndexChanged);
} // ThemedComboBox

void ThemedComboBox::enterEvent(QEvent *e)
{
    if (ThemeManager::getInstance ().useSystemTheme ()) {
        QComboBox::enterEvent(e);
        return;
    }

    _hovered = true;
    repaint();
} // enterEvent

void ThemedComboBox::leaveEvent(QEvent *e)
{
    if (ThemeManager::getInstance ().useSystemTheme ()) {
        QComboBox::leaveEvent(e);
        return;
    }

    _hovered = false;
    repaint();
} // leaveEvent

void ThemedComboBox::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    if (ThemeManager::getInstance ().useSystemTheme ()) {
        QComboBox::mousePressEvent(e);
        return;
    }

  //  _clicked = false; // the menu is shown immediatly on mouse press, so mouseReleaseEvent is never fired, also the user doesn't see the clicked state
    _hovered = false;
    repaint();
    QComboBox::mousePressEvent(e);
} // mousePressEvent

void ThemedComboBox::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    if (ThemeManager::getInstance ().useSystemTheme ()) {
        QComboBox::mouseReleaseEvent(e);
        return;
    }

  //  _clicked = false;
    _hovered = false;
    repaint();
    QComboBox::mouseReleaseEvent(e);
} // mouseReleaseEvent

void ThemedComboBox::paintEvent(QPaintEvent *e)
{
    if (ThemeManager::getInstance ().useSystemTheme ()) {
        QComboBox::paintEvent(e);
        return;
    }

    QPainter painter(this);

    // draw the background
    {
        if (!isEnabled()) {
            painter.drawPixmap(0, 0, 8, size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_disabled_left.png")) );
            painter.drawPixmap(8, 0, (size().width() - 16), size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_disabled_middle.png")) );
            painter.drawPixmap((size().width() - 8), 0, 8, size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_disabled_right.png")) );
        } else if (_hovered) {
            painter.drawPixmap(0, 0, 8, size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_hovered_left.png")) );
            painter.drawPixmap(8, 0, (size().width() - 16), size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_hovered_middle.png")) );
            painter.drawPixmap((size().width() - 8), 0, 8, size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_hovered_right.png")) );
        } else { // normal
            painter.drawPixmap(0, 0, 8, size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_normal_left.png")) );
            painter.drawPixmap(8, 0, (size().width() - 16), size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_normal_middle.png")) );
            painter.drawPixmap((size().width() - 8), 0, 8, size().height(), QPixmap((ThemeManager::getInstance ().currentButtonThemePath() + "button_normal_right.png")) );
        }
    } // draw the background

    // draw the text
    if (!currentText().isNull()) {
        painter.setFont(this->font());

        QFontMetrics fontMetrics = painter.fontMetrics(); // used to calculate the width and height of the text, in px

        QPoint center_y = QPoint( 10, // find the center (Y) of the button to center the text
                                (height() + fontMetrics.height()) / 2 );

        if (!isEnabled())
            painter.setPen(QColor(154, 160, 168));

        painter.drawText(center_y, currentText()); // paint the text
    } // draw the text

    painter.drawPixmap((size().width() - 15), 0, 15, size().height(), QPixmap((ThemeManager::getInstance ().currentThemePath ()+ "ComboBox_arrows.png")) );
} //paintEvent
