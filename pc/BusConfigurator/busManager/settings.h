#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QtXml>
#include <QFile>


class Settings
{
public:

    struct Data{
        struct Connection{
            enum Interface{
                Serial,
                Tcp,
                Udp,
                SocketCan
            }interface;
            QString ip;
            QString port;
        }connection;
    };

    void readFile(QString filePath);

    Settings::Data getSettings(void);

private:
    Data _data;
};

#endif // SETTINGS_H
