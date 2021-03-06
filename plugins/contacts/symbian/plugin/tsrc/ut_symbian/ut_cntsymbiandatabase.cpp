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

#include "ut_cntsymbiandatabase.h"
#include "cntsymbiandatabase.h"
#include "cntsymbianengine.h"
#include <QtTest/QtTest>

void TestCntSymbianDatabase::initTestCase()
{
    QContactManager::Error error;
    QMap<QString, QString> emptyParameters;
    m_engine = new CntSymbianEngine(emptyParameters, &error);
    if (error == QContactManager::NoError)
        removeAllContacts();
    else
        QSKIP("Error creating manager", SkipAll);
    
    connect(m_engine, SIGNAL(contactsAdded(const QList<QContactLocalId>&)), this, SLOT(handleAddedOrig(const QList<QContactLocalId>&)));
    connect(m_engine, SIGNAL(contactsChanged(const QList<QContactLocalId>&)), this, SLOT(handleChangedOrig(const QList<QContactLocalId>&)));
    connect(m_engine, SIGNAL(contactsRemoved(const QList<QContactLocalId>&)), this, SLOT(handleRemovedOrig(const QList<QContactLocalId>&)));
    connect(m_engine, SIGNAL(selfContactIdChanged(const QContactLocalId&, const QContactLocalId&)), this, SLOT(handleMyCardChangedOrig(const QContactLocalId&, const QContactLocalId&)));    
    connect(m_engine, SIGNAL(relationshipsAdded(QList<QContactLocalId>)), this, SLOT(handleRelationshipsAddedOrig(QList<QContactLocalId>)));
    connect(m_engine, SIGNAL(relationshipsRemoved(QList<QContactLocalId>)), this, SLOT(handleRelationshipsRemovedOrig(QList<QContactLocalId>)));
}

void TestCntSymbianDatabase::cleanupTestCase()
{
    removeAllContacts();
    delete m_engine;
}

void TestCntSymbianDatabase::init()
{
    addedContactId = 0;
    addedOrigContactId = 0;
    changedContactId = 0;
    changedOrigContactId = 0;
    removedContactId.clear();
    removedOrigContactId.clear();
    ownCardId = 0;
    ownCardOrigId = 0;
    addedRelId1 = 0;
    addedRelId2 = 0;
    addedRelOrigId1 = 0;
    addedRelOrigId2 = 0;
    removedRelId1 = 0;
    removedRelId2 = 0;
    removedRelOrigId1 = 0;
    removedRelOrigId2 = 0;
}

void TestCntSymbianDatabase::cleanup()
{
    removeAllContacts();
}

void TestCntSymbianDatabase::removeAllContacts()
{
    if(m_engine) {
        // Empty cnt database
        QContactManager::Error err(QContactManager::NoError);
        QList<QContactSortOrder> sortOrders;
        QContactFilter defaultFilter = QContactFilter();
        QList<QContactLocalId> cnts_ids = m_engine->contactIds(defaultFilter, sortOrders, &err);
        QVERIFY(err == QContactManager::NoError);

        for(int i=0; i<cnts_ids.count(); i++) {
            QVERIFY(m_engine->removeContact(cnts_ids[i], &err));
        }
    }
}

void TestCntSymbianDatabase::contactDatabase()
{
    QContactManagerEngine *engine = 0;
    QContactManager::Error error;
    CntSymbianDatabase *db;
    db = new CntSymbianDatabase(engine, &error);
    QVERIFY(db != 0);
    //cntsymbiandatabase instantiation should fail if provided QContactManagerEngine is null 
    QVERIFY(error != QContactManager::NoError);

    CContactDatabase *cdb = db->contactDatabase();
    QVERIFY(cdb == 0);
}

void TestCntSymbianDatabase::createChangeRemoveContactEvents()
{
    // one more engine to check that it also gets db events
    QContactManager::Error error;
    QMap<QString, QString> emptyParameters;
    CntSymbianEngine* engine = new CntSymbianEngine(emptyParameters, &error);
    QVERIFY(error == QContactManager::NoError);
    connect(engine, SIGNAL(contactsAdded(const QList<QContactLocalId>&)), this, SLOT(handleAdded(const QList<QContactLocalId>&)));
    connect(engine, SIGNAL(contactsChanged(const QList<QContactLocalId>&)), this, SLOT(handleChanged(const QList<QContactLocalId>&)));
    connect(engine, SIGNAL(contactsRemoved(const QList<QContactLocalId>&)), this, SLOT(handleRemoved(const QList<QContactLocalId>&)));

    // save new contact
    QContact c;
    c.setType(QContactType::TypeContact);
    QVERIFY(m_engine->saveContact(&c, &error));
    QVERIFY(error == QContactManager::NoError);

    // wait a bit and check that both engines got an event about new contact
    QTest::qWait(1000); //1sec
    QVERIFY(addedOrigContactId != 0);
    QVERIFY(addedOrigContactId == addedContactId);
    
    // update existing contact
    QContactName name;
    name.setFirstName("name");
    c.saveDetail(&name);
    QVERIFY(m_engine->saveContact(&c, &error));
    QVERIFY(error == QContactManager::NoError);

    // wait a bit and check that both engines got an event about changed contact
    QTest::qWait(1000); //1sec
    QVERIFY(changedOrigContactId != 0);
    QVERIFY(changedOrigContactId == changedContactId);
    
    // remove contact
    QVERIFY(m_engine->removeContact(c.localId(), &error));
    QVERIFY(error == QContactManager::NoError);

    // wait a bit and check that both engines got an event about changed contact
    QTest::qWait(1000); //1sec
    QVERIFY(removedOrigContactId.count() != 0);
    QVERIFY(removedOrigContactId == removedContactId);
    
    delete engine;
}

void TestCntSymbianDatabase::batchRemoveOperation()
{
    // one more engine to check that it also gets db events
    QContactManager::Error error;
    QMap<int, QContactManager::Error> errorMap;
    QMap<QString, QString> emptyParameters;
    CntSymbianEngine* engine = new CntSymbianEngine(emptyParameters, &error);
    QVERIFY(error == QContactManager::NoError);
    connect(engine, SIGNAL(contactsAdded(const QList<QContactLocalId>&)), this, SLOT(handleAdded(const QList<QContactLocalId>&)));
    connect(engine, SIGNAL(contactsChanged(const QList<QContactLocalId>&)), this, SLOT(handleChanged(const QList<QContactLocalId>&)));
    connect(engine, SIGNAL(contactsRemoved(const QList<QContactLocalId>&)), this, SLOT(handleRemoved(const QList<QContactLocalId>&)));
    
    // save and delete 2 contacts
    QContact c1;
    c1.setType(QContactType::TypeContact);
    QVERIFY(m_engine->saveContact(&c1, &error));
    QVERIFY(error == QContactManager::NoError);
    QContact c2;
    c2.setType(QContactType::TypeContact);
    QVERIFY(m_engine->saveContact(&c2, &error));
    QVERIFY(error == QContactManager::NoError);
    
    QList<QContactLocalId> contacts;
    contacts.append(c1.localId());
    contacts.append(c2.localId());
    QVERIFY(m_engine->removeContacts(contacts, &errorMap, &error));
    QVERIFY(error == QContactManager::NoError);
    
    // wait a bit and check that both engines got an event about deleted contacts
    QTest::qWait(1000); //1sec
    QVERIFY(removedOrigContactId.count() != 0);
    QVERIFY(removedOrigContactId == removedContactId);
    
    delete engine;
}

void TestCntSymbianDatabase::ownCardEvents()
{
    // one more engine to check that it also gets db events
    QContactManager::Error error;
    QMap<QString, QString> emptyParameters;
    CntSymbianEngine* engine = new CntSymbianEngine(emptyParameters, &error);
    QVERIFY(error == QContactManager::NoError);
    connect(engine, SIGNAL(selfContactIdChanged(const QContactLocalId&, const QContactLocalId&)), this, SLOT(handleMyCardChanged(const QContactLocalId&, const QContactLocalId&)));
    // save new contact and assign it as my card
    QContact c;
    c.setType(QContactType::TypeContact);
    QVERIFY(m_engine->saveContact(&c, &error));
    QVERIFY(error == QContactManager::NoError);
    QVERIFY(m_engine->setSelfContactId(c.localId(), &error));
    QVERIFY(error == QContactManager::NoError);

    // wait a bit and check that both engines got an event about changed own card
    QTest::qWait(1000); //1sec
    QVERIFY(ownCardId != 0);
    QVERIFY(ownCardId == ownCardOrigId);
    
    delete engine;
}

void TestCntSymbianDatabase::relationshipEvents()
{
    // one more engine to check that it also gets db events
    QContactManager::Error error;
    QMap<QString, QString> emptyParameters;
    CntSymbianEngine* engine = new CntSymbianEngine(emptyParameters, &error);
    QVERIFY(error == QContactManager::NoError);
    connect(engine, SIGNAL(relationshipsAdded(QList<QContactLocalId>)), this, SLOT(handleRelationshipsAdded(QList<QContactLocalId>)));
    connect(engine, SIGNAL(relationshipsRemoved(QList<QContactLocalId>)), this, SLOT(handleRelationshipsRemoved(QList<QContactLocalId>)));
    // save new contact and group
    QContact c;
    c.setType(QContactType::TypeContact);
    QVERIFY(m_engine->saveContact(&c, &error));
    QVERIFY(error == QContactManager::NoError);
    QContact g;
    g.setType(QContactType::TypeGroup);
    QVERIFY(m_engine->saveContact(&g, &error));
    QVERIFY(error == QContactManager::NoError);
    
    //add a relationship between group and contact
    QContactRelationship relationship;
    relationship.setRelationshipType(QContactRelationship::HasMember);
    relationship.setFirst(g.id());
    relationship.setSecond(c.id());
    QVERIFY(m_engine->saveRelationship(&relationship, &error));
    QVERIFY(error == QContactManager::NoError);
    
    // wait a bit and check that both engines got an event about added relationship
    QTest::qWait(1000); //1sec
    QVERIFY(addedRelId1 != 0);
    QVERIFY(addedRelId2 != 0);
    QVERIFY(addedRelId1 == addedRelOrigId1);
    QVERIFY(addedRelId2 == addedRelOrigId2);
    
    //remove a relationship between group and contact
    QVERIFY(m_engine->removeRelationship(relationship, &error));
    QVERIFY(error == QContactManager::NoError);
    
    // wait a bit and check that both engines got an event about removed relationship
    QTest::qWait(1000); //1sec
    QVERIFY(removedRelId1 != 0);
    QVERIFY(removedRelId2 != 0);
    QVERIFY(removedRelId1 == removedRelOrigId1);
    QVERIFY(removedRelId2 == removedRelOrigId2);
    
    delete engine;
}

void TestCntSymbianDatabase::handleAddedOrig(const QList<QContactLocalId>& contactIds)
{
    if (contactIds.count() > 0) {
        addedOrigContactId = contactIds.at(0);
    }
}

void TestCntSymbianDatabase::handleChangedOrig(const QList<QContactLocalId>& contactIds)
{
    if (contactIds.count() > 0) {
        changedOrigContactId = contactIds.at(0);
    }
}

void TestCntSymbianDatabase::handleRemovedOrig(const QList<QContactLocalId>& contactIds)
{
    removedOrigContactId.append(contactIds);
}

void TestCntSymbianDatabase::handleMyCardChangedOrig(const QContactLocalId& /*oldId*/, const QContactLocalId& newId)
{
    ownCardOrigId = newId;
}

void TestCntSymbianDatabase::handleRelationshipsAddedOrig(QList<QContactLocalId> contactIds)
{
    if (contactIds.count() > 1) {
        addedRelOrigId1 = contactIds.at(0);
        addedRelOrigId2 = contactIds.at(1);
    }
}

void TestCntSymbianDatabase::handleRelationshipsRemovedOrig(QList<QContactLocalId> contactIds)
{
    if (contactIds.count() > 1) {
        removedRelOrigId1 = contactIds.at(0);
        removedRelOrigId2 = contactIds.at(1);
    }
    
}

void TestCntSymbianDatabase::handleAdded(const QList<QContactLocalId>& contactIds)
{
    if (contactIds.count() > 0) {
        addedContactId = contactIds.at(0);
    }
}

void TestCntSymbianDatabase::handleChanged(const QList<QContactLocalId>& contactIds)
{
    if (contactIds.count() > 0) {
        changedContactId = contactIds.at(0);
    }
}

void TestCntSymbianDatabase::handleRemoved(const QList<QContactLocalId>& contactIds)
{
    removedContactId.append(contactIds);
}

void TestCntSymbianDatabase::handleMyCardChanged(const QContactLocalId& /*oldId*/, const QContactLocalId& newId)
{
    ownCardId = newId;
}

void TestCntSymbianDatabase::handleRelationshipsAdded(QList<QContactLocalId> contactIds)
{
    if (contactIds.count() > 1) {
        addedRelId1 = contactIds.at(0);
        addedRelId2 = contactIds.at(1);
    }
}

void TestCntSymbianDatabase::handleRelationshipsRemoved(QList<QContactLocalId> contactIds)
{
    if (contactIds.count() > 1) {
        removedRelId1 = contactIds.at(0);
        removedRelId2 = contactIds.at(1);
    }
    
}
