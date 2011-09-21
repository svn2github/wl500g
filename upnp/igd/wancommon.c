/*
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"

#include "bcmnvram.h"
#include "bcmutils.h"

static int WANCommonInterfaceConfig_Init(PService psvc, service_state_t state);
static int WANCommonInterfaceConfig_GetVar(struct Service *psvc, int varindex);
static void WANCommonInterface_UpdateStats(timer_t t, PService psvc);

extern osl_link_t osl_link_status(char *devname);
extern uint osl_max_bitrates(char *devname, ulong *rx, ulong *tx);

#define GetCommonLinkProperties		DefaultAction

#define NewTotalBytesSent		DefaultAction
#define GetTotalBytesReceived		DefaultAction
#define GetTotalPacketsSent		DefaultAction
#define GetTotalPacketsReceived		DefaultAction
#define GetActiveConnection		DefaultAction
#define GetTotalBytesSent		DefaultAction

#define VAR_WANAccessType		0
#define VAR_Layer1UpstreamMaxBitRate		1
#define VAR_Layer1DownstreamMaxBitRate		2
#define VAR_PhysicalLinkStatus		3
#define VAR_EnabledForInternet		4
#define VAR_TotalBytesSent		5
#define VAR_TotalBytesReceived		6
#define VAR_TotalPacketsSent		7
#define VAR_TotalPacketsReceived		8

static char *WANAccessType_allowedValueList[] = { "DSL", "POTS", "Cable", "Ethernet", "Other", NULL };
static char *PhysicalLinkStatus_allowedValueList[] = { "Up", "Down", "Initializing", "Unavailable", NULL };


static VarTemplate StateVariables[] = {
    { "WANAccessType", "Ethernet", VAR_STRING|VAR_LIST,  (allowedValue) { WANAccessType_allowedValueList } },
    { "Layer1UpstreamMaxBitRate", "", VAR_ULONG },
    { "Layer1DownstreamMaxBitRate", "", VAR_ULONG },
    { "PhysicalLinkStatus", "", VAR_EVENTED|VAR_STRING|VAR_LIST,  (allowedValue) { PhysicalLinkStatus_allowedValueList } },
    { "EnabledForInternet", "1", VAR_EVENTED|VAR_BOOL },
    { "TotalBytesSent", "", VAR_ULONG },
    { "TotalBytesReceived", "", VAR_ULONG },
    { "TotalPacketsSent", "", VAR_ULONG },
    { "TotalPacketsReceived", "", VAR_ULONG },
    { NULL }
};


static Action _GetCommonLinkProperties = {
    "GetCommonLinkProperties", GetCommonLinkProperties,
   (Param []) {
       {"NewWANAccessType", VAR_WANAccessType, VAR_OUT},
       {"NewLayer1UpstreamMaxBitRate", VAR_Layer1UpstreamMaxBitRate, VAR_OUT},
       {"NewLayer1DownstreamMaxBitRate", VAR_Layer1DownstreamMaxBitRate, VAR_OUT},
       {"NewPhysicalLinkStatus", VAR_PhysicalLinkStatus, VAR_OUT},
       { 0 }
    }
};

static Action _GetTotalBytesReceived = { 
    "GetTotalBytesReceived", GetTotalBytesReceived,
        (Param []) {
            {"NewTotalBytesReceived", VAR_TotalBytesReceived, VAR_OUT},
            { 0 }
        }
};


static Action _GetTotalBytesSent = { 
    "GetTotalBytesSent", GetTotalBytesSent,
        (Param []) {
            {"NewTotalBytesSent", VAR_TotalBytesSent, VAR_OUT},
            { 0 }
        }
};

static Action _GetTotalPacketsReceived = { 
    "GetTotalPacketsReceived", GetTotalPacketsReceived,
        (Param []) {
            {"NewTotalPacketsReceived", VAR_TotalPacketsReceived, VAR_OUT},
            { 0 }
        }
};

static Action _GetTotalPacketsSent = { 
    "GetTotalPacketsSent", GetTotalPacketsSent,
        (Param []) {
            {"NewTotalPacketsSent", VAR_TotalPacketsSent, VAR_OUT},
            { 0 }
        }
};




static PAction Actions[] = {
    &_GetCommonLinkProperties,
    &_GetTotalBytesSent,
    &_GetTotalBytesReceived,
    &_GetTotalPacketsReceived,
    &_GetTotalPacketsSent,
    NULL
};

ServiceTemplate Template_WANCommonInterfaceConfig = {
    "WANCommonInterfaceConfig:1",
    WANCommonInterfaceConfig_Init,
    WANCommonInterfaceConfig_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions,
    0,
    "urn:upnp-org:serviceId:WANCommonIFC"
};

static int WANCommonInterfaceConfig_Init(PService psvc, service_state_t state)
{
    PWANDevicePrivateData pdevdata;
    PWANCommonPrivateData pdata;
    struct  itimerspec  timer;

    switch (state) {
    case SERVICE_CREATE:
	pdevdata = (PWANDevicePrivateData) psvc->device->opaque;
	pdata = (PWANCommonPrivateData) malloc(sizeof(WANCommonPrivateData));
	if (pdata) {
	    memset(pdata, 0, sizeof(WANCommonPrivateData));
	    psvc->opaque = pdata;
	    
	    pdata->if_up = osl_link_status(pdevdata->ifname);
	    
	    /* interface speed (bits/sec) */
	    osl_max_bitrates(pdevdata->ifname, &pdata->rx_bitrate, &pdata->tx_bitrate);
	    
	    /* once a second we want to update the statistics variables in the wancommoninterface service */
	    memset(&timer, 0, sizeof(timer));
	    timer.it_interval.tv_sec = 3;
	    timer.it_value.tv_sec = 3;
	    pdata->eventhandle = enqueue_event(&timer, (event_callback_t)WANCommonInterface_UpdateStats, (void *) psvc );
	}
	break;

    case SERVICE_DESTROY:
	pdata = (PWANCommonPrivateData) psvc->opaque;
	
	timer_delete(pdata->eventhandle);
	free(pdata);
	break;
    } /* end switch */
    return TRUE;
}


static void WANCommonInterface_UpdateStats(timer_t t, PService psvc)
{
    PWANDevicePrivateData pdevdata = (PWANDevicePrivateData) psvc->device->opaque;
    PWANCommonPrivateData pdata = (PWANCommonPrivateData) psvc->opaque;
    uint status;

    status = osl_link_status(pdevdata->ifname);
    if (status != pdata->if_up) {
	mark_changed(psvc, VAR_PhysicalLinkStatus);
    }
    pdata->if_up = status;

    if ((psvc->flags & VAR_CHANGED) == VAR_CHANGED) 
	update_all_subscriptions(psvc);
}

static int WANCommonInterfaceConfig_GetVar(struct Service *psvc, int varindex)
{
    PWANDevicePrivateData pdevdata = (PWANDevicePrivateData) psvc->device->opaque;
    PWANCommonPrivateData pdata = (PWANCommonPrivateData) psvc->opaque;
    struct StateVar *var;
    static time_t then;
    time_t now;
    var = &(psvc->vars[varindex]);

    time(&now);
    if (now != then) {
	osl_ifstats(pdevdata->ifname, &pdata->stats);
	then = now;
    }

    switch (varindex) {
    case VAR_TotalBytesSent:
	sprintf(var->value, "%ld", pdata->stats.tx_bytes);
	break;
    case VAR_TotalBytesReceived:
	sprintf(var->value, "%ld", pdata->stats.rx_bytes);
	break;
    case VAR_TotalPacketsReceived:
	sprintf(var->value, "%ld", pdata->stats.rx_packets);
	break;
    case VAR_TotalPacketsSent:
	sprintf(var->value, "%ld", pdata->stats.tx_packets);
	break;
    case VAR_Layer1UpstreamMaxBitRate:
	sprintf(var->value, "%ld", pdata->tx_bitrate);
	break;
    case VAR_Layer1DownstreamMaxBitRate:
	sprintf(var->value, "%ld", pdata->rx_bitrate);
	break;
    case VAR_PhysicalLinkStatus:
	sprintf(var->value, "%s", (pdata->if_up ? "Up" : "Down"));
	break;
    }

    return TRUE;
}

