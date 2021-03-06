/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgeopositioninfosource_s60_p.h"
#include "qgeosatelliteinfosource_s60_p.h"
#include "qmlbackendao_s60_p.h"
#include <QDebug>

QTM_BEGIN_NAMESPACE

//The name of the requestor //Added by PM
//_LIT(KRequestor,"QTMobility Location");

// constructor
CQMLBackendAO::CQMLBackendAO() :
        CActive(EPriorityStandard), // Standard priority
        mPosInfo(NULL),
        mRequester(NULL),
        mRequesterSatellite(NULL),
        mRequestType(RequestType(0))
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::CQMLBackendAO\n" ;
#endif
}

//
//
//
CQMLBackendAO* CQMLBackendAO::NewLC(QObject *aRequester,
                                    RequestType  aRequestType, TPositionModuleId  aModId)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::NewLC\n" ;
#endif
    CQMLBackendAO* self = new(ELeave) CQMLBackendAO();
    CleanupStack::PushL(self);
    self->ConstructL(aRequester, aRequestType, aModId);
    return self;
}

//
//
//
CQMLBackendAO* CQMLBackendAO::NewL(QObject *aRequester,
                                   RequestType  aRequestType, TPositionModuleId  aModId)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::NewL\n" ;
#endif
    CQMLBackendAO* self = CQMLBackendAO::NewLC(aRequester, aRequestType, aModId);
    CleanupStack::Pop(); // self;
    return self;
}

//
//
//
TInt CQMLBackendAO::ConstructL(QObject *aRequester, RequestType  aRequestType,
                               TPositionModuleId  aModId)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::ConstructL\n" ;
#endif

    if (aRequester->inherits("QGeoSatelliteInfoSource"))
        mRequesterSatellite = static_cast<CQGeoSatelliteInfoSourceS60*>(aRequester);
    else
        mRequester = static_cast<CQGeoPositionInfoSourceS60*>(aRequester);

    mRequestType = aRequestType;

    if ((mRequestType == RegularUpdate) || (mRequestType == OnceUpdate)) {

        RPositionServer PosServer;

        if (aRequester->inherits("QGeoSatelliteInfoSource"))
            PosServer = mRequesterSatellite->getPositionServer();
        else
            PosServer = mRequester->getPositionServer();

#if !defined QT_NO_DEBUG
        qDebug() << "CQMLBackendAO::ConstructL, mPositioner.Open";
#endif
        User::LeaveIfError(mPositioner.Open(PosServer, aModId));

#if !defined QT_NO_DEBUG
        qDebug() << "CQMLBackendAO::ConstructL, mPositioner.SetRequestor";
#endif
        User::LeaveIfError(mPositioner.SetRequestor(CRequestor::ERequestorService ,
                CRequestor::EFormatApplication, _L("QTmobility_Location")));

    }

    CActiveScheduler::Add(this); // Add to scheduler

    return ETrue;
}

//
CQMLBackendAO::~CQMLBackendAO()
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::CQMLBackendAO\n" ;
#endif
    Cancel();

    if ((mRequestType == OnceUpdate) || (mRequestType == RegularUpdate)) {
        //close the subsession
        mPositioner.Close();

        if (mPosInfo) {
            mPosInfo->ClearRequestedFields();

            mPosInfo->ClearPositionData();
            //delete the HPositionGenericInfo
            delete mPosInfo;
        }
    } else if (mRequestType == DeviceStatus) {
        RPositionServer posServer;

        //done only by the position server Active Object
        if (mRequester) {
            posServer = mRequester->getPositionServer();
            posServer.Close();
        }
        else if(mRequesterSatellite) {
            posServer = mRequesterSatellite->getPositionServer();
            posServer.Close();
        }
    }
}

//
void CQMLBackendAO::DoCancel()
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::DoCancel\n" ;
#endif
    CancelAll();
}


//
void CQMLBackendAO::RunL()
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::RunL\n" ;
#endif
    switch (mRequestType) {
        case DeviceStatus :
            handleDeviceNotification(iStatus.Int());
            break;
        case RegularUpdate :
        case OnceUpdate:
            handlePosUpdateNotification(iStatus.Int());
            break;
        default         :
            break;
    }
}

//
TInt CQMLBackendAO::RunError(TInt aError)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::RunError\n" ;
#endif
    return aError;
}


// checks any pending request in activeobject
bool CQMLBackendAO::isRequestPending()
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::isRequestPending\n" ;
#endif
    if (IsActive())
        return true;
    else
        return false;
}



// Async call to get notifications about device status.
void CQMLBackendAO::notifyDeviceStatus(TPositionModuleStatusEventBase &aStatusEvent)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::notifyDeviceStatus\n" ;
#endif
    RPositionServer PosServ;

    if (mRequester)
        PosServ = mRequester->getPositionServer();
    else
        PosServ = mRequesterSatellite->getPositionServer();

    //register for device status events
    TPositionModuleStatusEventBase::TModuleEvent RequestedEvents(
        TPositionModuleStatusEventBase::EEventAll);

    aStatusEvent.SetRequestedEvents(RequestedEvents);

    PosServ.NotifyModuleStatusEvent(aStatusEvent, iStatus);

    SetActive();

}

void CQMLBackendAO::CancelAll()
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::CancelAll\n" ;
#endif

    RPositionServer PosServer;
    if (mRequestType == DeviceStatus) {
        if (mRequester)
            PosServer = mRequester->getPositionServer();
        else
            PosServer = mRequesterSatellite->getPositionServer();

        PosServer.CancelRequest(EPositionServerNotifyModuleStatusEvent);
    } else if ((mRequestType == OnceUpdate) || (mRequestType == RegularUpdate)) {
        mPositioner.CancelRequest(EPositionerNotifyPositionUpdate);
    }

}

//Initialize the posInfo with appropriate fields
bool CQMLBackendAO::initializePosInfo()
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::initializePosInfo\n" ;
#endif
    if (!mPosInfo) {
        mPosInfo = HPositionGenericInfo::New();

        if (mPosInfo == NULL)
            return FALSE;
    } else {
        mPosInfo->ClearRequestedFields();

        mPosInfo->ClearPositionData();
    }
    RPositionServer posServer;
    TPositionModuleInfo moduleInfo;
    TInt error = KErrNone;

    //get the posiiton server
    posServer = mRequester->getPositionServer();

    //retrieve the module id used by the posiitoner
    if (mRequestType == RegularUpdate)
        error = posServer.GetModuleInfoById(mRequester->getCurrentPositonModuleID(), moduleInfo);
    else
        error = posServer.GetModuleInfoById(mRequester->getRequestUpdateModuleID(), moduleInfo);

    if (error == KErrNone) {

        //get capabilities of the module
        TPositionModuleInfo::TCapabilities caps = moduleInfo.Capabilities();

        if (caps & TPositionModuleInfo::ECapabilitySatellite) {
            mPosInfo->SetRequestedField(EPositionFieldSatelliteNumInView);
            mPosInfo->SetRequestedField(EPositionFieldSatelliteNumUsed);
        }

        if (caps & TPositionModuleInfo::ECapabilitySpeed) {
            mPosInfo->SetRequestedField(EPositionFieldHorizontalSpeed);
            mPosInfo->SetRequestedField(EPositionFieldVerticalSpeed);
        }
        if (caps & TPositionModuleInfo::ECapabilityCompass) {
            mPosInfo->SetRequestedField(EPositionFieldMagneticCourseError);
            mPosInfo->SetRequestedField(EPositionFieldHeading);
        }
        return TRUE;
    }
    return FALSE;
}

//requestUpdate : request for position update once
void CQMLBackendAO::requestUpdate(int aTimeout)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::requestUpdate\n" ;
#endif
    TPositionUpdateOptions  aPosOption;

    mPositioner.GetUpdateOptions(aPosOption);

    aPosOption.SetUpdateInterval(TTimeIntervalMicroSeconds(0));

    aPosOption.SetUpdateTimeOut(TTimeIntervalMicroSeconds(aTimeout * 1000));

    mPositioner.SetUpdateOptions(aPosOption);

    startUpdates();
}


//
void CQMLBackendAO::cancelUpdate()
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::cancelUpdate\n" ;
#endif
    Cancel();

}


//
void CQMLBackendAO::handleDeviceNotification(int aError)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::handleDeviceNotification\n" ;
#endif
    switch (aError) {
            //NotifyPositionModulestatusEvent successfully completed
        case KErrNone :

            //module not found
        case KErrNotFound :
        if (mRequester) {
            mRequester->updateDeviceStatus();
        }
        else {
            mRequesterSatellite->updateDeviceStatus();
        }
        break;

            //request has been successfully cancelled
        case KErrCancel :
            break;

            //unrecoverabe errors
        default :
            break;
    }
}


//
void CQMLBackendAO::handlePosUpdateNotification(int aError)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::handlePosUpdateNotification\n" ;
#endif
    HPositionGenericInfo *positionInfo = NULL;

    TPositionSatelliteInfo satInfo;
    switch (aError) {
            //NotifyPositionUpdate successfully completed
        case KErrNone :
            //requested information could not be retrieved within the maximum peroid
        case KErrTimedOut:
            if (mRequester) {
                positionInfo = HPositionGenericInfo::New();

                if (positionInfo == NULL)
                    return;

                //copy the buffer contents into a new HPositionGenericInfo buffer,to be used
                //for creating QGeoPositionInfo object later
                memcpy(positionInfo , mPosInfo , mPosInfo->BufferSize());
            } else {
                satInfo  = mPosSatInfo;
            }

            //if regUpdateAO, request for the next update
            if (mRequestType == RegularUpdate) {
                if (mRequester) {
                    initializePosInfo();
                    mPositioner.NotifyPositionUpdate(*mPosInfo, iStatus);
                } else {
					mPosSatInfo.ClearSatellitesInView();
                    mPositioner.NotifyPositionUpdate(mPosSatInfo, iStatus);
					}

                SetActive();
            }

        if (mRequester) {
            mRequester->updatePosition(positionInfo, aError);
            delete positionInfo;
        }
        else {
            if ((aError != KErrTimedOut) || (mRequestType != RegularUpdate)) {
                mRequesterSatellite->updatePosition(satInfo, aError, (mRequestType == RegularUpdate));
            }
        }

            break;

        default :
        if (mRequester) {
            mRequester->updatePosition(positionInfo, aError); // positionInfo will be NULL
        }
        else {
            mRequesterSatellite->updatePosition(satInfo, aError, (mRequestType == RegularUpdate));
        }
        break;

    }
}

//////////////////////////////////////////////////////////////
// Sets the interval for getting the regular notification   //
// the time interval set is in milli seconds                      //
//////////////////////////////////////////////////////////////
int CQMLBackendAO::setUpdateInterval(int aMilliSec)
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::setUpdateInterval\n" ;
#endif
    int minimumUpdateInterval = 0;
    TInt64 mUpdateInterval = 0 ;


    if (mRequester)
        minimumUpdateInterval = mRequester->minimumUpdateInterval();
    else
        minimumUpdateInterval = mRequesterSatellite->minimumUpdateInterval();

    if (minimumUpdateInterval < 0)
        minimumUpdateInterval = 100;
    // if the current requesttype is  regular updates
    // then set the updateinterval otherwise ignore
    //if(mRequestType != REQ_REG_UPDATE)
    //    return;

    TPositionUpdateOptions  aPosOption;

    TInt error = mPositioner.GetUpdateOptions(aPosOption);

    // TTimeIntervalMicroSeconds is converted seconds
    TInt currentUpdateInterval  = aPosOption.UpdateInterval().Int64() / 1000;

    // If msec is not 0 and is less than the value returned by minimumUpdateInterval(),
    // the interval will be set to the minimum interval.
    // if (aMilliSec != 0 && aMilliSec <= minimumUpdateInterval) {
    // workaround, not accepting zero as value, see QTMOBILITY-995
    if (aMilliSec <= minimumUpdateInterval) {
        mUpdateInterval = minimumUpdateInterval;
    } else {
        mUpdateInterval = aMilliSec;
    }

    // if the same value is being set then just ignore it.
    if (currentUpdateInterval == mUpdateInterval) {
        return mUpdateInterval;
    }

    // will set Either zero, minimum or +ve value
    // seconds converted to TTimeIntervalMicroSeconds
    aPosOption.SetUpdateInterval(TTimeIntervalMicroSeconds(mUpdateInterval * 1000));
    // set the timeout to the smaller of 150% of interval or update interval + 10 seconds
    TInt64 mUpdateTimeout = (mUpdateInterval * 3) / 2;
    if (mUpdateTimeout > mUpdateInterval + 10000)
        mUpdateTimeout = mUpdateInterval + 10000;

    if (aMilliSec > 0)
        aPosOption.SetUpdateTimeOut(TTimeIntervalMicroSeconds(mUpdateTimeout * 1000));

    error = mPositioner.SetUpdateOptions(aPosOption);

    return mUpdateInterval;
}

void CQMLBackendAO::startUpdates()
{
#if !defined QT_NO_DEBUG
	qDebug() << "CQMLBackendAO::startUpdates\n" ;
#endif
    if (!IsActive()) {
        if (mRequester) {
            initializePosInfo();
            mPositioner.NotifyPositionUpdate(*mPosInfo , iStatus);
        }
        else {
            mPosSatInfo.ClearSatellitesInView();
            mPositioner.NotifyPositionUpdate(mPosSatInfo , iStatus);
		}
		
        SetActive();

    }
}

QTM_END_NAMESPACE
