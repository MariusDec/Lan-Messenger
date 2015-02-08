#include "themedbutton.h"

#include <QPainter>
#include <QMenu>
#include <QMouseEvent>
#include <QApplication>

#include "thememanager.h"

ThemedButton::ThemedButton(QWidget *parent, int ID, int menuID)
    : QToolButton(parent), _id(ID), _menuId(menuID), _style(Normal) {
  if (_id > -1)
    connect(this, &ThemedButton::clicked, [this]() { emit clickedId(_id); });

  //setStyleSheet(QStringLiteral("QToolButton { padding: 3px 0px; }"));
} // ThemedButton

void ThemedButton::setMenu(QMenu *menu) {
  QToolButton::setMenu(menu);

  connect(this->menu(), &QMenu::aboutToHide, [this]() {
    this->_selected = false;
    this->_hovered = false;
    this->repaint();
  });
} // setMenu

void ThemedButton::addAction(QAction *action) {
  if (!menu()) {
    QMenu *menu = new QMenu;
    menu->addAction(action);

    setMenu(menu);
  } else {
    menu()->addAction(action);
  }
} // addAction

void ThemedButton::addActions(QList<QAction *> actions) {
  if (!menu()) {
    QMenu *menu = new QMenu;
    menu->addActions(actions);

    setMenu(menu);
  } else {
    menu()->addActions(actions);
  }
} // addActions

void ThemedButton::enterEvent(QEvent *e) {
  if (ThemeManager::getInstance().useSystemTheme()) {
    QToolButton::enterEvent(e);
    return;
  }

  _hovered = true;
  repaint();
} // enterEvent

void ThemedButton::leaveEvent(QEvent *e) {
  if (ThemeManager::getInstance().useSystemTheme()) {
    QToolButton::leaveEvent(e);
    return;
  }

  _hovered = false;
  repaint();
} // leaveEvent

void ThemedButton::mousePressEvent(QMouseEvent *e) {
  if (e->button() != Qt::LeftButton)
    return;

  if (ThemeManager::getInstance().useSystemTheme()) {
    QToolButton::mousePressEvent(e);
    return;
  }

  _clicked = true;
  repaint();
} // mousePressEvent

void ThemedButton::mouseReleaseEvent(QMouseEvent *e) {
  if (e->button() != Qt::LeftButton)
    return;

  if (ThemeManager::getInstance().useSystemTheme()) {
    QToolButton::mouseReleaseEvent(e);
    return;
  }

  _clicked = false;

  QRect rect = geometry();
  rect.moveTopLeft(parentWidget()->mapToGlobal(
      rect.topLeft())); // get the absolute (screen) button coordinates
  if (rect.contains(
          QCursor::pos())) { // if the cursor is over the button when released
    _hovered = true;

    if (menu() and !menu()->isEmpty())
      _selected = true;

    repaint();
    click(); // simulate a click
  } else
    _hovered = false;
} // mouseReleaseEvent

void ThemedButton::paintEvent(QPaintEvent *e) {
  if (ThemeManager::getInstance().useSystemTheme()) {
    QToolButton::paintEvent(e);
    return;
  }

  QPainter painter(this);

     painter.setRenderHints(QPainter::Antialiasing |
     QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform |
     QPainter::HighQualityAntialiasing);

  // draw the background
  {
    if (!isEnabled()) {
      painter.drawPixmap(
          0, 0, 8, size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_disabled_left.png")));
      painter.drawPixmap(
          8, 0, (size().width() - 16), size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_disabled_middle.png")));
      painter.drawPixmap(
          (size().width() - 8), 0, 8, size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_disabled_right.png")));
    } else if (_clicked) {
      painter.drawPixmap(
          0, 0, 8, size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_clicked_left.png")));
      painter.drawPixmap(
          8, 0, (size().width() - 16), size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_clicked_middle.png")));
      painter.drawPixmap(
          (size().width() - 8), 0, 8, size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_clicked_right.png")));
    } else if (_selected) {
      painter.drawPixmap(
          0, 0, 8, size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_selected_left.png")));
      painter.drawPixmap(
          8, 0, (size().width() - 16), size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_selected_middle.png")));
      painter.drawPixmap(
          (size().width() - 8), 0, 8, size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_selected_right.png")));
    } else if (_hovered) {
      painter.drawPixmap(
          0, 0, 8, size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_hovered_left.png")));
      painter.drawPixmap(
          8, 0, (size().width() - 16), size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_hovered_middle.png")));
      painter.drawPixmap(
          (size().width() - 8), 0, 8, size().height(),
          QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                   "button_hovered_right.png")));
    } else { // normal
      if (!autoRaise()) {
        painter.drawPixmap(
            0, 0, 8, size().height(),
            QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                     "button_normal_left.png")));
        painter.drawPixmap(
            8, 0, (size().width() - 16), size().height(),
            QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                     "button_normal_middle.png")));
        painter.drawPixmap(
            (size().width() - 8), 0, 8, size().height(),
            QPixmap((ThemeManager::getInstance().currentButtonThemePath() +
                     "button_normal_right.png")));
      } else {
          painter.setBackground (QApplication::palette().window ());
          painter.setPen (Qt::transparent);
          painter.drawRect (rect ());
      }
    }
  } // draw the background

  // draw the text
  if (!text().isNull() and (toolButtonStyle() != Qt::ToolButtonIconOnly or
      icon().isNull())) {
    int iconWidth = 0; // to reserve space for the icon to be drawn
    if (!icon().isNull() and
        toolButtonStyle() != Qt::ToolButtonTextOnly) // if it has an icon, and
                                                     // the button style is not
                                                     // 'show only text'
      iconWidth = iconSize().width();

    painter.setFont(this->font());

    QFontMetrics fontMetrics = painter.fontMetrics(); // used to calculate the
                                                      // width and height of the
                                                      // text, in px

    QPoint center =
        QPoint(((width() + iconWidth) - fontMetrics.width(text())) /
                   2, // find the center (X-Y) of the button to center the text
               (height() + fontMetrics.height()) / 2);

    if (!isEnabled())
      painter.setPen(QColor(154, 160, 168));

    painter.drawText(center, text()); // paint the text
  }                                   // draw the text

  // draw the icon
  if (!icon().isNull() and
      (!text().isNull() or toolButtonStyle() != Qt::ToolButtonTextOnly)) {
      QSize pxSize = iconSize();
      if (_iconFitSize && toolButtonStyle() == Qt::ToolButtonIconOnly)
          pxSize = QSize(height() - 8, width() - 8);

    QPixmap pixmap =
        icon().pixmap(pxSize, isEnabled() ? QIcon::Normal : QIcon::Disabled,
                      isChecked() ? QIcon::On : QIcon::Off);

    if (toolButtonStyle() != Qt::ToolButtonIconOnly)
      painter.drawPixmap(2, ((height() - pixmap.height()) / 2), pixmap);
    else
      painter.drawPixmap(((width() - pixmap.width()) / 2),
                         ((height() - pixmap.height()) / 2), pixmap);
  } // draw the icon

  if (menu() and !menu()->isEmpty()) {
    painter.drawPixmap((size().width() - 13), (height() - 11), 9, 7,
                       QPixmap((ThemeManager::getInstance().currentThemePath() +
                                "button_arrow.png")));
  } // draw a small arrow in right bottom corner
} // paintEvent
