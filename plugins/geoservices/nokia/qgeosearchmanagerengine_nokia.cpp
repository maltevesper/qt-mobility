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
** This file is part of the Ovi services plugin for the Maps and
** Navigation API.  The use of these services, whether by use of the
** plugin or by other means, is governed by the terms and conditions
** described by the file OVI_SERVICES_TERMS_AND_CONDITIONS.txt in
** this package, located in the directory containing the Ovi services
** plugin source code.
**
****************************************************************************/

#include "qgeosearchmanagerengine_nokia.h"
#include "qgeosearchreply_nokia.h"
#include "marclanguagecodes.h"
#include "qlocationnetworkaccessmanagerfactory.h"

#include <qgeoaddress.h>
#include <qgeocoordinate.h>
#include <QNetworkProxy>
#include <QUrl>
#include <QMap>

QGeoSearchManagerEngineNokia::QGeoSearchManagerEngineNokia(const QMap<QString, QVariant> &parameters, QGeoServiceProvider::Error *error, QString *errorString)
        : QGeoSearchManagerEngine(parameters),
        m_host("loc.desktop.maps.svc.ovi.com"),
        m_token(QGeoServiceProviderFactoryNokia::defaultToken),
        m_referer(QGeoServiceProviderFactoryNokia::defaultReferer)
{
    // Get manager from declarative factory or create a new one
    m_networkManager = QLocationNetworkAccessManagerFactory::instance()->create(this);

    if (parameters.contains("places.proxy")) {
        QString proxy = parameters.value("places.proxy").toString();
        if (!proxy.isEmpty()) {
            QUrl proxyUrl(proxy);
            if (proxyUrl.isValid()) {
                m_networkManager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, 
                    proxyUrl.host(),
                    proxyUrl.port(8080),
                    proxyUrl.userName(),
                    proxyUrl.password()));
            }
        }
    }

    if (parameters.contains("places.host")) {
        QString host = parameters.value("places.host").toString();
        if (!host.isEmpty())
            m_host = host;
    }

    if (parameters.contains("places.referer")) {
        m_referer = parameters.value("places.referer").toString();
    }

    if (parameters.contains("places.token")) {
        m_token = parameters.value("places.token").toString();
    }
    else if (parameters.contains("token")) {
        m_token = parameters.value("token").toString();
    }

    if (parameters.contains("places.app_id")) {
        m_applicationId = parameters.value("places.app_id").toString();
    }
    else if (parameters.contains("app_id")) {
        m_applicationId = parameters.value("app_id").toString();
    }

    setSupportsGeocoding(true);
    setSupportsReverseGeocoding(true);

    QGeoSearchManager::SearchTypes supportedSearchTypes;
    supportedSearchTypes |= QGeoSearchManager::SearchGeocode;
    setSupportedSearchTypes(supportedSearchTypes);

    if (error)
        *error = QGeoServiceProvider::NoError;

    if (errorString)
        *errorString = "";
#ifdef USE_CHINA_NETWORK_REGISTRATION
    connect(&m_networkInfo, SIGNAL(currentMobileCountryCodeChanged(const QString&)), SLOT(currentMobileCountryCodeChanged(const QString&)));
    currentMobileCountryCodeChanged(m_networkInfo.currentMobileCountryCode());
#endif

}

QGeoSearchManagerEngineNokia::~QGeoSearchManagerEngineNokia() {}

QGeoSearchReply* QGeoSearchManagerEngineNokia::geocode(const QGeoAddress &address,
        QGeoBoundingArea *bounds)
{
    if (!supportsGeocoding()) {
        QGeoSearchReply *reply = new QGeoSearchReply(QGeoSearchReply::UnsupportedOptionError, "Geocoding is not supported by this service provider.", this);
        emit error(reply, reply->error(), reply->errorString());
        return reply;
    }

    QString requestString = "http://";
    requestString += m_host;
    requestString += "/geocoder/gc/2.0?referer=" + m_referer;

    if (!m_token.isNull())
        requestString += "&token=" + m_token;

    if (!m_applicationId.isEmpty()) {
        requestString += "&app_id=";
        requestString += m_applicationId;
    }

    requestString += "&lg=";
    requestString += languageToMarc(locale().language());

    requestString += "&country=";
    requestString += address.country();

    if (!address.state().isEmpty()) {
        requestString += "&state=";
        requestString += address.state();
    }

    if (!address.city().isEmpty()) {
        requestString += "&city=";
        requestString += address.city();
    }

    if (!address.postcode().isEmpty()) {
        requestString += "&zip=";
        requestString += address.postcode();
    }

    if (!address.street().isEmpty()) {
        requestString += "&street=";
        requestString += address.street();
    }    

    // TODO?
    // street number has been removed from QGeoAddress
    // do we need to try to split it out from QGeoAddress::street
    // in order to geocode properly

    // Old code:
//    if (!address.streetNumber().isEmpty()) {
//        requestString += "&number=";
//        requestString += address.streetNumber();
//    }

    return search(requestString, bounds);
}

QGeoSearchReply* QGeoSearchManagerEngineNokia::reverseGeocode(const QGeoCoordinate &coordinate,
        QGeoBoundingArea *bounds)
{
    if (!supportsReverseGeocoding()) {
        QGeoSearchReply *reply = new QGeoSearchReply(QGeoSearchReply::UnsupportedOptionError, "Reverse geocoding is not supported by this service provider.", this);
        emit error(reply, reply->error(), reply->errorString());
        return reply;
    }

    QString requestString = "http://";
    requestString += m_host;
    requestString += "/geocoder/rgc/2.0?referer=" + m_referer;
    if (!m_token.isNull())
        requestString += "&token=" + m_token;

    if (!m_applicationId.isEmpty()) {
        requestString += "&app_id=";
        requestString += m_applicationId;
    }

    requestString += "&long=";
    requestString += trimDouble(coordinate.longitude());
    requestString += "&lat=";
    requestString += trimDouble(coordinate.latitude());

    requestString += "&lg=";
    requestString += languageToMarc(locale().language());

    return search(requestString, bounds);
}

QGeoSearchReply* QGeoSearchManagerEngineNokia::search(const QString &searchString,
        QGeoSearchManager::SearchTypes searchTypes,
        int limit,
        int offset,
        QGeoBoundingArea *bounds)
{
    // NOTE this will eventually replaced by a much improved implementation
    // which will make use of the additionLandmarkManagers()
    if ((searchTypes != QGeoSearchManager::SearchTypes(QGeoSearchManager::SearchAll))
            && ((searchTypes & supportedSearchTypes()) != searchTypes)) {

        QGeoSearchReply *reply = new QGeoSearchReply(QGeoSearchReply::UnsupportedOptionError, "The selected search type is not supported by this service provider.", this);
        emit error(reply, reply->error(), reply->errorString());
        return reply;
    }

    QString requestString = "http://";
    requestString += m_host;
    requestString += "/geocoder/gc/2.0?referer=" + m_referer;

    if (!m_token.isNull())
        requestString += "&token=" + m_token;

    if (!m_applicationId.isEmpty()) {
        requestString += "&app_id=";
        requestString += m_applicationId;
    }

    requestString += "&lg=";
    requestString += languageToMarc(locale().language());

    requestString += "&obloc=";
    requestString += searchString;

    if (limit > 0) {
        requestString += "&total=";
        requestString += QString::number(limit);
    }

    if (offset > 0) {
        requestString += "&offset=";
        requestString += QString::number(offset);
    }

    return search(requestString, bounds, limit, offset);
}

QGeoSearchReply* QGeoSearchManagerEngineNokia::search(QString requestString,
        QGeoBoundingArea *bounds,
        int limit,
        int offset)
{
    QNetworkReply *networkReply = m_networkManager->get(QNetworkRequest(QUrl(requestString)));
    QGeoSearchReplyNokia *reply = new QGeoSearchReplyNokia(networkReply, limit, offset, bounds, this);

    connect(reply,
            SIGNAL(finished()),
            this,
            SLOT(placesFinished()));

    connect(reply,
            SIGNAL(error(QGeoSearchReply::Error, QString)),
            this,
            SLOT(placesError(QGeoSearchReply::Error, QString)));

    return reply;
}

QString QGeoSearchManagerEngineNokia::trimDouble(double degree, int decimalDigits)
{
    QString sDegree = QString::number(degree, 'g', decimalDigits);

    int index = sDegree.indexOf('.');

    if (index == -1)
        return sDegree;
    else
        return QString::number(degree, 'g', decimalDigits + index);
}

void QGeoSearchManagerEngineNokia::placesFinished()
{
    QGeoSearchReply *reply = qobject_cast<QGeoSearchReply*>(sender());

    if (!reply)
        return;

    if (receivers(SIGNAL(finished(QGeoSearchReply*))) == 0) {
        reply->deleteLater();
        return;
    }

    emit finished(reply);
}

void QGeoSearchManagerEngineNokia::placesError(QGeoSearchReply::Error error, const QString &errorString)
{
    QGeoSearchReply *reply = qobject_cast<QGeoSearchReply*>(sender());

    if (!reply)
        return;

    if (receivers(SIGNAL(error(QGeoSearchReply*, QGeoSearchReply::Error, QString))) == 0) {
        reply->deleteLater();
        return;
    }

    emit this->error(reply, error, errorString);
}

void QGeoSearchManagerEngineNokia::currentMobileCountryCodeChanged(const QString & mcc)
{
    if (mcc == "460" || mcc == "461" || mcc == "454" || mcc == "455") {
        m_host="pr.geo.maps.svc.nokia.com.cn";
    }
}
