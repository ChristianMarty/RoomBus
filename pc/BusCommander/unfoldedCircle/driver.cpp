#include "driver.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QHttpHeaders>

using namespace UnfoldedCircle;

Driver::Driver(QString ip, uint32_t port)
{
    _driverUrl = "ws://"+ip+":"+QString::number(port);
}

void Driver::sendDriverRegistration(QString remoteIp, uint32_t pin)
{
    QString version = QString::number(_version.major)+"."+QString::number(_version.minor)+"."+QString::number(_version.patch);

    QJsonObject name {{"en", _name}};
    QJsonObject description {{"en", _description}};
    QJsonObject json {
        {"driver_id", _driverId},
        {"name", name},
        {"driver_url", _driverUrl},
        {"version", version},
        {"enabled", true},
        {"description", description},
        {"device_discovery", _deviceDiscovery},
        {"setup_data_schema", {}},
        {"release_date", _releaseDate.toString("yyyy-MM-dd")}
    };

    QJsonDocument data;
    data.setObject(json);

    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest request;

    QByteArray userHeader = "Basic "+QString("web-configurator:" + QString::number(pin)).toLocal8Bit().toBase64();

    QHttpHeaders headers;
    headers.append(QHttpHeaders::WellKnownHeader::ContentType, "application/json");
    headers.append("Authorization", userHeader);

    request.setUrl(QUrl("http://"+remoteIp+"/api/intg/drivers"));
    request.setHeaders(headers);

    manager->post(request, data.toJson(QJsonDocument::Compact));
}

Version Driver::version() const
{
    return _version;
}

