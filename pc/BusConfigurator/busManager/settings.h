#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QtXml>
#include <QFile>

struct settings_t{
    struct connection_t{
        enum interface_t{
            serial,
            tcp,
            udp
        }interface;
        QString ip;
        QString port;
    }connection;
};

class settings : public QObject
{
    Q_OBJECT
public:
    explicit settings(QObject *parent = nullptr);

    void readFile(QString filePath);

    settings_t getSettings(void);

signals:

private:

    settings_t _settings;

};

#endif // SETTINGS_H
