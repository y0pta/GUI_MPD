#include "cmpdelementwidget.h"
const QString COLOR_ERROR = "#A40835";
const QString COLOR_CONNECTED = "#5BC30B";
const QString COLOR_DISCONNECTED = "#313737";
const QString COLOR_FREEZED_ERROR = "#A03C45";
const QString COLOR_FREEZED_CONNECTED = "#3CA03C";
const QString COLOR_FREEZED_DISCONNECTED = "#515757";
const QString COLOR_FREEZED_DISABLED = "#8C8C8C";
const QString COLOR_DISABLED = "#515757";
const QString COLOR_BORDER_ACTIVE = "#5DCEC6";
const QString COLOR_BORDER_DISACTIVE = "#00665E";

const QString CONNECT = "Подсоединить";
const QString DISCONNECT = "Отсоединить";

CMpdElementWidget::CMpdElementWidget(QString text, QWidget *pwgt) : QPushButton(pwgt)
{
    connect(&m_menu, &QMenu::triggered, this, &CMpdElementWidget::menuAction);
    connect(&m_menu, &QMenu::aboutToHide, this, &CMpdElementWidget::_menuClosed);
    m_menu.addAction(CONNECT);
    m_menu.addAction(DISCONNECT);

    _resetView(text);
}

void CMpdElementWidget::_resetView(QString text)
{
    setText(text);
    setState(m_state);
}

void CMpdElementWidget::_setView()
{
    //Меняем фон и границу stylesheet'ами
    QString style_str;

    style_str = "QPushButton { color: white; background-color: ";
    if (!freezed || active)
        switch (m_state) {
        case EState::eConnected:
            style_str += COLOR_CONNECTED;
            break;
        case EState::eDisconnected:
            style_str += COLOR_DISCONNECTED;
            break;
        case EState::eError:
            style_str += COLOR_ERROR;
            break;
        case EState::eDisabled:
            style_str += COLOR_DISABLED;
            break;
        }
    else
        switch (m_state) {
        case EState::eConnected:
            style_str += COLOR_FREEZED_CONNECTED;
            break;
        case EState::eDisconnected:
            style_str += COLOR_FREEZED_DISCONNECTED;
            break;
        case EState::eError:
            style_str += COLOR_FREEZED_ERROR;
            break;
        case EState::eDisabled:
            style_str += COLOR_FREEZED_DISABLED;
            break;
        }

    //Меняем выделение границы
    if (active && enabled) {
        style_str += "; border: 6px solid #5DCEC6; }";
    } else {
        style_str += "; border: 2px solid #00665E; }";
    }

    setStyleSheet(style_str);
}

void CMpdElementWidget::_menuClosed()
{
    if (underMouse())
        setActive(true);
    else
        setActive(false);
}

void CMpdElementWidget::menuAction(QAction *act)
{
    if (m_state == EState::eConnected && act->text() == DISCONNECT) {
        emit s_wantChangeState(EState::eDisconnected);
    }
    if (m_state == EState::eDisconnected && act->text() == CONNECT) {
        setActive(true);
        emit s_wantChangeState(EState::eConnected);
    }
}

void CMpdElementWidget::setState(EState st)
{
    if (!freezed) {
        m_state = st;
        _setView();
    }
}

void CMpdElementWidget::setActive(bool st)
{
    if (!freezed) {
        active = st;
        _setView();
    }
}

void CMpdElementWidget::setFreeze(bool st)
{
    freezed = st;
    _setView();
}

void CMpdElementWidget::setEnabled(bool en)
{
    if (!freezed) {
        enabled = en;
        setState(m_state);
    }
}

void CMpdElementWidget::mousePressEvent(QMouseEvent *e)
{
    if (enabled && !freezed) {
        // если нажали правую клавишу, показываем меню
        if (e->button() == Qt::RightButton) {
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
    Q_UNUSED(event)
    if (enabled && !freezed) {
        active = true;
        _setView();
    }
}
void CMpdElementWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    if (enabled && !m_menu.isVisible() && !freezed) {
        active = false;
        _setView();
    }
}
