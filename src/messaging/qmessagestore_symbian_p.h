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
#ifndef QMESSAGESTOREPRIVATE_SYMBIAN_H
#define QMESSAGESTOREPRIVATE_SYMBIAN_H

#include "qmessagestore.h"

class CMTMEngine;

class QMessageStorePrivate
{
public:
    enum NotificationType
    {
        Added,
        Updated,
        Removed
    };

    QMessageStorePrivate();
    ~QMessageStorePrivate();
    
    void initialize(QMessageStore *store);
    
    QMessageAccountIdList queryAccounts(const QMessageAccountFilter &filter = QMessageAccountFilter(), const QMessageAccountOrdering &ordering = QMessageAccountOrdering(), uint limit = 0, uint offset = 0) const;
    int countAccounts(const QMessageAccountFilter &filter = QMessageAccountFilter()) const;
    QMessageAccount account(const QMessageAccountId &id) const;

    QMessageFolderIdList queryFolders(const QMessageFolderFilter &filter = QMessageFolderFilter(), const QMessageFolderOrdering &ordering = QMessageFolderOrdering(), uint limit = 0, uint offset = 0) const;
    int countFolders(const QMessageFolderFilter &filter = QMessageFolderFilter()) const;
    QMessageFolder folder(const QMessageFolderId &id) const;
    
    bool addMessage(QMessage *m);
    bool updateMessage(QMessage *m);
    bool removeMessage(const QMessageId &id, QMessageStore::RemovalOption option);
    bool removeMessages(const QMessageFilter &filter, QMessageStore::RemovalOption option);
    
    QMessage message(const QMessageId& id) const;
    
    QMessageStore::NotificationFilterId registerNotificationFilter(const QMessageFilter &filter);
    void unregisterNotificationFilter(QMessageStore::NotificationFilterId notificationFilterId);

    void messageNotification(QMessageStorePrivate::NotificationType type, const QMessageId& id,
                             const QMessageStore::NotificationFilterIdSet &matchingFilters);    
private:    
    QMessageStore* q_ptr;

    CMTMEngine* _mtmEngine;
    QMessageStore::ErrorCode _error;
    
    NotificationType _notificationType;
    QMessageIdList _ids;
    
    friend class QMessageStore;
};

#endif // QMESSAGESTOREPRIVATE_SYMBIAN_H