#include "stateReportProtocol.h"

stateReportProtocol::stateReportProtocol(busDevice *device):busProtocol(device)
{
}

void stateReportProtocol::pushData(busMessage msg)
{
    if(msg.protocol == Protocol::StateReportProtocol)
    {
        if(msg.command == stateReportChannelNameReporting) parseStateNameReport(msg);
        else if(msg.command == group0Report) parseGroup0Report(msg);
        else if(msg.command == individualStateReport) parseIndividualReport(msg);
    }
}

QList<Protocol> stateReportProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::StateReportProtocol);
    return temp;
}

void stateReportProtocol::reset(void)
{
    _stateReports.clear();
    _stateReportNames.clear();
}

void stateReportProtocol::parseGroup0Report(busMessage msg)
{
    for(uint8_t i = 0; i< msg.data.size(); i++)
    {
        state_t state0 = static_cast<state_t>((msg.data.at(i)&0x0F));
        state_t state1 = static_cast<state_t>(((msg.data.at(i)>>4)&0x0F));

        _stateReports[i*2] = state0;
        _stateReports[i*2+1] = state1;
    }

    emit stateChange();
}

void stateReportProtocol::parseIndividualReport(busMessage msg)
{
    for(uint8_t i = 0; i< msg.data.size(); i += 2)
    {
        uint8_t channel = static_cast<uint8_t>(msg.data.at(i));
        state_t state = static_cast<state_t>(msg.data.at(i+1));

        _stateReports[channel] = state;
    }

    emit stateChange();
}

void stateReportProtocol::parseStateNameReport(busMessage msg)
{
    uint8_t channelNr = static_cast<uint8_t>(msg.data.at(0));
    QString name = msg.data.remove(0,1);
    _stateReportNames[channelNr] = name;

    emit stateReportListChange();
}

void stateReportProtocol::requestStateReportChannelNames(void)
{
    busMessage msg;

    msg.protocol = Protocol::StateReportProtocol;
    msg.command = stateReportChannelNameRequest;

    for (uint8_t i = 0; i<64; i++)
    {
        msg.data.append(static_cast<char>(i));
    }

    sendMessage(msg);
}

void stateReportProtocol::requestState(uint8_t channel)
{
    busMessage msg;

    msg.protocol = Protocol::StateReportProtocol;
    msg.command = stateReportRequest;
    msg.data.append(static_cast<char>(channel));

    sendMessage(msg);
}

void stateReportProtocol::requestStateAll(void)
{
    busMessage msg;

    msg.protocol = Protocol::StateReportProtocol;
    msg.command = stateReportRequest;

    foreach(uint8_t channel, _stateReports.keys())
    {
        msg.data.append(static_cast<char>(channel));
    }

    sendMessage(msg);
}

QMap<uint8_t, QString> stateReportProtocol::stateReportNames() const
{
    return _stateReportNames;
}

QMap<uint8_t, stateReportProtocol::state_t> stateReportProtocol::stateReports() const
{
    return _stateReports;
}
