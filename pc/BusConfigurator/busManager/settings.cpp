#include "settings.h"

void Settings::readFile(QString filePath)
{
    QDomDocument xmlSettings;

    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly )){
        xmlSettings.setContent(&f);
    }
    f.close();

    QDomNodeList nodes = xmlSettings.elementsByTagName("connection");

    for(auto i = 0; i < nodes.count(); i++)
    {
        QString type = nodes.at(i).toElement().attribute("type","").toLower();
        QDomElement portElement = nodes.at(i).firstChildElement("port");
        QDomElement ipElement = nodes.at(i).firstChildElement("ip");

        if(type == "tcp"){
            _data.connection.interface = Data::Connection::Tcp;
            _data.connection.ip = ipElement.text();
            _data.connection.port = portElement.text();
        }if(type == "udp"){
            _data.connection.interface = Data::Connection::Udp;
            _data.connection.ip = ipElement.text();
            _data.connection.port = portElement.text();
        }else if(type == "serial"){
            _data.connection.interface = Data::Connection::Serial;
            _data.connection.port = portElement.text();
        }else if(type == "socketcan"){
            _data.connection.interface = Data::Connection::SocketCan;
            _data.connection.port = portElement.text();
        }
    }
}

Settings::Data Settings::getSettings()
{
    return _data;
}
