#ifndef CCMDCONVENTER_H
#define CCMDCONVENTER_H
#include <QObject>
#include "ssettings.h"
#include <QQueue>
#include <QTimer>
#include <QIODevice>

class CProtocolTransmitter : public QObject
{
    Q_OBJECT
public:
    CProtocolTransmitter(QIODevice *dev, QObject *parent = nullptr);
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
    void s_sectionRead(const SSection &sect);
    void s_noSectionRead(const QString &str);
    void s_requestConfirmed(const SSection &sect);
    void s_requestFailed(const SSection &sect);
    void s_serviceRequestFailed(QString name);
    void s_requestError(QList<QString> errorFields);

protected:
    /// отрезает все данные до начала секции и возвращает тип следующей
    ESectionType separateNoSection();
    /// читает секцию от [section name] до [end] и возвращает все, что внутри
    QByteArray readRawSection(QString sectionName);
    /// заполняет секцию заданного типа  из "сырых" данных
    template<typename SSectionClass>
    static void fillSection(const QByteArray &rawSection, SSectionClass &sect);
    /// обрабатывает подтверждения (подтверждено, не подтверждено)
    void processConfirmation(const SSection &sect);
    ///Имя следующего заголовка !!!Просто берет первое встречающееся слово, совпадающее со
    ///словарными именами секций
    QString nextSectionHeader(const QByteArray &data);

private:
    QIODevice *m_device { nullptr };
    /// тут храним последний запрос
    SSection lastReq;
    /// таймер для получения ответа от устройства
    QTimer m_timer;
};

#endif // CCMDCONVENTER_H
