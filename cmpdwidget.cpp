#include "cmpdwidget.h"
#include <QDebug>

CMpdWidget::CMpdWidget(QWidget *pwgt, const QString &mainName) : QWidget(pwgt)
{
    setMinimumSize(350, 180);
    qDebug() << "size " << size();
    m_main = new CMpdElementWidget(this);
    m_main->setText(mainName);
    m_main->setEnabled(false);

    _refreshView();
}

void CMpdWidget::elementClicked()
{
    auto el = qobject_cast<CMpdElementWidget *>(sender());
    emit s_clicked(el->text());
}

void CMpdWidget::addElements(uint num, QList<QString> names)
{
    for (uint i = 0; i < num; i++) {
        QString name = names.value(i);
        // если не задано имя для данного элемента, используем номер
        if (name == QString())
            name = QString::number(i);
        m_elements[name] = new CMpdElementWidget(name, this);
        connect(m_elements[name], &CMpdElementWidget::s_clicked, this, &CMpdWidget::elementClicked);
        connect(m_elements[name], &CMpdElementWidget::s_wantChangeState, this,
                &CMpdWidget::elementWantChangeState);
    }
    _refreshView();
}

void CMpdWidget::eraseElements(QList<QString> names)
{
    for (auto name : names) {
        disconnect(m_elements[name], &CMpdElementWidget::s_clicked, this,
                   &CMpdWidget::elementClicked);
        m_elements.remove(name);
    }
    _refreshView();
}

void CMpdWidget::setTextName(QString oldName, QString newName)
{
    // если такой элемент существует, меняем имя. Если нет, то ничего не делаем
    if (m_elements.find(oldName) != m_elements.end()) {
        auto el = m_elements[oldName];
        m_elements.take(oldName);
        m_elements.insert(newName, el);
    }
}

void CMpdWidget::setMainTextName(const QString &name)
{
    m_main->setText(name);
}

void CMpdWidget::_refreshView()
{
    auto w = size().width();
    auto h = size().height();

    // задаем главный прямоугольник
    QRect centralRect;
    centralRect.setWidth(w / 3);
    centralRect.setHeight(h);
    centralRect.moveCenter(QPoint(w / 2, h / 2));

    //Располагаем МПД по середине виджета
    m_main->setGeometry(centralRect);

    if (m_elements.size() > 0) {
        // узнаем количество элементов, которое нужно разместить по бокам главного элемента
        int num = m_elements.size();
        // слева от виджета и справа будет по num_left и num_right элементов. Есди num нечетное, то
        // справа больше
        int num_left = num / 2;
        int num_right = num / 2;
        if (num % 2 != 0)
            ++num_right;

        // узнаем высоту и задаем прямоугольник виджетов
        QRect rect;
        int w_el = w / 3;
        int h_el = h / (num_right * 2);
        rect.setWidth(w_el);
        rect.setHeight(h_el);

        //Начинаем заполнять с левого верхнего угла центрального виджета
        QPoint cur_point = centralRect.topLeft();
        cur_point -= QPoint(w_el / 2, 0);
        rect.moveCenter(cur_point);

        // Располагаем и задаем имена других виджетов
        auto it = m_elements.begin();
        for (int i = 0; i < num_left; i++) {
            cur_point += QPoint(0, h_el);

            rect.moveCenter(cur_point);
            it.value()->setGeometry(rect);
            it++;
            cur_point += QPoint(0, h_el);
        }

        cur_point = centralRect.topRight();
        cur_point += QPoint(w_el / 2, 0);
        rect.moveCenter(cur_point);

        for (int i = 0; i < num_right; i++) {
            cur_point += QPoint(0, h_el);
            rect.moveCenter(cur_point);
            it.value()->setGeometry(rect);
            it++;
            cur_point += QPoint(0, h_el);
        }
    }
}

void CMpdWidget::resizeEvent(QResizeEvent *event) {}

void CMpdWidget::elementWantChangeState(EState st)
{
    auto element = qobject_cast<CMpdElementWidget *>(sender());
    emit s_wantChangeState(m_elements.key(element), st);
    element->setEnabled(false);
}

void CMpdWidget::changeState(QString nameEl, EState st)
{
    m_elements[nameEl]->setState(st);
    m_elements[nameEl]->setEnabled(true);
}

void CMpdWidget::setEnabled(bool state)
{
    // m_main->setEnabled(state);
    if (m_elements.size() != 0)
        for (auto el : m_elements) {
            el->setEnabled(state);
        }
}
