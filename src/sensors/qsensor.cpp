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

#include "qsensor.h"
#include "qsensor_p.h"
#include "qsensorbackend.h"
#include "qsensormanager.h"
#include <QDebug>
#include <QMetaProperty>
#include <QTimer>

QTM_BEGIN_NAMESPACE

/*!
    \typedef qtimestamp
    \relates QSensor
    \inmodule QtSensors

    Sensor timestamps are represented by this typedef which is a 64 bit unsigned integer.

    Timestamps values are microseconds since a fixed point.
    You can use timestamps to see how far apart two sensor readings are.

    Note that sensor timestamps from different sensors may not be directly
    comparable (as they may choose different fixed points for their reference).

    \bold{Note that some platforms do not deliver timestamps correctly}.
    Applications should be prepared for occasional issues that cause timestamps to jump
    forwards or backwards. The \l{sensors-api.html#platform-notes}{platform notes} have
    more details.
*/

/*!
    \typedef qrange
    \relates QSensor
    \inmodule QtSensors

    This type is defined as a QPair.

    \code
    typedef QPair<int,int> qrange;
    \endcode

    \sa QPair, qrangelist, QSensor::availableDataRates
*/

/*!
    \typedef qrangelist
    \relates QSensor
    \inmodule QtSensors

    This type is defined as a list of qrange values.

    \code
    typedef QList<qrange> qrangelist;
    \endcode

    \sa QList, qrange, QSensor::availableDataRates
*/

/*!
    \class qoutputrange
    \inmodule QtSensors
    \brief The qoutputrange class holds the specifics of an output range.

    The class is defined as a simple struct.

    \code
    struct qoutputrange
    {
        qreal maximum;
        qreal minimum;
        qreal accuracy;
    };
    \endcode

    Each output range specifies a minimum and maximum value as well as an accuracy value.
    The accuracy value represents the resolution of the sensor. It is the smallest change
    the sensor can detect and is expressed using the same units as the minimum and maximum.

    Sensors must often trade off range for accuracy. To allow the user to determine which of
    these are more important the sensor may offer several output ranges. One output
    range may have reduced minimum and maximum values and increased sensitivity. Another output
    range may have higher minimum and maximum values with reduced sensitivity. Note that higher
    sensitivities will be represented by smaller accuracy values.

    An example of this tradeoff can be seen by examining the LIS302DL accelerometer. It has only
    256 possible values to report with. These values are scaled so that they can represent either
    -2G to +2G (with an accuracy value of 0.015G) or -8G to +8G (with an accuracy value of 0.06G).

    \sa qoutputrangelist, QSensor::outputRanges
    \since 1.0
*/

/*!
    \variable qoutputrange::maximum

    This is the maximum value for this output range.
    The units are defined by the sensor.
*/

/*!
    \variable qoutputrange::minimum

    This is the minimum value for this output range.
    The units are defined by the sensor.
*/

/*!
    \variable qoutputrange::accuracy

    The accuracy value represents the resolution of the sensor. It is the smallest change
    the sensor can detect and is expressed using the same units as the minimum and maximum.
*/

/*!
    \typedef qoutputrangelist
    \relates QSensor
    \inmodule QtSensors

    This type is defined as a list of qoutputrange values.

    \code
    typedef QList<qoutputrange> qoutputrangelist;
    \endcode

    \sa QList, qoutputrange, QSensor::outputRanges
*/

// A bit of a hack to call qRegisterMetaType when the library is loaded.
static int qtimestamp_id = qRegisterMetaType<QtMobility::qtimestamp>("QtMobility::qtimestamp");
static int qrange_id = qRegisterMetaType<QtMobility::qrange>("QtMobility::qrange");
static int qrangelist_id = qRegisterMetaType<QtMobility::qrangelist>("QtMobility::qrangelist");
static int qoutputrangelist_id = qRegisterMetaType<QtMobility::qoutputrangelist>("QtMobility::qoutputrangelist");

// =====================================================================

/*!
    \class QSensor
    \ingroup sensors_main
    \inmodule QtSensors

    \brief The QSensor class represents a single hardware sensor.

    The life cycle of a sensor is typically:

    \list
    \o Create a sub-class of QSensor on the stack or heap.
    \o Setup as required by the application.
    \o Start receiving values.
    \o Sensor data is used by the application.
    \o Stop receiving values.
    \endlist

    The sensor data is delivered via QSensorData and its sub-classes.

    \sa QSensorReading
    \since 1.0
*/

void QSensorPrivate::init(const QByteArray &sensorType)
{
    type = sensorType;
    q->registerInstance(); // so the availableSensorsChanged() signal works
}

/*!
    Construct the \a type sensor as a child of \a parent.

    Do not use this constructor if a derived class exists for the specific sensor type.

    The wrong way is to use the base class constructor:
    \snippet snippets/sensors/creating.cpp 3
    The right way is to create an instance of the derived class:
    \snippet snippets/sensors/creating.cpp 2

    The derived classes have
    additional properties and data members which are needed for certain features such as
    geo value support in QMagnetometer or acceleration mode support in QAccelerometer.
    These features will only work properly when creating a sensor instance from a QSensor
    subclass.

    Only use this constructor if there is no derived sensor class available. Note that all
    built-in sensors have a derived class, so using this constructor should only be necessary
    when implementing custom sensors, like in the \l {Grue Application}{Grue sensor
    example}.

    \since 1.0
*/
QSensor::QSensor(const QByteArray &type, QObject *parent)
    : QObject(parent)
    , d(new QSensorPrivate(this))
{
    d->init(type);
}

/*!
   \internal
 */
QSensor::QSensor(const QByteArray &type, QSensorPrivate *dd, QObject *parent)
    : QObject(parent), d(dd)
{
    d->init(type);
}

/*!
    Destroy the sensor. Stops the sensor if it has not already been stopped.
    \since 1.0
*/
QSensor::~QSensor()
{
    stop();
    Q_FOREACH (QSensorFilter *filter, d->filters)
        filter->setSensor(0);
    delete d->backend;
    d->backend = 0;
    // owned by the backend
    d->device_reading = 0;
    d->filter_reading = 0;
    d->cache_reading = 0;
}

/*!
    \property QSensor::connectedToBackend
    \brief a value indicating if the sensor has connected to a backend.

    A sensor that has not been connected to a backend cannot do anything useful.

    Call the connectToBackend() method to force the sensor to connect to a backend
    immediately. This is automatically called if you call start() so you only need
    to do this if you need access to sensor properties (ie. to poll the sensor's
    meta-data before you use it).
    \since 1.0
*/

bool QSensor::isConnectedToBackend() const
{
    return (d->backend != 0);
}

/*!
    \property QSensor::sensorid
    \brief the backend identifier for the sensor.

    Note that the identifier is filled out automatically
    when the sensor is connected to a backend. If you want
    to connect a specific backend, you should call
    setIdentifier() before connectToBackend().
    \since 1.0
*/

QByteArray QSensor::identifier() const
{
    return d->identifier;
}

void QSensor::setIdentifier(const QByteArray &identifier)
{
    if (isConnectedToBackend()) {
        qWarning() << "ERROR: Cannot call QSensor::setIdentifier while connected to a backend!";
        return;
    }
    d->identifier = identifier;
}

/*!
    \property QSensor::type
    \brief the type of the sensor.
    \since 1.0
*/

QByteArray QSensor::type() const
{
    return d->type;
}

/*!
    Try to connect to a sensor backend.

    Returns true if a suitable backend could be found, false otherwise.

    The type must be set before calling this method if you are using QSensor directly.

    \sa isConnectedToBackend()
    \since 1.0
*/
bool QSensor::connectToBackend()
{
    if (isConnectedToBackend())
        return true;

    d->backend = QSensorManager::createBackend(this);

    // Reset the properties to their default values and re-set them now so
    // that the logic we've put into the setters gets called.
    if (d->dataRate != 0) {
        int tmp = d->dataRate;
        d->dataRate = 0;
        setDataRate(tmp);
    }
    if (d->outputRange != -1) {
        int tmp = d->outputRange;
        d->outputRange = -1;
        setOutputRange(tmp);
    }

    return isConnectedToBackend();
}

/*!
    \property QSensor::busy
    \brief a value to indicate if the sensor is busy.

    Some sensors may be on the system but unavailable for use.
    This function will return true if the sensor is busy. You
    will not be able to start() the sensor.

    Note that this function does not return true if you
    are using the sensor, only if another process is using
    the sensor.

    \sa busyChanged()
    \since 1.0
*/

bool QSensor::isBusy() const
{
    return d->busy;
}

/*!
    \fn QSensor::busyChanged()

    This signal is emitted when the sensor is no longer busy.
    This can be used to grab a sensor when it becomes available.

    \code
    sensor.start();
    if (sensor.isBusy()) {
        // need to wait for busyChanged signal and try again
    }
    \endcode
    \since 1.0
*/

/*!
    \property QSensor::active
    \brief a value to indicate if the sensor is active.

    This is true if the sensor is active (returning values). This is false otherwise.

    Note that setting this value to true will not have an immediate effect. Instead,
    the sensor will be started once the event loop has been reached.
    \since 1.1
*/
void QSensor::setActive(bool active)
{
    if (active == isActive())
        return;

    if (active)
        QTimer::singleShot(0, this, SLOT(start())); // delay ensures all properties have been set if using QML
    else
        stop();
}

bool QSensor::isActive() const
{
    return d->active;
}

/*!
    \property QSensor::alwaysOn
    \brief a value to indicate if the sensor should remain running when the screen is off.

    Some platforms have a policy of suspending sensors when the screen turns off.
    Setting this property to true will ensure the sensor continues to run.
*/
/*!
    \fn QSensor::alwaysOnChanged()

    This signal is emitted when the alwaysOn property changes.
*/
void QSensor::setAlwaysOn(bool alwaysOn)
{
    if (d->alwaysOn == alwaysOn) return;
    d->alwaysOn = alwaysOn;
    emit alwaysOnChanged();
}

bool QSensor::isAlwaysOn() const
{
    return d->alwaysOn;
}

/*!
    \property QSensor::skipDuplicates
    \brief Indicates whether duplicate reading values should be omitted.

    When duplicate skipping is enabled, successive readings with the same or very
    similar values are omitted and skipped. This helps reducing the amount of processing
    done, as less sensor readings are made available. As a consequence, readings arrive at
    an irregular interval.

    Duplicate skipping is not just enabled for readings that are exactly the same, but also for
    readings that are quite similar, as each sensor has a bit of jitter even if the device is
    not moved.

    Support for this property depends on the backend. It is disabled by default.

    Duplicate skipping can only be changed while the sensor is not active.
*/
bool QSensor::skipDuplicates() const
{
    Q_D(const QSensor);
    return d->skipDuplicates;
}

/*!
    Sets the duplicate skipping to \a skipDuplicates.
*/
void QSensor::setSkipDuplicates(bool skipDuplicates)
{
    Q_D(QSensor);
    if (d->skipDuplicates != skipDuplicates) {
        d->skipDuplicates = skipDuplicates;
        emit skipDuplicatesChanged(skipDuplicates);
    }
}

/*!
    \fn skipDuplicatesChanged(bool skipDuplicates)

    This signal is emitted when the skipDuplicates property changes.

*/

/*!
    \property QSensor::availableDataRates
    \brief the data rates that the sensor supports.

    This is a list of the data rates that the sensor supports.
    Measured in Hertz.

    Entries in the list can represent discrete rates or a
    continuous range of rates.
    A discrete rate is noted by having both values the same.

    See the sensor_explorer example for an example of how to interpret and use
    this information.

    Note that this information is not mandatory as not all sensors have a rate at which
    they run. In such cases, the list will be empty.

    \sa QSensor::dataRate, qrangelist
    \since 1.0
*/

qrangelist QSensor::availableDataRates() const
{
    return d->availableDataRates;
}

/*!
    \property QSensor::dataRate
    \brief the data rate that the sensor should be run at.

    Measured in Hertz.

    The data rate is the maximum frequency at which the sensor can detect changes.

    Setting this property is not portable and can cause conflicts with other
    applications. Check with the sensor backend and platform documentation for
    any policy regarding multiple applications requesting a data rate.

    The default value (0) means that the app does not care what the data rate is.
    Applications should consider using a timer-based poll of the current value or
    ensure that the code that processes values can run very quickly as the platform
    may provide updates hundreds of times each second.

    This should be set before calling start() because the sensor may not
    notice changes to this value while it is running.

    Note that there is no mechanism to determine the current data rate in use by the
    platform.

    \sa QSensor::availableDataRates
    \since 1.0
*/

int QSensor::dataRate() const
{
    return d->dataRate;
}

void QSensor::setDataRate(int rate)
{
    if (d->dataRate != rate) {
        d->dataRate = rate;
        emit dataRateChanged();
    }
}

/*!
    Start retrieving values from the sensor.
    Returns true if the sensor was started, false otherwise.

    The sensor may fail to start for several reasons.

    Once an application has started a sensor it must wait until the sensor receives a
    new value before it can query the sensor's values. This is due to how the sensor
    receives values from the system. Sensors do not (in general) poll for new values,
    rather new values are pushed to the sensors as they happen.

    For example, this code will not work as intended.

    \badcode
    sensor->start();
    sensor->reading()->x(); // no data available
    \endcode

    To work correctly, the code that accesses the reading should ensure the
    readingChanged() signal has been emitted.

    \code
        connect(sensor, SIGNAL(readingChanged()), this, SLOT(checkReading()));
        sensor->start();
    }
    void MyClass::checkReading() {
        sensor->reading()->x();
    \endcode

    \sa QSensor::busy
    \since 1.0
*/
bool QSensor::start()
{
    if (isActive())
        return true;
    if (!connectToBackend())
        return false;
    // Set these flags to their defaults
    d->active = true;
    d->busy = false;
    // Backend will update the flags appropriately
    d->backend->start();
    Q_EMIT activeChanged();
    return isActive();
}

/*!
    Stop retrieving values from the sensor.

    This releases the sensor so that other processes can use it.

    \sa QSensor::busy
    \since 1.0
*/
void QSensor::stop()
{
    if (!isConnectedToBackend() || !isActive())
        return;
    d->active = false;
    d->backend->stop();
    Q_EMIT activeChanged();
}

/*!
    \property QSensor::reading
    \brief the reading class.

    The reading class provides access to sensor readings. The reading object
    is a volatile cache of the most recent sensor reading that has been received
    so the application should process readings immediately or save the values
    somewhere for later processing.

    Note that this will return 0 until a sensor backend is connected to a backend.

    Also note that readings are not immediately available after start() is called.
    Applications must wait for the readingChanged() signal to be emitted.

    \sa isConnectedToBackend(), start()
    \since 1.0
*/

QSensorReading *QSensor::reading() const
{
    return d->cache_reading;
}

/*!
    Add a \a filter to the sensor.

    The sensor does not take ownership of the filter.
    QSensorFilter will inform the sensor if it is destroyed.

    \sa QSensorFilter
    \since 1.0
*/
void QSensor::addFilter(QSensorFilter *filter)
{
    if (!filter) {
        qWarning() << "addFilter: passed a null filter!";
        return;
    }
    filter->setSensor(this);
    d->filters << filter;
}

/*!
    Remove \a filter from the sensor.

    \sa QSensorFilter
    \since 1.0
*/
void QSensor::removeFilter(QSensorFilter *filter)
{
    if (!filter) {
        qWarning() << "removeFilter: passed a null filter!";
        return;
    }
    d->filters.removeOne(filter);
    filter->setSensor(0);
}

/*!
    Returns the filters currently attached to the sensor.

    \sa QSensorFilter
    \since 1.2
*/
QList<QSensorFilter*> QSensor::filters() const
{
    return d->filters;
}

/*!
    \fn QSensor::d_func() const
    \internal
    \since 1.0
*/

/*!
    \fn QSensor::readingChanged()

    This signal is emitted when a new sensor reading is received.

    The sensor reading can be found in the QSensor::reading property. Note that the
    reading object is a volatile cache of the most recent sensor reading that has
    been received so the application should process the reading immediately or
    save the values somewhere for later processing.

    Before this signal has been emitted for the first time, the reading object will
    have uninitialized data.

    \sa start()
    \since 1.0
*/

/*!
    \fn QSensor::activeChanged()

    This signal is emitted when the QSensor::active property has changed.

    \sa QSensor::active
    \since 1.1
*/

/*!
    \property QSensor::outputRanges
    \brief a list of output ranges the sensor supports.

    A sensor may have more than one output range. Typically this is done
    to give a greater measurement range at the cost of lowering accuracy.

    Note that this information is not mandatory. This information is typically only
    available for sensors that have selectable output ranges (such as typical
    accelerometers).

    \sa QSensor::outputRange, qoutputrangelist
    \since 1.0
*/

qoutputrangelist QSensor::outputRanges() const
{
    return d->outputRanges;
}

/*!
    \property QSensor::outputRange
    \brief the output range in use by the sensor.

    This value represents the index in the QSensor::outputRanges list to use.

    Setting this property is not portable and can cause conflicts with other
    applications. Check with the sensor backend and platform documentation for
    any policy regarding multiple applications requesting an output range.

    The default value (-1) means that the app does not care what the output range is.

    Note that there is no mechanism to determine the current output range in use by the
    platform.

    \sa QSensor::outputRanges
    \since 1.0
*/

int QSensor::outputRange() const
{
    return d->outputRange;
}

void QSensor::setOutputRange(int index)
{
    if (index == -1 || !isConnectedToBackend()) {
        d->outputRange = index;
        return;
    }
    bool warn = true;
    if (index >= 0 && index < d->outputRanges.count()) {
        warn = false;
        d->outputRange = index;
    }
    if (warn) {
        qWarning() << "setOutputRange:" << index << "is not supported by the sensor.";
    }
}

/*!
    \property QSensor::description
    \brief a descriptive string for the sensor.
    \since 1.0
*/

QString QSensor::description() const
{
    return d->description;
}

/*!
    \property QSensor::error
    \brief the last error code set on the sensor.

    Note that error codes are sensor-specific.
    \since 1.0
*/

int QSensor::error() const
{
    return d->error;
}

/*!
    \fn QSensor::sensorError(int error)

    This signal is emitted when an \a error code is set on the sensor.
    Note that some errors will cause the sensor to stop working.
    You should call isActive() to determine if the sensor is still running.
    \since 1.0
*/

/*!
    \fn QSensor::availableSensorsChanged()

    This signal is emitted when the list of available sensors has changed.
    The sensors available to a program will not generally change over time
    however some of the avilable sensors may represent hardware that is not
    permanently connected. For example, a game controller that is connected
    via bluetooth would become available when it was on and would become
    unavailable when it was off.

    \sa QSensor::sensorTypes(), QSensor::sensorsForType()
    \since 1.2
*/

/*!
    \property QSensor::maxBufferSize

    The property holds the maximum buffer size.

    Note that this may be 1, in which case the sensor does not support any form of buffering.

    \sa QSensor::bufferSize, QSensor::efficientBufferSize
*/

int QSensor::maxBufferSize() const
{
    return d->maxBufferSize;
}

/*!
    Sets the maximum buffer size to \a maxBufferSize. This is to be called from the
    backend.
*/
void QSensor::setMaxBufferSize(int maxBufferSize)
{
    if (d->maxBufferSize != maxBufferSize) {
        d->maxBufferSize = maxBufferSize;
        emit maxBufferSizeChanged(maxBufferSize);
    }
}

/*!
    \property QSensor::efficientBufferSize

    The property holds the most efficient buffer size. Normally this is 1 (which means
    no particular size is most efficient). Some sensor drivers have a FIFO buffer which
    makes it more efficient to deliver the FIFO's size worth of readings at one time.

    \sa QSensor::bufferSize, QSensor::maxBufferSize
*/

int QSensor::efficientBufferSize() const
{
    return d->efficientBufferSize;
}

/*!
    Sets the efficient buffer size to \a efficientBufferSize. This is to be called from the
    backend.
*/
void QSensor::setEfficientBufferSize(int efficientBufferSize)
{
    if (d->efficientBufferSize != efficientBufferSize) {
        d->efficientBufferSize = efficientBufferSize;
        emit efficientBufferSizeChanged(efficientBufferSize);
    }
}

/*!
    \property QSensor::bufferSize

    This property holds the size of the buffer. By default, the buffer size is 1,
    which means no buffering.
    If the maximum buffer size is 1, then buffering is not supported
    by the sensor.

    Setting bufferSize greater than maxBufferSize will cause maxBufferSize to be used.

    Buffering is turned on when bufferSize is greater than 1. The sensor will collect
    the requested number of samples and deliver them all to the application at one time.
    They will be delivered to the application as a burst of changed readings so it is
    particularly important that the application processes each reading immediately or
    saves the values somewhere else.

    If stop() is called when buffering is on-going, the partial buffer is not delivered.

    When the sensor is started with buffering option, values are collected from that
    moment onwards. There is no pre-existing buffer that can be utilized.

    Some backends like Blackberry only support enabling or disabling the buffer and do not give
    control over the size. In this case, the maxBufferSize and efficientBufferSize properties
    might not be set at all, even though buffering is supported. Setting the bufferSize property
    to any value greater than 1 will enable buffering. After the sensor has been started,
    the bufferSize property will be set to the actual value by the backend.

    On Blackberry, buffering will not wait until the buffer is full to deliver new
    readings. Instead, the buffer will be used if the backend does not manage to retrieve the readings
    in time, for example when the event loop is blocked for too long. Without a buffer, these readings
    would simply be dropped.

    \sa QSensor::maxBufferSize, QSensor::efficientBufferSize
*/

int QSensor::bufferSize() const
{
    return d->bufferSize;
}

void QSensor::setBufferSize(int bufferSize)
{
    if (d->bufferSize != bufferSize) {
        d->bufferSize = bufferSize;
        emit bufferSizeChanged(bufferSize);
    }
}

// =====================================================================

/*!
    \class QSensorFilter
    \ingroup sensors_main
    \inmodule QtSensors

    \brief The QSensorFilter class provides an efficient
           callback facility for asynchronous notifications of
           sensor changes.

    Some sensors (eg. the accelerometer) are often accessed very frequently.
    This may be slowed down by the use of signals and slots.
    The QSensorFilter interface provides a more efficient way for the
    sensor to notify your class that the sensor has changed.

    Additionally, multiple filters can be added to a sensor. They are called
    in order and each filter has the option to modify the values in the reading
    or to suppress the reading altogether.

    Note that the values in the class returned by QSensor::reading() will
    not be updated until after the filters have been run.

    \sa filter()
    \since 1.0
*/

/*!
    \internal
    \since 1.0
*/
QSensorFilter::QSensorFilter()
    : m_sensor(0)
{
}

/*!
    Notifies the attached sensor (if any) that the filter is being destroyed.
    \since 1.0
*/
QSensorFilter::~QSensorFilter()
{
    if (m_sensor)
        m_sensor->removeFilter(this);
}

/*!
    \fn QSensorFilter::filter(QSensorReading *reading)

    This function is called when the sensor \a reading changes.

    The filter can modify the reading.

    Returns true to allow the next filter to receive the value.
    If this is the last filter, returning true causes the signal
    to be emitted and the value is stored in the sensor.

    Returns false to drop the reading.
    \since 1.0
*/

/*!
    \internal
    \since 1.0
*/
void QSensorFilter::setSensor(QSensor *sensor)
{
    m_sensor = sensor;
}

// =====================================================================

/*!
    \class QSensorReading
    \ingroup sensors_main
    \inmodule QtSensors

    \brief The QSensorReading class holds the readings from the sensor.

    Note that QSensorReading is not particularly useful by itself. The interesting
    data for each sensor is defined in a sub-class of QSensorReading.
    \since 1.0
*/

/*!
    \internal
    \since 1.0
*/
QSensorReading::QSensorReading(QObject *parent, QSensorReadingPrivate *_d)
    : QObject(parent)
    , d(_d?_d:new QSensorReadingPrivate)
{
}

/*!
    \internal
    \since 1.0
*/
QSensorReading::~QSensorReading()
{
}

/*!
    \property QSensorReading::timestamp
    \brief the timestamp of the reading.

    \sa qtimestamp
    \since 1.0
*/

/*!
    Returns the timestamp of the reading.
    \since 1.0
*/
qtimestamp QSensorReading::timestamp() const
{
    return d->timestamp;
}

/*!
    Sets the \a timestamp of the reading.
    \since 1.0
*/
void QSensorReading::setTimestamp(qtimestamp timestamp)
{
    d->timestamp = timestamp;
}

/*!
    Returns the number of extra properties that the reading has.

    Note that this does not count properties declared in QSensorReading.

    As an example, this returns 3 for QAccelerometerReading because
    there are 3 properties defined in that class.
    \since 1.0
*/
int QSensorReading::valueCount() const
{
    const QMetaObject *mo = metaObject();
    return mo->propertyCount() - mo->propertyOffset();
}

/*!
    Returns the value of the property at \a index.

    Note that this function is slower than calling the data function directly.

    Here is an example of getting a property via the different mechanisms available.

    Accessing directly provides the best performance but requires compile-time knowledge
    of the data you are accessing.

    \code
    QAccelerometerReading *reading = ...;
    qreal x = reading->x();
    \endcode

    You can also access a property by name. To do this you must call QObject::property().

    \code
    qreal x = reading->property("x").value<qreal>();
    \endcode

    Finally, you can access values via numeric index.

    \code
    qreal x = reading->value(0).value<qreal>();
    \endcode

    Note that value() can only access properties declared with Q_PROPERTY() in sub-classes
    of QSensorReading.

    \sa valueCount(), QObject::property()
    \since 1.0
*/
QVariant QSensorReading::value(int index) const
{
    // get them meta-object
    const QMetaObject *mo = metaObject();

    // determine the index of the property we want
    index += mo->propertyOffset();

    // get the meta-property
    QMetaProperty property = mo->property(index);

    // read the property
    return property.read(this);
}

/*!
    \fn QSensorReading::copyValuesFrom(QSensorReading *other)
    \internal

    Copy values from other into this reading. Implemented by sub-classes
    using the DECLARE_READING() and IMPLEMENT_READING() macros.

    Note that this method should only be called by QSensorBackend.
    \since 1.0
*/
void QSensorReading::copyValuesFrom(QSensorReading *other)
{
    QSensorReadingPrivate *my_ptr = d.data();
    QSensorReadingPrivate *other_ptr = other->d.data();
    /* Do a direct copy of the private class */
    *(my_ptr) = *(other_ptr);
}

/*!
    \fn QSensorReading::d_ptr()
    \internal
    No longer used. Exists to keep the winscw build happy.
    \since 1.0
*/

/*!
    \macro DECLARE_READING(classname)
    \relates QSensorReading
    \brief The DECLARE_READING macro adds some required methods to a reading class.

    This macro should be used for all reading classes. Pass the \a classname of your reading class.

    \code
    class MyReading : public QSensorReading
    {
        Q_OBJECT
        Q_PROPERTY(qreal myprop READ myprop)
        DECLARE_READING(MyReading)
    public:
        qreal myprop() const;
        vod setMyprop(qreal myprop);
    };
    \endcode

    \sa IMPLEMENT_READING()
*/

/*!
    \macro IMPLEMENT_READING(classname)
    \relates QSensorReading
    \brief The IMPLEMENT_READING macro implements the required methods for a reading class.

    This macro should be used for all reading classes. It should be placed into a single compilation
    unit (source file), not into a header file. Pass the \a classname of your reading class.

    \code
    IMPLEMENT_READING(MyReading)
    \endcode

    \sa DECLARE_READING()
*/

#include "moc_qsensor.cpp"
QTM_END_NAMESPACE

