#ifndef CCMDCONVENTER_H
#define CCMDCONVENTER_H
#include <QObject>
#include "ssettings.h"
#include <QQueue>
#include <QTimer>
#include <QIODevice>

/// На service-запросы нет таймаута
class CProtocolTransmitter : public QObject
{
    Q_OBJECT
public:
    CProtocolTransmitter(QIODevice *dev = nullptr, QObject *parent = nullptr);
    CProtocolTransmitter(QObject *parent = nullptr);
    ~CProtocolTransmitter() {}

    void setDevice(QIODevice *dev);
    void removeDevice();

    void getRequest(ESectionType type);
    void setRequest(const SSection &section);
    void serviceRequest(EServiceCommand cmd);

public slots:
    void readyRead();
    void requestTimeout();

signals:
    /// прочитали ответ на get-запрос
    void s_sectionRead(const SSection &sect);
    /// прочитали отладочное инфо
    void s_debugInfo(const QString &str);
    /// set-запрос SSection подтвержден (все поля <ok>)
    void s_requestConfirmed(const SSection &sect);
    /// set-запрос SSection не подтвержден (время ожидания превышено)
    void s_requestFailed(const SSection &sect);
    /// set-запрос SSection не подтвержден (ошибки)
    void s_requestError(QList<QString> errorFields);
    /// service-запрос не потвержден
    void s_serviceRequestFailed(QString name);
    /// service-запрос потвержден
    void s_serviceRequestConfirmed(QString name);

protected:
    /// отрезает все данные до начала секции и возвращает тип следующей
    ESectionType separateNoSection();
    /// читает секцию от [section name] до [end] и возвращает все, что внутри
    QByteArray readRawSection(QString sectionName);
    /// заполняет секцию заданного типа  из "сырых" данных
    void fillSection(const QByteArray &rawSection, SSection &sect);
    /// обрабатывает все подтверждения (подтверждено, не подтверждено)
    void processConfirmation(const SSection &sect);
    /// обрабатывает service-подтверждения (подтверждено, не подтверждено)
    void processServiceConfirmation(const SSection &sect);
    ///Имя следующего заголовка !!!Просто берет первое встречающееся слово, совпадающее со
    ///словарными именами секций
    QString nextSectionHeader(const QByteArray &data);

private:
    ///обрабатывает следующий в очереди запрос
    void processNextRequest();

private:
    QIODevice *m_device { nullptr };
    /// тут храним последний запрос
    QQueue<SSection> requests;
    /// таймер для получения ответа от устройства
    QTimer m_timer;
};

#endif // CCMDCONVENTER_H
