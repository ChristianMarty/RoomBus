#include "settings.h"

settings::settings(QObject *parent)
    : QObject{parent}
{

}

void settings::readFile(QString filePath)
{
    QDomDocument xmlSettings;

    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly ))
    {
        xmlSettings.setContent(&f);
    }
    f.close();

    //QDomElement connection=xmlSettings.documentElement();
    QDomNodeList nodes = xmlSettings.elementsByTagName("connection");

    for(auto i = 0; i < nodes.count(); i++)
    {
        QString type=nodes.at(i).toElement().attribute("type","");
        QDomElement portElement = nodes.at(i).firstChildElement("port");
        QDomElement ipElement = nodes.at(i).firstChildElement("ip");

        if(type.toLower() == "tcp")
        {
            _settings.connection.interface = settings_t::connection_t::tcp;
            _settings.connection.ip = ipElement.text();
            _settings.connection.port = portElement.text();
        }
        if(type.toLower() == "udp")
        {
            _settings.connection.interface = settings_t::connection_t::udp;
            _settings.connection.ip = ipElement.text();
            _settings.connection.port = portElement.text();
        }
        else if(type.toLower() == "serial")
        {
            _settings.connection.interface = settings_t::connection_t::serial;
            _settings.connection.port = portElement.text();
        }
    }
}

settings_t settings::getSettings()
{
    return _settings;
}


