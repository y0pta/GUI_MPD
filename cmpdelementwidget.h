#ifndef CMPDELEMENT_H
#define CMPDELEMENT_H

#include <QPushButton>
#include <QMenu>
#include <QMouseEvent>

// Класс элемента модели МПД (немного измененная кнопка)

class CMpdElementWidget : public QPushButton
{
    Q_OBJECT
    // Возможные состояния
    enum EState { eConnected, eDisconnected, eError };

public:
    CMpdElementWidget(QWidget *pwgt = 0) : QPushButton(pwgt) { _resetView(); }
    CMpdElementWidget(QString text, QWidget *pwgt = 0);

    void setEnabled(bool);
    // смена состояния и внешнего вида виджета
    void setState(EState st, bool active = 0);

public slots:
    void menuAction(QAction *act);

signals:
    void s_clicked();

protected:
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

private:
    // возвращает виджет в исходное состояние
    void _resetView(QString text = QString());

private:
    QMenu m_menu;
    EState m_state { eDisconnected };
    bool enabled { true };
};

#endif // CMPDELEMENT_H
