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

#ifndef QGSTREAMERMETADATAPROVIDER_H
#define QGSTREAMERMETADATAPROVIDER_H

#include <qmetadatacontrol.h>

#include <Phonon/MediaObject>

class QPhononPlayerSession;

class QPhononMetaDataProvider : public QMetaDataControl
{
    Q_OBJECT
public:
    QPhononMetaDataProvider( Phonon::MediaObject *session, QObject *parent );
    virtual ~QPhononMetaDataProvider();

    bool isMetaDataAvailable() const;
    bool isWritable() const;

    QVariant metaData(QtMedia::MetaData key) const;
    void setMetaData(QtMedia::MetaData key, const QVariant &value);
    QList<QtMedia::MetaData> availableMetaData() const;

    QVariant extendedMetaData(const QString &key) const;
    void setExtendedMetaData(const QString &key, const QVariant &value);
    QStringList availableExtendedMetaData() const;

private slots:
    void updateTags();

private:
    Phonon::MediaObject *m_session;
    bool m_metaDataAvailable;
    QMap<QString, QtMedia::MetaData> m_keysMap;
};

#endif // QGSTREAMERMETADATAPROVIDER_H