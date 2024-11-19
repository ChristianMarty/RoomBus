#ifndef ENTITY_H
#define ENTITY_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

namespace UnfoldedCircle
{
class Remote;

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

    explicit Entity(QString name, int entityId, Type type);

    virtual void commandHandler(QJsonObject data) {Q_UNUSED(data)};

    virtual QJsonObject json(void) const;

    int entityId() const;

    void setRemote(Remote *newRemote);

signals:

protected:
    QJsonObject _json(QJsonArray features = QJsonArray{}) const;


    void _sendEntityChange(QJsonObject attributes = QJsonObject{});

private:
    Remote *_remote = nullptr;

    QString _name;
    int _entityId;
    Type _type;

    QString _typeToString(Type type) const;
};

}
#endif // ENTITY_H
