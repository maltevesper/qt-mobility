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

#include "playerresourcepolicy.h"

#ifdef Q_WS_MAEMO_6
#define HAVE_RESOURCE_POLICY
#endif

//#define DEBUG_RESOURCE_POLICY
#include <QtCore/qdebug.h>

#ifdef HAVE_RESOURCE_POLICY
#include <policy/resource.h>
#include <policy/resources.h>
#include <policy/resource-set.h>
#endif

PlayerResourcePolicy::PlayerResourcePolicy(QObject *parent) :
    QObject(parent),
    m_videoEnabled(true),
    m_resourceSet(0),
    m_status(PlayerResourcePolicy::Initial)
{
#ifdef HAVE_RESOURCE_POLICY
    m_resourceSet = new ResourcePolicy::ResourceSet("player", this);
    m_resourceSet->setAlwaysReply();

    ResourcePolicy::AudioResource *audioResource = new ResourcePolicy::AudioResource("player");
    audioResource->setProcessID(QCoreApplication::applicationPid());
    audioResource->setStreamTag("media.name", "*");
    m_resourceSet->addResourceObject(audioResource);

    m_resourceSet->addResource(ResourcePolicy::VideoPlaybackType);
    m_resourceSet->update();

    connect(m_resourceSet, SIGNAL(resourcesGranted(const QList<ResourcePolicy::ResourceType>)),
            this, SLOT(handleResourcesGranted()));
    connect(m_resourceSet, SIGNAL(resourcesDenied()),
            this, SLOT(handleResourcesDenied()));
    connect(m_resourceSet, SIGNAL(lostResources()),
            this, SLOT(handleResourcesLost()));
    connect(m_resourceSet, SIGNAL(resourcesReleasedByManager()),
            this, SLOT(handleResourcesLost()));
#endif
}

PlayerResourcePolicy::~PlayerResourcePolicy()
{
}

bool PlayerResourcePolicy::isVideoEnabled() const
{
    return m_videoEnabled;
}

void PlayerResourcePolicy::setVideoEnabled(bool enabled)
{
    if (m_videoEnabled != enabled) {
        m_videoEnabled = enabled;

#ifdef HAVE_RESOURCE_POLICY
        if (enabled)
            m_resourceSet->addResource(ResourcePolicy::VideoPlaybackType);
        else
            m_resourceSet->deleteResource(ResourcePolicy::VideoPlaybackType);

        m_resourceSet->update();
#endif
    }
}

void PlayerResourcePolicy::acquire()
{
#ifdef HAVE_RESOURCE_POLICY

#ifdef DEBUG_RESOURCE_POLICY
    qDebug() << Q_FUNC_INFO << "Acquire resource";
#endif
    m_status = RequestedResource;
    m_resourceSet->acquire();
#else
    m_status = GrantedResource;
#endif
}

void PlayerResourcePolicy::release()
{
#ifdef HAVE_RESOURCE_POLICY

#ifdef DEBUG_RESOURCE_POLICY
    qDebug() << Q_FUNC_INFO << "Release resource";
#endif

    m_resourceSet->release();
#endif
    m_status = Initial;

}

bool PlayerResourcePolicy::isGranted() const
{
    return m_status == GrantedResource;
}

bool PlayerResourcePolicy::isRequested() const
{
    return m_status == RequestedResource;
}

void PlayerResourcePolicy::handleResourcesGranted()
{
    m_status = GrantedResource;
#ifdef DEBUG_RESOURCE_POLICY
    qDebug() << Q_FUNC_INFO << "Resource granted";
#endif
    emit resourcesGranted();
}

void PlayerResourcePolicy::handleResourcesDenied()
{
    m_status = Initial;
#ifdef DEBUG_RESOURCE_POLICY
    qDebug() << Q_FUNC_INFO << "Resource denied";
#endif
    emit resourcesDenied();
}

void PlayerResourcePolicy::handleResourcesLost()
{
#ifdef DEBUG_RESOURCE_POLICY
    qDebug() << Q_FUNC_INFO << "Resource lost";
#endif
    if (m_status != Initial) {
        m_status = Initial;
        emit resourcesLost();
    }

#ifdef HAVE_RESOURCE_POLICY
    m_resourceSet->release();
#endif
}
