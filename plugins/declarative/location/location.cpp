/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qdeclarativepositionsource_p.h"
#include "qdeclarativeposition_p.h"
#include "qdeclarativelandmark_p.h"
#include "qdeclarativelandmarkcategory_p.h"
#include "qdeclarativelandmarkmodel_p.h"
#include "qdeclarativelandmarkcategorymodel_p.h"

#include "qgeomapobject.h"
#include "qdeclarativegeoboundingbox_p.h"
#include "qdeclarativegeoaddress_p.h"
#include "qdeclarativegeoplace_p.h"
#include "qdeclarativecoordinate_p.h"

#include "qdeclarativegeoserviceprovider_p.h"
#include "qdeclarativegraphicsgeomap_p.h"
#include "qdeclarativegeomapgroupobject_p.h"
#include "qdeclarativegeomapobjectborder_p.h"
#include "qdeclarativegeomapcircleobject_p.h"
#include "qdeclarativegeomappixmapobject_p.h"
#include "qdeclarativegeomappolygonobject_p.h"
#include "qdeclarativegeomappolylineobject_p.h"
#include "qdeclarativegeomaprectangleobject_p.h"
#include "qdeclarativegeomaptextobject_p.h"

#include "qdeclarativegeomapmousearea_p.h"
#include "qdeclarativegeomapmouseevent_p.h"

//#include "qdeclarativegeocodemodel_p.h"
//#include "qdeclarativereversegeocodemodel_p.h"

#include <QtDeclarative/qdeclarativeextensionplugin.h>
#include <QtDeclarative/qdeclarative.h>

QT_BEGIN_NAMESPACE
QTM_USE_NAMESPACE

class QLocationDeclarativeModule: public QDeclarativeExtensionPlugin
{
    Q_OBJECT

public:
    virtual void registerTypes(const char* uri) {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtMobility.location"));
        // Elements available since Qt mobility 1.1:
        qmlRegisterType<QDeclarativePosition>(uri, 1, 1, "Position");
        qmlRegisterType<QDeclarativePositionSource>(uri, 1, 1, "PositionSource");
        qmlRegisterType<QDeclarativeLandmark>(uri, 1, 1, "Landmark");
        qmlRegisterType<QDeclarativeLandmarkModel>(uri, 1, 1, "LandmarkModel");
        qmlRegisterType<QDeclarativeLandmarkNameFilter>(uri, 1, 1, "LandmarkNameFilter");
        qmlRegisterType<QDeclarativeLandmarkCategoryFilter>(uri, 1, 1, "LandmarkCategoryFilter");
        qmlRegisterType<QDeclarativeLandmarkBoxFilter>(uri, 1, 1, "LandmarkBoxFilter");
        qmlRegisterType<QDeclarativeLandmarkProximityFilter>(uri, 1, 1, "LandmarkProximityFilter");
        qmlRegisterType<QDeclarativeLandmarkUnionFilter>(uri, 1, 1, "LandmarkUnionFilter");
        qmlRegisterType<QDeclarativeLandmarkIntersectionFilter>(uri, 1, 1, "LandmarkIntersectionFilter");
        qmlRegisterType<QDeclarativeLandmarkCategory>(uri, 1, 1, "LandmarkCategory");
        qmlRegisterType<QDeclarativeLandmarkCategoryModel>(uri, 1, 1, "LandmarkCategoryModel");
        qmlRegisterUncreatableType<QDeclarativeLandmarkFilterBase>(uri, 1, 1, "LandmarkFilterBase", QDeclarativeLandmarkFilterBase::tr("LandmarkFilterBase is an abstract class"));
        qmlRegisterUncreatableType<QDeclarativeLandmarkAbstractModel>(uri, 1, 1, "LandmarkAbstractModel", QDeclarativeLandmarkAbstractModel::tr("LandmarkAbstractModel is an abstract class"));
        qmlRegisterType<QDeclarativeCoordinate>(uri, 1, 1, "Coordinate");
        qmlRegisterType<QDeclarativeGeoBoundingBox>(uri, 1, 1, "BoundingBox");
        qmlRegisterType<QDeclarativeGeoPlace>(uri, 1, 1, "Place");
        qmlRegisterType<QDeclarativeGeoAddress>(uri, 1, 1, "Address");

        qmlRegisterType<QDeclarativeGeoServiceProvider>(uri, 1, 1, "Plugin");
        qmlRegisterType<QDeclarativeGeoServiceProviderParameter>(uri, 1, 1, "PluginParameter");
        qmlRegisterType<QDeclarativeGraphicsGeoMap>(uri, 1, 1, "Map");
        qmlRegisterType<QDeclarativeGeoMapObjectBorder>(); // used as grouped property
        qmlRegisterType<QGeoMapObject>(uri, 1, 1, "QGeoMapObject");
        qmlRegisterType<QDeclarativeGeoMapGroupObject>(uri, 1, 1, "MapGroup");
        qmlRegisterType<QDeclarativeGeoMapCircleObject>(uri, 1, 1, "MapCircle");
        qmlRegisterType<QDeclarativeGeoMapPolygonObject>(uri, 1, 1, "MapPolygon");
        qmlRegisterType<QDeclarativeGeoMapPolylineObject>(uri, 1, 1, "MapPolyline");
        qmlRegisterType<QDeclarativeGeoMapRectangleObject>(uri, 1, 1, "MapRectangle");
        qmlRegisterType<QDeclarativeGeoMapTextObject>(uri, 1, 1, "MapText");
        qmlRegisterType<QDeclarativeGeoMapPixmapObject>(uri, 1, 1, "MapImage");

        qmlRegisterType<QDeclarativeGeoMapMouseArea>(uri, 1, 2, "MapMouseArea");
        qmlRegisterType<QDeclarativeGeoMapMouseEvent>(uri, 1, 2, "MapMouseEvent");
        qmlRegisterType<QDeclarativeGeoMapObject>(uri, 1, 2, "MapObject");
        qmlRegisterType<QDeclarativeGeoMapObjectView>(uri, 1, 2, "MapObjectView");
    }
};

QT_END_NAMESPACE
#include "location.moc"

Q_EXPORT_PLUGIN2(declarative_location, QT_PREPEND_NAMESPACE(QLocationDeclarativeModule));



