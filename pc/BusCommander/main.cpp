#include <QCoreApplication>
#include <QCommandLineParser>

#include "unfoldedCircle/unfoldedCircleRemote.h"
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

    parser.addOption(ipOption);
    parser.addOption(pinOption);

    parser.process(app);

    RoomBusInterface roomBusInterface;
    UnfoldedCircle::Server remote{&roomBusInterface};

    if(parser.isSet(ipOption)&&parser.isSet(pinOption)){
        const QString ip = parser.value(ipOption);
        const QString pin = parser.value(pinOption);

        remote.registerDriver(ip, pin.toInt());
    }

    return app.exec();
}
