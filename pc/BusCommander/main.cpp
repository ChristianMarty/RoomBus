#include <QCoreApplication>
#include <QCommandLineParser>

#include "unfoldedCircle/server.h"
#include "roomBus/roomBus.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Unfolded Circle Remote 2 Integration Server");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption ipOption(QStringList("ip"),"IP address of the remote","ip");
    QCommandLineOption pinOption(QStringList("pin"),"IP address of the remote", "pin");
    QCommandLineOption comOption(QStringList("com"),"COM port of CANbeSerial interface", "com");
    QCommandLineOption canOption(QStringList("can"),"CAN interface", "can");

    parser.addOption(ipOption);
    parser.addOption(pinOption);
    parser.addOption(comOption);
    parser.addOption(canOption);


    parser.process(app);

    QString port;
    RoomBusInterface::Type type = RoomBusInterface::Type::Undefined;
    if(parser.isSet(comOption)){
        port = parser.value(comOption);
        type = RoomBusInterface::Type::Serial;
    }
    if(parser.isSet(canOption)){
        port = parser.value(canOption);
        type = RoomBusInterface::Type::Can;
    }

    RoomBusInterface roomBusInterface{type, port};
    UnfoldedCircle::Server remote{&roomBusInterface};

    if(parser.isSet(ipOption)&&parser.isSet(pinOption)){
        const QString ip = parser.value(ipOption);
        const QString pin = parser.value(pinOption);

        remote.registerDriver(ip, pin.toInt());
    }

    return app.exec();
}
