#ifndef CCMDCONVENTER_H
#define CCMDCONVENTER_H
#include "ssettings.h"
#include <QIODevice>
#include <QObject>
#include <QQueue>
#include <QTimer>

class CProtocolTransmitter : public QObject {
    Q_OBJECT
public:
    CProtocolTransmitter(QIODevice* dev = nullptr, QObject* parent = nullptr);
    CProtocolTransmitter(QObject* parent = nullptr);
    ~CProtocolTransmitter() { }

    void setDevice(QIODevice* dev);
    void removeDevice();

    void getRequest(ESectionType type);
    void setRequest(const SSection& section);
    void serviceRequest(EServiceCommand cmd);

public slots:
    void readyRead();
    void requestTimeout();

signals:
    /// прочитали ответ на get-запрос
    void s_sectionRead(const SSection& sect);
    /// прочитали отладочное инфо
    void s_debugInfo(const QString& str);
    /// set-запрос SSection подтвержден (все поля <ok>)
    void s_requestConfirmed(const SSection& sect);
    /// set-запрос SSection не подтвержден (время ожидания превышено)
    void s_requestFailed(const SSection& sect);
    /// set-запрос SSection не подтвержден (ошибки)
    void s_requestError(const SSection& req);
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
    void fillSection(const QByteArray& rawSection, SSection& sect);
    /// обрабатывает все подтверждения (подтверждено, не подтверждено), возвращает true, если
    /// подтверждение соответствует запросу
    bool processConfirmation(const SSection& request, SSection& answer);
    /// обрабатывает service-подтверждения (подтверждено, не подтверждено)
    bool processServiceConfirmation(const SSection& request, const SSection& answer);
    ///отправляет следующий в очереди запрос, если устройство доступно
    bool processNextRequest();
    ///ставит запрос в очередь
    void pushQueu(const SSection& sect);
    ///удаляет первый запрос из очереди
    SSection popQueu();
    ///первый завопрос из очереди
    SSection lastQueu();

private:
    QIODevice* m_device { nullptr };
    /// тут храним последний запрос
    QQueue<SSection> requests;
    bool reqProcessing { false };
    /// таймер для получения ответа от устройства
    QTimer m_timer;
};

#endif // CCMDCONVENTER_H
