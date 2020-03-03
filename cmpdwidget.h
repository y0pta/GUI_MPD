#ifndef CMPDWIDGET_H
#define CMPDWIDGET_H
#include <QWidget>
#include <cmpdelementwidget.h>

//Класс модели МПД
class CMpdWidget : public QWidget
{
public:
    CMpdWidget(QWidget *pwgt = 0);

public slots:

signals:
    //испускается, когда нажали на какой-то из элементов
    void s_clicked(QString nameEl);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void _setView(int w, int h);

private:
    CMpdElementWidget *m_centralElement;
    QList<CMpdElementWidget *> m_elements;
};

#endif // CMPDWIDGET_H
