#ifndef CMPDWIDGET_H
#define CMPDWIDGET_H
#include <QWidget>
#include <cmpdelementwidget.h>

//Класс модели МПД
class CMpdWidget : public QWidget {
    Q_OBJECT
public:
    CMpdWidget(QWidget* pwgt = 0, const QString& mainName = "МПД");

    ///  устанавливает количество перефирийных элементов
    void addElements(uint num, QList<QString> names = QList<QString>());
    void eraseElements(QList<QString> names);
    /// меняет имя существуего элемента
    void setTextName(QString oldName, QString newName);
    /// устанавливает имя главного элемента
    void setMainTextName(const QString& name);
    /// меняет статус элемента
    void changeState(QString nameEl, EState st);
    ///устанавнивает элемент в state. Если элемент не укзан, то устанавливает весь виджет
    void setEnabled(bool state, QString nameEl = "");
    ///замораживает/размораживает все элементы
    void setFreeze(bool st);

public slots:
    /// обработчик нажатия на элемент
    void elementClicked();
    /// обработчик запроса от элемента на смену статуса
    void elementWantChangeState(EState);

signals:
    ///испускается, когда нажали на какой-то из элементов
    void s_clicked(const QString& nameEl);
    ///испускается, когда элемент хочет сменить статус (коннект на дисконнект, например)
    void s_wantChangeState(QString nameEl, EState st);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void _refreshView();

private:
    CMpdElementWidget* m_main;
    QMap<QString, CMpdElementWidget*> m_elements;
};

#endif // CMPDWIDGET_H
