#include "cmpdwidget.h"
#include <QDebug>

CMpdWidget::CMpdWidget(QWidget *pwgt) : QWidget(pwgt)
{
    setMinimumSize(350, 180);
    qDebug() << "size " << size();
    _setView(size().width(), size().height());
}

void CMpdWidget::_setView(int w, int h)
{
    QRect centralRect;
    centralRect.setWidth(w / 3);
    centralRect.setHeight(h);
    centralRect.moveCenter(QPoint(w / 2, h / 2));

    //Располагаем МПД по середине виджета
    m_centralElement = new CMpdElementWidget("МПД", this);
    m_centralElement->setGeometry(centralRect);
    m_centralElement->setEnabled(false);

    QRect rect;
    rect.setWidth(w / 3);
    rect.setHeight(h / 5);

    // Располагаем и задаем имена других виджетов
    auto el = new CMpdElementWidget("RS-232", this);
    rect.moveTopLeft(centralRect.topRight());
    rect.translate(0, rect.height());
    el->setGeometry(QRect(rect));
    m_elements.append(el);
    el = new CMpdElementWidget("RS-485", this);
    rect.moveBottomLeft(centralRect.bottomRight());
    rect.translate(0, -rect.height());
    el->setGeometry(rect);
    m_elements.append(el);
    el = new CMpdElementWidget("Radio", this);
    rect.moveCenter(centralRect.center());
    rect.translate(-centralRect.width(), 0);
    el->setGeometry(rect);
    m_elements.append(el);
}

void CMpdWidget::resizeEvent(QResizeEvent *event)
{
    _setView(event->size().width(), event->size().height());
}
