/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qgeotiledmaprequest.h"
#include "qgeotiledmaprequest_p.h"

#include "qgeotiledmapdata.h"

QTM_BEGIN_NAMESPACE

QGeoTiledMapRequest::QGeoTiledMapRequest()
        : d_ptr(new QGeoTiledMapRequestPrivate()) {}

/*!
*/
QGeoTiledMapRequest::QGeoTiledMapRequest(QGeoTiledMapData *mapData, int row, int column, const QRect &tileRect)
        : d_ptr(new QGeoTiledMapRequestPrivate())
{
    d_ptr->mapData = mapData;
    d_ptr->row = row;
    d_ptr->column = column;
    d_ptr->tileRect = tileRect;

    d_ptr->zoomLevel = mapData->zoomLevel();
    d_ptr->mapType = mapData->mapType();
    //d_ptr->imageFormat = viewport->imageFormat();
}

/*!
*/
QGeoTiledMapRequest::QGeoTiledMapRequest(const QGeoTiledMapRequest &other)
        : d_ptr(other.d_ptr) {}

/*!
*/
QGeoTiledMapRequest::~QGeoTiledMapRequest() {}

/*!
*/
QGeoTiledMapRequest& QGeoTiledMapRequest::operator= (const QGeoTiledMapRequest & other)
{
    d_ptr = other.d_ptr;

    return *this;
}

bool QGeoTiledMapRequest::operator== (const QGeoTiledMapRequest &other) const
{
    return (d_ptr->row == other.d_ptr->row) &&
           (d_ptr->column == other.d_ptr->column) &&
           (d_ptr->zoomLevel == other.d_ptr->zoomLevel) &&
           (d_ptr->mapType == other.d_ptr->mapType);
}

/*!
*/
QGeoTiledMapData* QGeoTiledMapRequest::mapData() const
{
    return d_ptr->mapData;
}

/*!
*/
QGraphicsGeoMap::MapType QGeoTiledMapRequest::mapType() const
{
    return d_ptr->mapType;
}

/*!
*/
int QGeoTiledMapRequest::zoomLevel() const
{
    return d_ptr->zoomLevel;
}

/*!
*/
int QGeoTiledMapRequest::row() const
{
    return d_ptr->row;
}

/*!
*/
int QGeoTiledMapRequest::column() const
{
    return d_ptr->column;
}

/*!
*/
QRect QGeoTiledMapRequest::tileRect() const
{
    return d_ptr->tileRect;
}

uint qHash(const QGeoTiledMapRequest &key)
{
    uint result = QT_PREPEND_NAMESPACE(qHash)(key.row() * 13);
    result += QT_PREPEND_NAMESPACE(qHash)(key.column() * 17);
    result += QT_PREPEND_NAMESPACE(qHash)(key.zoomLevel() * 19);
    result += QT_PREPEND_NAMESPACE(qHash)(static_cast<int>(key.mapType()));
    return result;
}

/*******************************************************************************
*******************************************************************************/

QGeoTiledMapRequestPrivate::QGeoTiledMapRequestPrivate()
        : QSharedData(),
        mapData(0) {}

QGeoTiledMapRequestPrivate::QGeoTiledMapRequestPrivate(const QGeoTiledMapRequestPrivate &other)
        : QSharedData(other),
        mapData(other.mapData),
        mapType(other.mapType),
        zoomLevel(other.zoomLevel),
        row(other.row),
        column(other.column),
        tileRect(other.tileRect) {}

QGeoTiledMapRequestPrivate::~QGeoTiledMapRequestPrivate() {}

QGeoTiledMapRequestPrivate& QGeoTiledMapRequestPrivate::operator= (const QGeoTiledMapRequestPrivate & other)
{
    mapData = other.mapData;
    mapType = other.mapType;
    zoomLevel = other.zoomLevel;
    row = other.row;
    column = other.column;
    tileRect = other.tileRect;

    return *this;
}

QTM_END_NAMESPACE
