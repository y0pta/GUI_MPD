#include "cmpdelementwidget.h"

CMpdElementWidget::CMpdElementWidget(QString text, QWidget *pwgt) : QPushButton(pwgt)
{
    connect(&m_menu, &QMenu::triggered, this, &CMpdElementWidget::menuAction);
    _resetView(text);
}

void CMpdElementWidget::_resetView(QString text)
{
    setText(text);
    setState(m_state);
}

void CMpdElementWidget::menuAction(QAction *act)
{
    if (m_state == eDisconnected) {
        setState(eConnected);
    } else {
        setState(eDisconnected);
    }
}

void CMpdElementWidget::setState(EState st, bool active)
{
    // чистим меню
    m_menu.clear();
    //Меняем фон
    QString style_str;
    switch (st) {
    case eConnected:
        style_str = "QPushButton { background-color: #74E868; ";
        m_menu.addAction("Отсоединить");
        break;
    case eDisconnected:
        style_str = "QPushButton { background-color: #555555; ";
        m_menu.addAction("Подсоединить");
        break;
    case eError:
        style_str = "QPushButton { background-color: #FD7279; ";
        m_menu.addAction("Подсоединить");
        break;
    default:
        return;
        break;
    }
    m_state = st;

    //Меняем выделение границы
    if (active) {
        style_str += "border: 6px solid #5DCEC6; }";
    } else {
        style_str += "border: 2px solid #00665E; }";
    }

    setStyleSheet(style_str);
}

void CMpdElementWidget::setEnabled(bool en)
{
    enabled = en;
    if (!en)
        setState(eDisconnected);
}

void CMpdElementWidget::mousePressEvent(QMouseEvent *e)
{
    if (enabled) {
        // если нажали правую клавишу, показваем меню
        if (e->button() == Qt::RightButton) {
            auto pos = e->globalPos();
            m_menu.popup(e->globalPos());
        }
        // если нажали на левую клавишу, сигнализируем
        if (e->button() == Qt::LeftButton) {
            emit s_clicked();
        }
    }
}

void CMpdElementWidget::enterEvent(QEvent *event)
{
    if (enabled)
        setState(m_state, true);
}
void CMpdElementWidget::leaveEvent(QEvent *event)
{
    if (enabled)
        setState(m_state, false);
}
