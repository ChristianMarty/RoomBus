#ifndef ENTITY_H
#define ENTITY_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

namespace MiniBus {
    class Message;
}

namespace UnfoldedCircle
{
class Server;

class Entity : public QObject
{
    Q_OBJECT
public:

    enum class Type {
        buttonEntity,
        climateEntity,
        coverEntity,
        lightEntity,
        mediaPlayerEntity,
        sensorEntity,
        switchEntity,
        activityEntity,
        macroEntity,
        remoteEntity,
        irEmitterEntity
    };

    explicit Entity(QString name, Type type);

    virtual void commandHandler(QJsonObject data) {Q_UNUSED(data)};

    virtual void stateSystemHandler(uint16_t channel, uint8_t state){Q_UNUSED(channel); Q_UNUSED(state);};
    virtual void valueSystemHandler(const MiniBus::Message &message){Q_UNUSED(message)};

    virtual QJsonObject json(void) const;

    int entityId() const;

    void setServer(Server *newServer, int entityId);

signals:

protected:
    QJsonObject _json(QJsonArray features = QJsonArray{}) const;


    void _sendEntityChange(QJsonObject attributes = QJsonObject{});
    Server *_server = nullptr;

private:
    QString _name;
    int _entityId;
    Type _type;

    QString _typeToString(Type type) const;
};

}
#endif // ENTITY_H
