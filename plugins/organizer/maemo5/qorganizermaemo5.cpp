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

#include "qorganizermaemo5_p.h"
#include "qtorganizer.h"

#include <CEvent.h>
#include <CJournal.h>
#include <CTodo.h>

QTM_USE_NAMESPACE

QOrganizerItemManagerEngine* QOrganizerItemMaemo5Factory::engine(const QMap<QString, QString>& parameters, QOrganizerItemManager::Error* error)
{
    Q_UNUSED(parameters);
    Q_UNUSED(error);


    QOrganizerItemMaemo5Engine* ret = new QOrganizerItemMaemo5Engine(); // manager takes ownership and will clean up.
    return ret;
}

QString QOrganizerItemMaemo5Factory::managerName() const
{
    return QString("maemo5");
}
Q_EXPORT_PLUGIN2(qtorganizer_maemo5, QOrganizerItemMaemo5Factory);


QOrganizerItemMaemo5Engine::~QOrganizerItemMaemo5Engine()
{
}

QString QOrganizerItemMaemo5Engine::managerName() const
{
    return QLatin1String("maemo5");
}

QMap<QString, QString> QOrganizerItemMaemo5Engine::managerParameters() const
{
    return QMap<QString, QString>();
}

int QOrganizerItemMaemo5Engine::managerVersion() const
{
    return 1;
}

QList<QOrganizerItem> QOrganizerItemMaemo5Engine::itemInstances(const QOrganizerItem& generator, const QDateTime& periodStart, const QDateTime& periodEnd, int maxCount, QOrganizerItemManager::Error* error) const
{
    /*
        TODO

        This function should create a list of instances that occur in the time period from the supplied item.
        The periodStart should always be valid, and either the periodEnd or the maxCount will be valid (if periodEnd is
        valid, use that.  Otherwise use the count).  It's permissible to limit the number of items returned...

        Basically, if the generator item is an Event, a list of EventOccurrences should be returned.  Similarly for
        Todo/TodoOccurrence.

        If there are no instances, return an empty list.

        The returned items should have a QOrganizerItemInstanceOrigin detail that points to the generator and the
        original instance that the event would have occurred on (e.g. with an exception).

        They should not have recurrence information details in them.

        We might change the signature to split up the periodStart + periodEnd / periodStart + maxCount cases.
    */

    return QOrganizerItemManagerEngine::itemInstances(generator, periodStart, periodEnd, maxCount, error);
}

QList<QOrganizerItemLocalId> QOrganizerItemMaemo5Engine::itemIds(const QOrganizerItemFilter& filter, const QList<QOrganizerItemSortOrder>& sortOrders, QOrganizerItemManager::Error* error) const
{
    *error = QOrganizerItemManager::NoError;
    int calError = 0;

    QList<QOrganizerItem> partiallyFilteredItems;

    std::vector<CCalendar*> allCalendars = d->m_mcInstance->getListCalFromMc();
    for (unsigned int i = 0; i < allCalendars.size(); i++) {
        CCalendar *currCal = allCalendars[i];
        QString calName = QString::fromStdString(currCal->getCalendarName());

	// get the events
        std::vector<CEvent*> events = currCal->getEvents(calError);
        if (calError) {
            *error = QOrganizerItemManager::UnspecifiedError;
        }
        for (unsigned int j = 0; j < events.size(); ++j) {
            partiallyFilteredItems.append(convertCEventToQEvent(events[j], calName));
            delete events[j];
        }

	// get the todos
        std::vector<CTodo*> todos = currCal->getTodos(calError);
        if (calError) {
            *error = QOrganizerItemManager::UnspecifiedError;
        }
        for (unsigned int j = 0; j < todos.size(); ++j) {
            partiallyFilteredItems.append(convertCTodoToQTodo(todos[j], calName));
            delete todos[j];
        }

	// get the journals
        std::vector<CJournal*> journals = currCal->getJournals(calError);
        if (calError) {
            *error = QOrganizerItemManager::UnspecifiedError;
        }
        for (unsigned int j = 0; j < journals.size(); ++j) {
            partiallyFilteredItems.append(convertCJournalToQJournal(journals[j], calName));
	    delete journals[j];
        }
    }
    d->m_mcInstance->releaseListCalendars(allCalendars);
    
    QList<QOrganizerItem> ret;

    foreach(const QOrganizerItem& item, partiallyFilteredItems) {
        if (QOrganizerItemManagerEngine::testFilter(filter, item)) {
            ret.append(item);
        }
    }

    return QOrganizerItemManagerEngine::sortItems(ret, sortOrders);
}

QList<QOrganizerItem> QOrganizerItemMaemo5Engine::items(const QOrganizerItemFilter& filter, const QList<QOrganizerItemSortOrder>& sortOrders, const QOrganizerItemFetchHint& fetchHint, QOrganizerItemManager::Error* error) const
{
    Q_UNUSED(fetchHint);
    *error = QOrganizerItemManager::NoError;
    int calError = 0;

    QList<QOrganizerItem> partiallyFilteredItems;

    std::vector<CCalendar*> allCalendars = d->m_mcInstance->getListCalFromMc();
    for (unsigned int i = 0; i < allCalendars.size(); i++) {
        CCalendar *currCal = allCalendars[i];
        QString calName = QString::fromStdString(currCal->getCalendarName());

	// get the events
        std::vector<CEvent*> events = currCal->getEvents(calError);
        if (calError) {
            *error = QOrganizerItemManager::UnspecifiedError;
        }
        for (unsigned int j = 0; j < events.size(); ++j) {
            partiallyFilteredItems.append(convertCEventToQEvent(events[j], calName));
            delete events[j];
        }

	// get the todos
        std::vector<CTodo*> todos = currCal->getTodos(calError);
        if (calError) {
            *error = QOrganizerItemManager::UnspecifiedError;
        }
        for (unsigned int j = 0; j < todos.size(); ++j) {
            partiallyFilteredItems.append(convertCTodoToQTodo(todos[j], calName));
            delete todos[j];
        }

	// get the journals
        std::vector<CJournal*> journals = currCal->getJournals(calError);
        if (calError) {
            *error = QOrganizerItemManager::UnspecifiedError;
        }
        for (unsigned int j = 0; j < journals.size(); ++j) {
            partiallyFilteredItems.append(convertCJournalToQJournal(journals[j], calName));
            delete journals[j];
        }
    }
    d->m_mcInstance->releaseListCalendars(allCalendars);

    QList<QOrganizerItem> ret;

    foreach(const QOrganizerItem& item, partiallyFilteredItems) {
        if (QOrganizerItemManagerEngine::testFilter(filter, item)) {
            QOrganizerItemManagerEngine::addSorted(&ret, item, sortOrders);
        }
    }

    return ret;
}

QOrganizerItem QOrganizerItemMaemo5Engine::item(const QOrganizerItemLocalId& itemId, const QOrganizerItemFetchHint& fetchHint, QOrganizerItemManager::Error* error) const
{
    return QOrganizerItemManagerEngine::item(itemId, fetchHint, error);
}

bool QOrganizerItemMaemo5Engine::saveItems(QList<QOrganizerItem>* items, QMap<int, QOrganizerItemManager::Error>* errorMap, QOrganizerItemManager::Error* error)
{
    *error = QOrganizerItemManager::NoError;

    // still need a cache of id to string
    // because on save (if update) need that string
    // so that you can extract the calendar name
    // and then check that the item still exists in that calendar
    // if not, re-add, after setting id to zero and removing entry from map.
    // if so, modify.

    int calError;
    for (int i = 0; i < items->size(); i++) {
        QOrganizerItem curr = items->at(i);
	if (curr.type() == QOrganizerItemType::TypeEvent) {
            QOrganizerEvent event = curr;
            CEvent* cevent = convertQEventToCEvent(event);
            CCalendar *curr = d->m_mcInstance->getCalendarById(cevent->getCalendarId(), calError); // XXX TODO: this line won't work, see comment above.
	    QString calName = QString::fromStdString(curr->getCalendarName());
	    if (QString::fromStdString(cevent->getId()).isEmpty()) {
                curr->modifyEvent(cevent, calError);
            } else {
                curr->addEvent(cevent, calError);
                QString hashKey = calName + ":" + QString::fromStdString(cevent->getId());
		d->m_cIdToQId.insert(hashKey, QOrganizerItemLocalId(qHash(hashKey)));
            }
	   
            if (calError) {
                if (errorMap)
                    errorMap->insert(i, QOrganizerItemManager::UnspecifiedError);
                *error = QOrganizerItemManager::UnspecifiedError;
            } else {
                items->replace(i, convertCEventToQEvent(cevent, calName));
            }
	    delete cevent;
            delete curr;
        } else if (curr.type() == QOrganizerItemType::TypeEventOccurrence) {
	} else if (curr.type() == QOrganizerItemType::TypeTodo) {
        } else if (curr.type() == QOrganizerItemType::TypeTodoOccurrence) {
        } else if (curr.type() == QOrganizerItemType::TypeJournal) {
        } else { // QOrganizerItemType::TypeNote
        }
    }
    
    return (*error == QOrganizerItemManager::NoError);
}

bool QOrganizerItemMaemo5Engine::removeItems(const QList<QOrganizerItemLocalId>& itemIds, QMap<int, QOrganizerItemManager::Error>* errorMap, QOrganizerItemManager::Error* error)
{
    /*
        TODO

        Remove a list of items, given by their id.

        If you encounter an error, insert an error into the appropriate place in the error map,
        and update the error variable as well.

        DoesNotExistError should be used if the id refers to a non existent item.
    */
    return QOrganizerItemManagerEngine::removeItems(itemIds, errorMap, error);
}

QMap<QString, QOrganizerItemDetailDefinition> QOrganizerItemMaemo5Engine::detailDefinitions(const QString& itemType, QOrganizerItemManager::Error* error) const
{
    /* TODO - once you know what your engine will support, implement this properly.  One way is to call the base version, and add/remove things as needed */
    return detailDefinitions(itemType, error);
}

QOrganizerItemDetailDefinition QOrganizerItemMaemo5Engine::detailDefinition(const QString& definitionId, const QString& itemType, QOrganizerItemManager::Error* error) const
{
    /* TODO - the default implementation just calls the base detailDefinitions function.  If that's inefficent, implement this */
    return QOrganizerItemManagerEngine::detailDefinition(definitionId, itemType, error);
}

bool QOrganizerItemMaemo5Engine::saveDetailDefinition(const QOrganizerItemDetailDefinition& def, const QString& itemType, QOrganizerItemManager::Error* error)
{
    /* TODO - if you support adding custom fields, do that here.  Otherwise call the base functionality. */
    return QOrganizerItemManagerEngine::saveDetailDefinition(def, itemType, error);
}

bool QOrganizerItemMaemo5Engine::removeDetailDefinition(const QString& definitionId, const QString& itemType, QOrganizerItemManager::Error* error)
{
    /* TODO - if you support removing custom fields, do that here.  Otherwise call the base functionality. */
    return QOrganizerItemManagerEngine::removeDetailDefinition(definitionId, itemType, error);
}

bool QOrganizerItemMaemo5Engine::startRequest(QOrganizerItemAbstractRequest* req)
{
    /*
        TODO

        This is the entry point to the async API.  The request object describes the
        type of request (switch on req->type()).  Req will not be null when called
        by the framework.

        Generally, you can queue the request and process them at some later time
        (probably in another thread).

        Once you start a request, call the updateRequestState and/or the
        specific updateXXXXXRequest functions to mark it in the active state.

        If your engine is particularly fast, or the operation involves only in
        memory data, you can process and complete the request here.  That is
        probably not the case, though.

        Note that when the client is threaded, and the request might live on a
        different thread, you might need to be careful with locking.  In particular,
        the request might be deleted while you are still working on it.  In this case,
        your requestDestroyed function will be called while the request is still valid,
        and you should block in that function until your worker thread (etc) has been
        notified not to touch that request any more.

        We plan to provide some boiler plate code that will allow you to:

        1) implement the sync functions, and have the async versions call the sync
           in another thread

        2) or implement the async versions of the function, and have the sync versions
           call the async versions.

        It's not ready yet, though.

        Return true if the request can be started, false otherwise.  You can set an error
        in the request if you like.
    */
    return QOrganizerItemManagerEngine::startRequest(req);
}

bool QOrganizerItemMaemo5Engine::cancelRequest(QOrganizerItemAbstractRequest* req)
{
    /*
        TODO

        Cancel an in progress async request.  If not possible, return false from here.
    */
    return QOrganizerItemManagerEngine::cancelRequest(req);
}

bool QOrganizerItemMaemo5Engine::waitForRequestFinished(QOrganizerItemAbstractRequest* req, int msecs)
{
    /*
        TODO

        Wait for a request to complete (up to a max of msecs milliseconds).

        Return true if the request is finished (including if it was already).  False otherwise.

        You should really implement this function, if nothing else than as a delay, since clients
        may call this in a loop.

        It's best to avoid processing events, if you can, or at least only process non-UI events.
    */
    return QOrganizerItemManagerEngine::waitForRequestFinished(req, msecs);
}

void QOrganizerItemMaemo5Engine::requestDestroyed(QOrganizerItemAbstractRequest* req)
{
    /*
        TODO

        This is called when a request is being deleted.  It lets you know:

        1) the client doesn't care about the request any more.  You can still complete it if
           you feel like it.
        2) you can't reliably access any properties of the request pointer any more.  The pointer will
           be invalid once this function returns.

        This means that if you have a worker thread, you need to let that thread know that the
        request object is not valid and block until that thread acknowledges it.  One way to do this
        is to have a QSet<QOIAR*> (or QMap<QOIAR, MyCustomRequestState>) that tracks active requests, and
        insert into that set in startRequest, and remove in requestDestroyed (or when it finishes or is
        cancelled).  Protect that set/map with a mutex, and make sure you take the mutex in the worker
        thread before calling any of the QOIAR::updateXXXXXXRequest functions.  And be careful of lock
        ordering problems :D

    */
    return QOrganizerItemManagerEngine::requestDestroyed(req);
}

bool QOrganizerItemMaemo5Engine::hasFeature(QOrganizerItemManager::ManagerFeature feature, const QString& itemType) const
{
    // TODO - the answer to the question may depend on the type
    Q_UNUSED(itemType);
    switch(feature) {
        case QOrganizerItemManager::MutableDefinitions:
            // TODO If you support save/remove detail definition, return true
            return false;

        case QOrganizerItemManager::Anonymous:
            // TODO if this engine is anonymous (e.g. no other engine can share the data) return true
            // (mostly for an in memory engine)
            return false;
        case QOrganizerItemManager::ChangeLogs:
            // TODO if this engine supports filtering by last modified/created/removed timestamps, return true
            return false;
    }
    return false;
}

bool QOrganizerItemMaemo5Engine::isFilterSupported(const QOrganizerItemFilter& filter) const
{
    // TODO if you engine can natively support the filter, return true.  Otherwise you should emulate support in the item{Ids} functions.
    Q_UNUSED(filter);
    return false;
}

QList<QVariant::Type> QOrganizerItemMaemo5Engine::supportedDataTypes() const
{
    QList<QVariant::Type> ret;
    // TODO - tweak which data types this engine understands
    ret << QVariant::String;
    ret << QVariant::Date;
    ret << QVariant::DateTime;
    ret << QVariant::Time;

    return ret;
}

QStringList QOrganizerItemMaemo5Engine::supportedItemTypes() const
{
    // TODO - return which [predefined] types this engine supports
    QStringList ret;

    ret << QOrganizerItemType::TypeEvent;
    ret << QOrganizerItemType::TypeEventOccurrence;
    ret << QOrganizerItemType::TypeJournal;
    ret << QOrganizerItemType::TypeNote;
    ret << QOrganizerItemType::TypeTodo;
    ret << QOrganizerItemType::TypeTodoOccurrence;

    return ret;
}


QOrganizerEvent QOrganizerItemMaemo5Engine::convertCEventToQEvent(CEvent* cevent, const QString& calendarName) const
{
    QOrganizerEvent ret;
    ret.setLocationGeoCoordinates(QString(QLatin1String(cevent->getGeo().data())));
    ret.setPriority(static_cast<QOrganizerItemPriority::Priority>(cevent->getPriority())); // assume that the saved priority is vCal compliant.
    ret.setDisplayLabel(QString(QLatin1String(cevent->getSummary().data())));
    ret.setDescription(QString(QLatin1String(cevent->getDescription().data())));
    ret.setLocationName(QString(QLatin1String(cevent->getLocation().data())));
    ret.setStartDateTime(QDateTime::fromTime_t(cevent->getDateStart()));
    ret.setEndDateTime(QDateTime::fromTime_t(cevent->getDateEnd()));

    QOrganizerItemId rId;
    rId.setManagerUri(managerUri());
    QString hashKey = calendarName + ":" + QString::fromStdString(cevent->getId());
    rId.setLocalId(QOrganizerItemLocalId(qHash(hashKey)));
    ret.setId(rId);
    // and the recurrence information...

    return ret;
}

QOrganizerTodo QOrganizerItemMaemo5Engine::convertCTodoToQTodo(CTodo* ctodo, const QString& calendarName) const
{
    QOrganizerTodo ret;
    ret.setPriority(static_cast<QOrganizerItemPriority::Priority>(ctodo->getPriority()));
    ret.setNotBeforeDateTime(QDateTime::fromTime_t(ctodo->getDateStart()));
    ret.setDisplayLabel(QString(QLatin1String(ctodo->getSummary().data())));
    ret.setDueDateTime(QDateTime::fromTime_t(ctodo->getDue()));

    QOrganizerItemId rId;
    rId.setManagerUri(managerUri());
    QString hashKey = calendarName + ":" + QString::fromStdString(ctodo->getId());
    rId.setLocalId(QOrganizerItemLocalId(qHash(hashKey)));
    ret.setId(rId);
    // and the recurrence information

    return ret;
}

QOrganizerTodoOccurrence QOrganizerItemMaemo5Engine::convertCTodoToQTodoOccurrence(CTodo* ctodo, const QString& calendarName) const
{
    QOrganizerTodoOccurrence ret;
    ret.setPriority(static_cast<QOrganizerItemPriority::Priority>(ctodo->getPriority()));
    ret.setNotBeforeDateTime(QDateTime::fromTime_t(ctodo->getDateStart()));
    ret.setDisplayLabel(QString(QLatin1String(ctodo->getSummary().data())));
    ret.setDueDateTime(QDateTime::fromTime_t(ctodo->getDue()));
    ret.setProgressPercentage(ctodo->getPercentComplete());
    ret.setStatus(static_cast<QOrganizerItemTodoProgress::Status>(ctodo->getStatus()));
    ret.setFinishedDateTime(QDateTime::fromTime_t(ctodo->getCompleted()));
    ret.setStartedDateTime(QDateTime::fromTime_t(ctodo->getDateStart()));
    
    // XXX TODO: set the parent information stuff.
    QOrganizerItemId rId;
    rId.setManagerUri(managerUri());
    QString hashKey = calendarName + ":" + QString::fromStdString(ctodo->getId());
    rId.setLocalId(QOrganizerItemLocalId(qHash(hashKey)));
    ret.setId(rId);

    return ret;
}

QOrganizerJournal QOrganizerItemMaemo5Engine::convertCJournalToQJournal(CJournal* cjournal, const QString& calendarName) const
{
    QOrganizerJournal ret;
    ret.setDescription(QString(QLatin1String(cjournal->getDescription().data())));

    QOrganizerItemId rId;
    rId.setManagerUri(managerUri());
    QString hashKey = calendarName + ":" + QString::fromStdString(cjournal->getId());
    rId.setLocalId(QOrganizerItemLocalId(qHash(hashKey)));
    ret.setId(rId);
    
    return ret;
}

CEvent* QOrganizerItemMaemo5Engine::convertQEventToCEvent(const QOrganizerEvent& event) const
{
    // first grab the QOILId, dehash it, find the calendar name, and then
    // set the id and calendar id in the event item.
    // then set all of the details and return.

    CEvent* ret = new CEvent;
    return ret;
}


