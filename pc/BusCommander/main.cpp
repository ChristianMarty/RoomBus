#include <QCoreApplication>
#include "unfoldedCircle/unfoldedCircleRemote.h"

#include <QCommandLineParser>
#include "busAccess.h"

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

    UnfoldedCircle::Server remote;

    if(parser.isSet(ipOption)&&parser.isSet(pinOption)){
        const QString ip = parser.value(ipOption);
        const QString pin = parser.value(pinOption);

        remote.registerDriver(ip, pin.toInt());
    }

    RoomBusAccess busAccess;
    busAccess.openSerialConnection("COM8");

    return app.exec();
}
