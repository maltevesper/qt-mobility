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

#ifndef QMEDIAPLAYLISTNAVIGATOR_H
#define QMEDIAPLAYLISTNAVIGATOR_H

#include <qmediaplaylistprovider.h>
#include <qmediaplaylist.h>
#include <QtCore/qobject.h>

class QMediaPlaylistNavigatorPrivate;
class Q_MEDIA_EXPORT QMediaPlaylistNavigator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QMediaPlaylist::PlaybackMode playbackMode READ playbackMode WRITE setPlaybackMode NOTIFY playbackModeChanged)
    Q_PROPERTY(int currentPosition READ currentPosition WRITE jump NOTIFY currentPositionChanged)
    Q_PROPERTY(QMediaContent currentItem READ currentItem NOTIFY currentItemChanged)
    Q_PROPERTY(QMediaContent nextItem READ nextItem)
    Q_PROPERTY(QMediaContent previousItem READ previousItem)

public:
    QMediaPlaylistNavigator(QMediaPlaylistProvider *playlist, QObject *parent = 0);
    virtual ~QMediaPlaylistNavigator();

    QMediaPlaylistProvider *playlist() const;
    void setPlaylist(QMediaPlaylistProvider *playlist);

    QMediaPlaylist::PlaybackMode playbackMode() const;

    QMediaContent currentItem() const;
    QMediaContent nextItem(int steps = 1) const;
    QMediaContent previousItem(int steps = 1) const;

    QMediaContent itemAt(int position) const;

    int currentPosition() const;
    int nextPosition(int steps = 1) const;
    int previousPosition(int steps = 1) const;

public Q_SLOTS:
    void next();
    void previous();

    void jump(int);

    void setPlaybackMode(QMediaPlaylist::PlaybackMode mode);

Q_SIGNALS:
    void activated(const QMediaContent &content);
    void currentPositionChanged(int);
    void playbackModeChanged(QMediaPlaylist::PlaybackMode mode);

    void surroundingItemsChanged();

protected:
    QMediaPlaylistNavigatorPrivate *d_ptr;

private:
    Q_DISABLE_COPY(QMediaPlaylistNavigator)
    Q_DECLARE_PRIVATE(QMediaPlaylistNavigator)

    Q_PRIVATE_SLOT(d_func(), void _q_itemsInserted(int start, int end))
    Q_PRIVATE_SLOT(d_func(), void _q_itemsRemoved(int start, int end))
    Q_PRIVATE_SLOT(d_func(), void _q_itemsChanged(int start, int end))
};

#endif // QMEDIAPLAYLISTNAVIGATOR_H