/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Internal Headers
#include "magnetometersensorsym.h"

/**
 * set the id of the magnetometer sensor
 */
const char *CMagnetometerSensorSym::id("sym.magnetometer");

/**
 * Factory function, this is used to create the magnetometer sensor object
 * @return CMagnetometerSensorSym if successful, leaves on failure
 */
CMagnetometerSensorSym* CMagnetometerSensorSym::NewL(QSensor *sensor)
    {
    CMagnetometerSensorSym* self = new (ELeave) CMagnetometerSensorSym(sensor);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;    
    }

/**
 * Destructor
 * Closes the backend resources
 */
CMagnetometerSensorSym::~CMagnetometerSensorSym()
    {
    //Closes the backend resources
    Close();
    }

/**
 * Default constructor
 */
CMagnetometerSensorSym::CMagnetometerSensorSym(QSensor *sensor):CSensorBackendSym(sensor),
        iCalibrationLevel(0.0)
    {
    if(sensor)
        {
        setReading<QMagnetometerReading>(&iReading);
        }
    iBackendData.iSensorType = KSensrvChannelTypeIdMagnetometerXYZAxisData;    
    //Enable Property listening, required to get Calibration level
    SetListening(ETrue, ETrue);
    }

/**
 * start is overridden to allow retrieving initial calibration property before
 * and to set the required value type flags
 */
void CMagnetometerSensorSym::start()
    {
    if(sensor())
        {
        // Initialize the values
        iReading.setX(0);
        iReading.setY(0);
        iReading.setZ(0);
        // Set the required type of values
        QVariant v = sensor()->property("returnGeoValues");
        iReturnGeoValues = (v.isValid() && v.toBool()); // if the property isn't set it's false
        }
    // get current property value for calibration and set it to reading
    TSensrvProperty calibration;
    TRAPD(err, iBackendData.iSensorChannel->GetPropertyL(KSensrvPropCalibrationLevel,ESensrvSingleProperty, calibration));
    // If error in getting the calibration level, continue to start the sensor
    // as it is not a fatal error
    if ( err == KErrNone )
        {
        TInt calibrationVal;
        calibration.GetValue(calibrationVal);
        iCalibrationLevel = calibrationVal * (1.0/3.0);
        }
    // Call backend start
    CSensorBackendSym::start();
    }

/*
 * RecvData is used to retrieve the sensor reading from sensor server
 * It is implemented here to handle magnetometer sensor specific
 * reading data and provides conversion and utility code
 */ 
void CMagnetometerSensorSym::RecvData(CSensrvChannel &aChannel)
    {
    TPckg<TSensrvMagnetometerAxisData> magnetometerpkg( iData );
    TInt ret = aChannel.GetData( magnetometerpkg );
    if(KErrNone != ret)
        {
        // If there is no reading available, return without setting
        return;
        }
    // Get a lock on the reading data
    iBackendData.iReadingLock.Wait();
    // If Geo values are requested set it
    if(iReturnGeoValues)
        {
        iReading.setX(iData.iAxisXCalibrated);
        iReading.setY(iData.iAxisYCalibrated);
        iReading.setZ(iData.iAxisZCalibrated);
        }
    // If Raw values are requested set it
    else
        {
        iReading.setX(iData.iAxisXRaw);
        iReading.setY(iData.iAxisYRaw);
        iReading.setZ(iData.iAxisZRaw);
        }
    // Set the timestamp
    iReading.setTimestamp(iData.iTimeStamp.Int64());
    // Set the calibration level
    iReading.setCalibrationLevel(iCalibrationLevel);
    // Release the lock
    iBackendData.iReadingLock.Signal();
    }

/**
 * HandlePropertyChange is called from backend, to indicate a change in property
 */
void CMagnetometerSensorSym::HandlePropertyChange(CSensrvChannel &/*aChannel*/, const TSensrvProperty &aChangedProperty)
    {
    if(aChangedProperty.GetPropertyId() != KSensrvPropCalibrationLevel)
        {
        // Do nothing, if calibration property has not changed
        return;
        }
    TInt calibrationlevel;
    aChangedProperty.GetValue(calibrationlevel);
    // As Qt requires calibration level in qreal but symbian provides in enum
    // It has been agreed with DS Team that the following mechanism will be
    // used till discussions with qt mobility are complete
    iCalibrationLevel = (1.0/3.0) * calibrationlevel;
    }

/*
 * Used to retrieve the current calibration level
 * iCalibrationLevel is automatically updated whenever there is a change
 * in calibration level
 */
qreal CMagnetometerSensorSym::GetCalibrationLevel()
    {
    return iCalibrationLevel;
    }

/**
 * Second phase constructor
 * Initialize the backend resources
 */
void CMagnetometerSensorSym::ConstructL()
    {
    InitializeL();
    }
