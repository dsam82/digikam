/*
 *  Copyright (C) 2011 Tuomo Penttinen, all rights reserved.
 *
 *  Author: Tuomo Penttinen <tp@herqq.org>
 *
 *  This file is part of Herqq UPnP Av (HUPnPAv) library.
 *
 *  Herqq UPnP Av is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Herqq UPnP Av is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Herqq UPnP Av. If not, see <http://www.gnu.org/licenses/>.
 */

#include "hconnectionmanager_info.h"

#include "hresourcetype.h"
#include "hactions_setupdata.h"
#include "hstatevariables_setupdata.h"

namespace Herqq
{

namespace Upnp
{

namespace Av
{

/*!
 * \defgroup hupnp_av_cm Connection Manager
 * \ingroup hupnp_av
 *
 * \brief This page discusses the design and use of the HUPnPAv's ConnectionManager
 * functionality.
 */

/*******************************************************************************
 * HConnectionManagerInfo
 ******************************************************************************/
HConnectionManagerInfo::HConnectionManagerInfo()
{
}

HConnectionManagerInfo::~HConnectionManagerInfo()
{
}

HConnectionManagerInfo::Direction
    HConnectionManagerInfo::directionFromString(const QString& arg)
{
    Direction retVal = DirectionUndefined;
    if (arg.compare(QLatin1String("Input"), Qt::CaseInsensitive) == 0)
    {
        retVal = DirectionInput;
    }
    else if (arg.compare(QLatin1String("Output"), Qt::CaseInsensitive) == 0)
    {
        retVal = DirectionOutput;
    }
    return retVal;
}

QString HConnectionManagerInfo::directionToString(Direction arg)
{
    QString retVal;
    switch(arg)
    {
    case DirectionUndefined:
        break;
    case DirectionInput:
        retVal = QLatin1String("Input");
        break;
    case DirectionOutput:
        retVal = QLatin1String("Output");
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return retVal;
}

HConnectionManagerInfo::ConnectionStatus
    HConnectionManagerInfo::statusFromString(const QString& arg)
{
    ConnectionStatus retVal = StatusUnknown;
    if (arg.compare(QLatin1String("Ok"), Qt::CaseInsensitive) == 0)
    {
        retVal = StatusOk;
    }
    else if (arg.compare(QLatin1String("Unknown"), Qt::CaseInsensitive) == 0)
    {
        retVal = StatusUnknown;
    }
    else if (arg.compare(QLatin1String("ContentFormatMismatch"), Qt::CaseInsensitive) == 0)
    {
        retVal = StatusContentFormatMismatch;
    }
    else if (arg.compare(QLatin1String("InsufficientBandwidth"), Qt::CaseInsensitive) == 0)
    {
        retVal = StatusInsufficientBandwidth;
    }
    else if (arg.compare(QLatin1String("UnreliableChannel"), Qt::CaseInsensitive) == 0)
    {
        retVal = StatusUnreliableChannel;
    }
    return retVal;
}

QString HConnectionManagerInfo::statusToString(ConnectionStatus arg)
{
    QString retVal;
    switch(arg)
    {
    case StatusUnknown:
        retVal = QLatin1String("Unknown");
        break;
    case StatusOk:
        retVal = QLatin1String("OK");
        break;
    case StatusContentFormatMismatch:
        retVal = QLatin1String("ContentFormatMismatch");
        break;
    case StatusInsufficientBandwidth:
        retVal = QLatin1String("InsufficientBandwidth");
        break;
    case StatusUnreliableChannel:
        retVal = QLatin1String("UnreliableChannel");
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return retVal;
}

const HResourceType& HConnectionManagerInfo::supportedServiceType()
{
    static const HResourceType retVal(QLatin1String("urn:schemas-upnp-org:service:ConnectionManager:2"));
    return retVal;
}

HActionsSetupData HConnectionManagerInfo::actionsSetupData()
{
    HActionsSetupData retVal;

    retVal.insert(HActionSetup(QLatin1String("GetProtocolInfo")));
    retVal.insert(HActionSetup(QLatin1String("PrepareForConnection"), InclusionOptional));
    retVal.insert(HActionSetup(QLatin1String("ConnectionComplete"), InclusionOptional));

    retVal.insert(HActionSetup(QLatin1String("GetCurrentConnectionIDs")));
    retVal.insert(HActionSetup(QLatin1String("GetCurrentConnectionInfo")));

    return retVal;
}

HStateVariablesSetupData HConnectionManagerInfo::stateVariablesSetupData()
{
    HStateVariablesSetupData retVal;

    retVal.insert(HStateVariableInfo(QLatin1String("SourceProtocolInfo"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("SinkProtocolInfo"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("CurrentConnectionIDs"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_ConnectionStatus"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_ConnectionManager"),HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_ProtocolInfo"), HUpnpDataTypes::string));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_ConnectionID"), HUpnpDataTypes::i4));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_AVTransportID"), HUpnpDataTypes::i4));
    retVal.insert(HStateVariableInfo(QLatin1String("A_ARG_TYPE_RcsID"), HUpnpDataTypes::i4));

    return retVal;
}

}
}
}