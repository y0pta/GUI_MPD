#ifndef CMPDELEMENT_H
#define CMPDELEMENT_H

#include <QPushButton>
#include <QMenu>
#include <QMouseEvent>
#include <ssettings.h>

// Класс элемента модели МПД (немного измененная кнопка)

class CMpdElementWidget : public QPushButton
{
    Q_OBJECT
public:
    CMpdElementWidget(QWidget *pwgt = 0) : QPushButton(pwgt) { _resetView(); }
    CMpdElementWidget(QString text, QWidget *pwgt = 0);

    void setEnabled(bool);
    // смена состояния и внешнего вида виджета
    void setState(EState st);
    // устанавливает выделение
    void setActive(bool st);
    // замораживает текущее состояние
    void setFreeze(bool st);

public slots:
    void menuAction(QAction *act);

signals:
    void s_clicked();
    void s_wantChangeState(EState st);
    void s_menuVisible();

protected:
    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

private:
    // возвращает виджет в исходное состояние
    void _resetView(QString text = QString());
    // устанавливает вид в зависимости state
    void _setView();
private slots:
    void _menuClosed();

private:
    QMenu m_menu;
    EState m_state { EState::eDisconnected };
    bool enabled { true };
    bool active { false };
    bool freezed { false };
};

#endif // CMPDELEMENT_H
