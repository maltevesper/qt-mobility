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

#include "ut_cntsymbianengine.h"
#include "cntsymbianengine.h"
#include "qcontactrelationship.h"
#include "qcontactrelationshipfilter.h"

#include <qcontactmanager.h>
#include <qcontactdetailfilter.h>
#include <qcontactorganization.h>

#include <QtTest/QtTest>

void TestSymbianEngine::initTestCase()
{
    QContactManager::Error error;
    QMap<QString, QString> emptyParameters;
    
    m_engine = new CntSymbianEngine(emptyParameters, error);
    if (error == QContactManager::NoError)
        removeAllContacts();
    else
        QSKIP("Error creating manager", SkipAll);
}

void TestSymbianEngine::cleanupTestCase()
{
    removeAllContacts();
    delete m_engine;
}

void TestSymbianEngine::init()
{
}

void TestSymbianEngine::cleanup()
{
    removeAllContacts();
}

void TestSymbianEngine::removeAllContacts()
{
    if(m_engine) {
        // Empty cnt database
        QContactManager::Error err;
        QList<QContactSortOrder> sortOrders;
        QList<QContactLocalId> cnts_ids = m_engine->contacts(sortOrders, err);
        QVERIFY(err == QContactManager::NoError);
        
        for(int i=0; i<cnts_ids.count(); i++) {
            QVERIFY(m_engine->removeContact(cnts_ids[i], err));
        }
    }
}

void TestSymbianEngine::ctors()
{
    QContactManager::Error error;
    QMap<QString, QString> params;
    
    // Ctor
    CntSymbianEngine *ce;
    ce = new CntSymbianEngine(params, error);
    QVERIFY(ce != NULL);
    QVERIFY(error == QContactManager::NoError);
    if (error == QContactManager::NoError) {
        QVERIFY(ce->managerName() == CNT_SYMBIAN_MANAGER_NAME);
        QVERIFY(ce->m_contactFilter != 0);
        QVERIFY(ce->m_contactSorter != 0);
        QVERIFY(ce->m_dataBase != 0);
        QVERIFY(ce->m_relationship != 0);
        QVERIFY(ce->m_transformContact != 0);
    } else {
        QSKIP("Error creating CntSymbianEngine in ctor", SkipSingle);
    }
    // copy ctor
    CntSymbianEngine *ce1(ce);
    QVERIFY(ce->managerName() == ce1->managerName());
    QVERIFY(ce->m_contactFilter == ce1->m_contactFilter);
    QVERIFY(ce->m_contactSorter == ce1->m_contactSorter);
    QVERIFY(ce->m_dataBase == ce1->m_dataBase);
    QVERIFY(ce->m_relationship == ce1->m_relationship);
    QVERIFY(ce->m_transformContact == ce1->m_transformContact);
    
    // assignment
    CntSymbianEngine *ce2 = ce;
    QVERIFY(ce->managerName() == ce2->managerName());
    QVERIFY(ce->m_contactFilter == ce2->m_contactFilter);
    QVERIFY(ce->m_contactSorter == ce2->m_contactSorter);
    QVERIFY(ce->m_dataBase == ce2->m_dataBase);
    QVERIFY(ce->m_relationship == ce2->m_relationship);
    QVERIFY(ce->m_transformContact == ce2->m_transformContact);

    // dref
    ce->deref();
    /*
    QVERIFY(ce->m_contactFilter == 0xDEDEDEDE);
    QVERIFY(ce->m_contactSorter == 0xDEDEDEDE);
    QVERIFY(ce->m_dataBase == 0xDEDEDEDE);
    QVERIFY(ce->m_relationship == 0xDEDEDEDE);
    QVERIFY(ce->m_transformContact == 0xDEDEDEDE);
    */
}

void TestSymbianEngine::saveContact()
{
    QContactManager::Error err;
    QList<QContactSortOrder> sortOrders;
    QContactId empty;

    int init_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    
    // Save a "NULL" contact
    QVERIFY(!m_engine->saveContact(NULL, err));
    QVERIFY(err == QContactManager::BadArgumentError);
    int current_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(init_count == current_count);
    
    // Save a contact that is not in database
    QContact invaId;
    QVERIFY(m_engine->saveContact(&invaId, err));   // Add to db
    QVERIFY(err == QContactManager::NoError);
    QContactId cId = invaId.id();
    m_engine->removeContact(invaId.localId(), err);   // Ensure not in db
    QVERIFY(err == QContactManager::NoError);
    invaId.setId(cId);
    QVERIFY(!m_engine->saveContact(&invaId, err));   // Update non existent contact
    QVERIFY(err == QContactManager::DoesNotExistError);
    current_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(init_count == current_count);
    
    QContact alice;
    alice.setType("Jargon");
    
    // Save a "non contact(Jargon) type" contact
    QVERIFY(!m_engine->saveContact(&alice, err));
    QVERIFY(err == QContactManager::InvalidDetailError);
    QVERIFY(alice.id() == empty);
    QVERIFY(alice.localId() == 0);
    current_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(init_count == current_count);
    
    // Save a valid contact
    alice.setType(QContactType::TypeContact);
    QVERIFY(m_engine->saveContact(&alice, err));
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(alice.id() != empty);
    QVERIFY(alice.localId() != 0);
    QString uri = QString(QLatin1String(CNT_SYMBIAN_MANAGER_NAME));
    QVERIFY(alice.id().managerUri().contains(uri, Qt::CaseInsensitive));
    current_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(init_count + 1 == current_count);
}

void TestSymbianEngine::saveContacts()
{    
    QContactManager::Error err;
    QList<QContactSortOrder> sortOrders;
    QList<QContact> contacts;
    QContactId empty;
    int count = 5;
    
    int init_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);

    // NULL
    QList<QContactManager::Error> errors = m_engine->saveContacts(NULL, err);    
    QVERIFY(errors.count() == 0);
    QVERIFY(err == QContactManager::BadArgumentError);
    int current_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(init_count == current_count);
    
    // Save a "non contact(Jargon) type" contacts
    for(int i=0; i<count; i++) {
        QContact alice;
        alice.setType("Jargon");
        contacts.append(alice);
    }
    errors = m_engine->saveContacts(&contacts, err);
    QVERIFY(err == QContactManager::InvalidDetailError);
    foreach(QContactManager::Error err, errors) {
        QVERIFY(err == QContactManager::InvalidDetailError);
    }
    foreach(QContact c, contacts) {
        QVERIFY(c.id() == empty);
        QVERIFY(c.localId() == 0);
    }
    current_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(init_count == current_count);
    contacts.clear();
    
    // Save valid contacts
    for(int i=0; i<count; i++) {
        QContact alice;
        alice.setType(QContactType::TypeContact);
        contacts.append(alice);
    }
    errors = m_engine->saveContacts(&contacts, err);
    QVERIFY(err == QContactManager::NoError);
    foreach(QContactManager::Error err, errors) {
        QVERIFY(err == QContactManager::NoError);
    }
    QString uri = QString(QLatin1String(CNT_SYMBIAN_MANAGER_NAME));
    foreach(QContact c, contacts) {
        QVERIFY(c.id() != empty);
        QVERIFY(c.localId() != 0);
        QVERIFY(c.id().managerUri().contains(uri, Qt::CaseInsensitive));
    }
    current_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(init_count + count == current_count);
    contacts.clear();
    
    // Save with one invalid contact in list
    init_count = m_engine->contacts(sortOrders, err).count();
    for(int i=0; i<count; i++) {
        QContact alice;
        alice.setType(QContactType::TypeContact);
        contacts.append(alice);
    }
    QContact invalid;
    invalid.setType("Jasdfasd");
    contacts.insert(3, invalid);
    
    errors = m_engine->saveContacts(&contacts, err);
    QVERIFY(err == QContactManager::InvalidDetailError);
    for(int i=0; i<errors.count(); i++) {
        if (i == 3)
            QVERIFY(errors[i] == QContactManager::InvalidDetailError);
        else
            QVERIFY(errors[i] == QContactManager::NoError);
    }
    
    for(int i=0; i<contacts.count(); i++) {
        QContact c = contacts[i];
        if (i == 3) {
            QVERIFY(c.id() == empty);
            QVERIFY(c.localId() == 0);
        } else {
            QVERIFY(c.id() != empty);
            QVERIFY(c.localId() != 0);
            QVERIFY(c.id().managerUri().contains(uri, Qt::CaseInsensitive));
        }
    }
    current_count = m_engine->contacts(sortOrders, err).count();
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(init_count + count == current_count);
    contacts.clear();    
}

void TestSymbianEngine::retrieveContact()
{
    QContactManager::Error err;
    
    QContact alice;
    alice.setType(QContactType::TypeContact);
    QVERIFY(m_engine->saveContact(&alice, err));
    QVERIFY(err == QContactManager::NoError);
    
    // Retrieve "non contact"
    QContact c = m_engine->contact(0, err);
    QVERIFY(&c != NULL);
    QVERIFY(c.localId() == 0);
    QVERIFY(err == QContactManager::DoesNotExistError);
    
    // Retrieve valid existing contact
    QContactLocalId aid = alice.localId();
    c = m_engine->contact(aid, err);
    QVERIFY(&c != NULL);
    QVERIFY(c.localId() == aid);
    QVERIFY(err == QContactManager::NoError);
}

void TestSymbianEngine::retrieveContacts()
{
    QContactManager::Error err;
    QContactFilter f;
    QList<QContactSortOrder> s;
    QList<QContactLocalId> cnt_ids;
    
    QContact c;
    c.setType(QContactType::TypeContact);
    QContactName cn;
    cn.setFirst("aaa");
    QVERIFY(c.saveDetail(&cn));
    
    QContactPhoneNumber number;
    number.setContexts("Home");
    number.setSubTypes("Mobile");
    number.setNumber("12345678");
    QVERIFY(c.saveDetail(&number));
    QVERIFY(m_engine->saveContact(&c, err));
    QVERIFY(err == QContactManager::NoError);
    
    QContact d;
    d.setType(QContactType::TypeContact);
    QContactName dn;
    dn.setFirst("bbb");
    QVERIFY(d.saveDetail(&dn));
    QVERIFY(m_engine->saveContact(&d, err));
    QVERIFY(err == QContactManager::NoError);
    
    QContact e;
    e.setType(QContactType::TypeGroup);
    QContactName en;
    en.setFirst("ccc");
    QVERIFY(e.saveDetail(&en));
    QVERIFY(m_engine->saveContact(&e, err));
    QVERIFY(err == QContactManager::NoError);
    
    // Retrieve all contacts
    cnt_ids = m_engine->contacts(f, s, err);
    QVERIFY(err == QContactManager::NoError);
    
    QContactDetailFilter mobileFilter;
    mobileFilter.setDetailDefinitionName(QContactPhoneNumber::DefinitionName, QContactPhoneNumber::FieldSubTypes);
    mobileFilter.setValue(QLatin1String(QContactPhoneNumber::SubTypeMobile));
    
    // Retrieve contacts with mobile number
    cnt_ids = m_engine->contacts(mobileFilter, s, err);
    QVERIFY(err == QContactManager::NoError);

    QContactDetailFilter invalidFilter;
    mobileFilter.setDetailDefinitionName("asfdasdf", "asdfasdf");
    
    // Retrieve contacts with invalid filter
    cnt_ids = m_engine->contacts(invalidFilter, s, err);
    QVERIFY(err == QContactManager::NoError);
    
    // Retrieve sorted contacts
    QContactSortOrder sortOrder;
    QList<QContactSortOrder> s1;
    sortOrder.setDetailDefinitionName(QContactName::DefinitionName,  QContactName::FieldFirst);
    sortOrder.setBlankPolicy(QContactSortOrder::BlanksLast);
    sortOrder.setDirection(Qt::AscendingOrder);
    sortOrder.setCaseSensitivity(Qt::CaseInsensitive);
    s1.append(sortOrder);
    
    cnt_ids = m_engine->contacts(s1, err);
    QVERIFY(err == QContactManager::NoError);
    
    // Retrieve with invalid sort order
    QContactSortOrder sortOrder1;
    QList<QContactSortOrder> s2;
    sortOrder1.setDetailDefinitionName("asdfasdf", "asdfasd");
    
    cnt_ids = m_engine->contacts(s2, err);
    QVERIFY(err == QContactManager::NoError);
}

void TestSymbianEngine::updateContact()
{
    QContactManager::Error err;
    QContact c;
    
    QVERIFY(m_engine->saveContact(&c, err));
    QVERIFY(err == QContactManager::NoError);
    
    int details_before = c.details().count();

    QContactName aliceName;
    aliceName.setFirst("Alice");
    c.saveDetail(&aliceName);
    
    QContactPhoneNumber number;
    number.setContexts("Home");
    number.setSubTypes("Mobile");
    number.setNumber("12345678");
    c.saveDetail(&number);

    // update the contact
    QContactLocalId id = c.localId();
    QVERIFY(m_engine->saveContact(&c, err));
    QVERIFY(err == QContactManager::NoError);

    // Verify that contact has been updated
    QContact d = m_engine->contact(id, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(d.localId() == id);
    QVERIFY(d.details().count() > details_before);
    QString str = d.detail(QContactPhoneNumber::DefinitionName).definitionName();
    QVERIFY(str == QContactPhoneNumber::DefinitionName);
}

void TestSymbianEngine::removeContact()
{
    QContactManager::Error err;
    
    QContact c;
    c.setType(QContactType::TypeContact);
    QVERIFY(m_engine->saveContact(&c, err));
    QVERIFY(err == QContactManager::NoError);

    // Remove existing contact
    QContactLocalId id = c.localId();
    QVERIFY(m_engine->removeContact(id, err));
    QVERIFY(err == QContactManager::NoError);

    // Verify that contact has been removed
    QContact f = m_engine->contact(id, err);
    QVERIFY(f.localId() == 0);
    QVERIFY(err == QContactManager::DoesNotExistError);

    // Remove non existent contact
    QVERIFY(!m_engine->removeContact(0, err));
    QVERIFY(err == QContactManager::DoesNotExistError);
}

void TestSymbianEngine::removeContacts()
{
    QContactManager::Error err;
    QList<QContactLocalId> contacts;
    int count = 5;
        
    // Remove non existent contacts
    QList<QContactManager::Error> errors = m_engine->removeContacts(&contacts, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(errors.count() == 0);
        
    // NULL argument
    errors = m_engine->removeContacts(NULL, err);
    QVERIFY(errors.count() == 0);
    QVERIFY(err == QContactManager::BadArgumentError);
    
    // Remove existing contacts
    for(int i=0; i<count; i++) {
        QContact c;
        c.setType(QContactType::TypeContact);
        QVERIFY(m_engine->saveContact(&c, err));
        QVERIFY(err == QContactManager::NoError);
        contacts.append(c.localId());
    }
    errors = m_engine->removeContacts(&contacts, err);
    QVERIFY(err == QContactManager::NoError);
    foreach(QContactManager::Error e, errors) {
        QVERIFY(e == QContactManager::NoError);
    }
    
    // Verify that contacts have been removed
    for(int i=0; i<contacts.count(); i++) {
        QContact f = m_engine->contact(contacts[i], err);
        QVERIFY(f.localId() == 0);
        QVERIFY(err == QContactManager::DoesNotExistError);
    }
    
    // Remove a list with one non existent contact
    contacts.clear();
    for(int i=0; i<count; i++) {
        QContact c;
        c.setType(QContactType::TypeContact);
        QVERIFY(m_engine->saveContact(&c, err));
        QVERIFY(err == QContactManager::NoError);
        contacts.append(c.localId());
    }
    contacts.insert(3, 0);
    
    errors = m_engine->removeContacts(&contacts, err);
    QVERIFY(err == QContactManager::DoesNotExistError);
    for(int i=0; i<errors.count(); i++) {
        if (i == 3)
            QVERIFY(errors[i] == QContactManager::DoesNotExistError);
        else
            QVERIFY(errors[i] == QContactManager::NoError);
    }
    
    for(int i=0; i<contacts.count(); i++) {
        QContact f = m_engine->contact(contacts[i], err);
        QVERIFY(f.localId() == 0);
        QVERIFY(err == QContactManager::DoesNotExistError);
    }
}

void TestSymbianEngine::addOwnCard()
{
    QContactManager::Error err;
    
    // Create a new contact own card
    QContact own;
    QContactName ownName;
    ownName.setFirst("Own");
    own.saveDetail(&ownName);
    QVERIFY(m_engine->saveContact(&own, err));
    QVERIFY(err == QContactManager::NoError);

    // Set a non existent contact as own card and verify
    // ensure this contact does not exist in dbase
    QContactLocalId id(12);
    m_engine->removeContact(id, err);   // Dont test err. May or may not be in dbase
    QVERIFY(!m_engine->setSelfContactId(id, err)); // does not exist
    QVERIFY(err == QContactManager::DoesNotExistError);

    // Test a "0" contact id
    QVERIFY(!m_engine->setSelfContactId(0, err)); // Bad argument
    QVERIFY(err == QContactManager::BadArgumentError);

    // Set an existent contact as own card
    QVERIFY(m_engine->setSelfContactId(own.localId(), err));
    QVERIFY(err == QContactManager::NoError);
}

void TestSymbianEngine::retrieveOwnCard()
{
    QContactManager::Error err;

    // Create a new contact own card
    QContact own;
    QContactName ownName;
    ownName.setFirst("Own");
    own.saveDetail(&ownName);
    QVERIFY(m_engine->saveContact(&own, err));
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(m_engine->setSelfContactId(own.localId(), err));
    QVERIFY(err == QContactManager::NoError);
    
    // Fetch existing self contact
    QContactLocalId own_id = m_engine->selfContactId(err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(own_id == own.localId());

    // Remove self contact and verify
    QVERIFY(m_engine->removeContact(own.localId(), err));
    QContactLocalId idr = m_engine->selfContactId(err);
    QVERIFY(err == QContactManager::DoesNotExistError);
    QVERIFY(idr == 0);
}

void TestSymbianEngine::filterSupport()
{
    // Filter feature support
    QContactDetailFilter df;
    df.setDetailDefinitionName(QContactPhoneNumber::DefinitionName, QContactPhoneNumber::FieldSubTypes);
    QVERIFY(m_engine->filterSupported(df));
    /*  To move to filter test cases
    QContactActionFilter af;
    QVERIFY(!m_engine->filterSupported(af));
    QContactChangeLogFilter clf;
    QVERIFY(!m_engine->filterSupported(clf));
    QContactDetailRangeFilter drf;
    QVERIFY(!m_engine->filterSupported(drf));
    QContactIntersectionFilter isf;
    QVERIFY(!m_engine->filterSupported(isf));
    QContactInvalidFilter invf;
    QVERIFY(!m_engine->filterSupported(invf));
    QContactLocalIdFilter liff;
    QVERIFY(!m_engine->filterSupported(liff));
    QContactRelationshipFilter rf;
    QVERIFY(!m_engine->filterSupported(rf));
    QContactUnionFilter uf;
    QVERIFY(!m_engine->filterSupported(uf));
    */    
    QContactFilter f;
    QVERIFY(!m_engine->filterSupported(f));
}

void TestSymbianEngine::featureSupport()
{
    //hasFeature(QContactManager::ManagerFeature feature, const QString& contactType)
    QContactManager::ManagerFeature f;
 
    // NULL parameters
    QVERIFY(!m_engine->hasFeature(f, ""));
    
    // empty feature param
    QVERIFY(!m_engine->hasFeature(f, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(f, QContactType::TypeGroup));
    
    // Non existent feature
    f = static_cast<QContactManager::ManagerFeature>(198);
    QVERIFY(!m_engine->hasFeature(f, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(f, QContactType::TypeGroup));
    
    // empty contact type param
    QVERIFY(!m_engine->hasFeature(QContactManager::Groups, ""));
    QVERIFY(!m_engine->hasFeature(QContactManager::ActionPreferences, ""));
    QVERIFY(!m_engine->hasFeature(QContactManager::MutableDefinitions, ""));
    QVERIFY(!m_engine->hasFeature(QContactManager::Relationships, ""));
    QVERIFY(!m_engine->hasFeature(QContactManager::ArbitraryRelationshipTypes, ""));
    QVERIFY(!m_engine->hasFeature(QContactManager::SelfContact, ""));
    QVERIFY(!m_engine->hasFeature(QContactManager::Anonymous, ""));
    QVERIFY(!m_engine->hasFeature(QContactManager::ChangeLogs, ""));
    
    // wrong contact type param
    QVERIFY(!m_engine->hasFeature(QContactManager::Groups, "aserasdf"));
    QVERIFY(!m_engine->hasFeature(QContactManager::ActionPreferences, "aserasdf"));
    QVERIFY(!m_engine->hasFeature(QContactManager::MutableDefinitions, "aserasdf"));
    QVERIFY(!m_engine->hasFeature(QContactManager::Relationships, "aserasdf"));
    QVERIFY(!m_engine->hasFeature(QContactManager::ArbitraryRelationshipTypes, "aserasdf"));
    QVERIFY(!m_engine->hasFeature(QContactManager::SelfContact, "aserasdf"));
    QVERIFY(!m_engine->hasFeature(QContactManager::Anonymous, "aserasdf"));
    QVERIFY(!m_engine->hasFeature(QContactManager::ChangeLogs, "aserasdf"));
    
    // TypeContact contact type param
    QVERIFY(m_engine->hasFeature(QContactManager::Groups, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::ActionPreferences, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::MutableDefinitions, QContactType::TypeContact));
    QVERIFY(m_engine->hasFeature(QContactManager::Relationships, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::ArbitraryRelationshipTypes, QContactType::TypeContact));
    QVERIFY(m_engine->hasFeature(QContactManager::SelfContact, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::Anonymous, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::ChangeLogs, QContactType::TypeContact));
    
    // TypeGroup contact type param
    QVERIFY(m_engine->hasFeature(QContactManager::Groups, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::ActionPreferences, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::MutableDefinitions, QContactType::TypeContact));
    QVERIFY(m_engine->hasFeature(QContactManager::Relationships, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::ArbitraryRelationshipTypes, QContactType::TypeContact));
    QVERIFY(m_engine->hasFeature(QContactManager::SelfContact, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::Anonymous, QContactType::TypeContact));
    QVERIFY(!m_engine->hasFeature(QContactManager::ChangeLogs, QContactType::TypeContact));    
}

void TestSymbianEngine::addGroup()
{
    QContactManager::Error err = QContactManager::NoError;
    QContactId empty;

    // Add valid group contact
    QContact g;
    g.setType(QContactType::TypeGroup);

    QVERIFY(m_engine->saveContact(&g, err));
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(g.id() != empty);
    QVERIFY(g.localId() != 0);
    QString uri = QString(QLatin1String(CNT_SYMBIAN_MANAGER_NAME));
    QVERIFY(g.id().managerUri().contains(uri, Qt::CaseInsensitive));
    
    // Add invalid group contact
    QContact g1;
    g1.setType("Jargon");
            
    QVERIFY(!m_engine->saveContact(&g1, err));
    QVERIFY(err == QContactManager::InvalidDetailError);
    QVERIFY(g1.id() == empty);
    QVERIFY(g1.localId() == 0);
}

void TestSymbianEngine::retrieveGroup()
{
    QContactManager::Error err;
    QList<QContactSortOrder> s;
    
    // retrieve group contacts
    QList<QContactLocalId> grp_ids = m_engine->contacts(QContactType::TypeGroup, s, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(err == QContactManager::NoError);
    
    QContact g;
    g.setType(QContactType::TypeGroup);
    QVERIFY(m_engine->saveContact(&g, err));
    QVERIFY(err == QContactManager::NoError);
    
    QList<QContactLocalId> grp_ids1 = m_engine->contacts(QContactType::TypeGroup, s, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(grp_ids.count() + 1 == grp_ids1.count());
    
}

void TestSymbianEngine::singleRelationship()
{
    // More relationships logic is tested in relationship unit tests.
    // We do simple tests here
    QContactManager::Error error;
    
    QContact a;
    QVERIFY(m_engine->saveContact(&a, error));
    QContact b;
    QVERIFY(m_engine->saveContact(&b, error));
    
    QContactRelationship rel;
    rel.setFirst(a.id());
    rel.setSecond(b.id());
    rel.setRelationshipType(QContactRelationship::HasSpouse);
    
    // Add relationship
    m_engine->saveRelationship(&rel, error);
    bool isValid(false);
    if (error == QContactManager::NoError ||
        error == QContactManager::NotSupportedError)
        isValid = true;
    
    QVERIFY(isValid);
    
    // Remove relationship
    m_engine->removeRelationship(rel, error);
    if (error == QContactManager::NoError ||
        error == QContactManager::NotSupportedError)
        isValid = true;
    else
        isValid = false;
    QVERIFY(isValid);
}

void TestSymbianEngine::batchRelationships()
{
    // More relationships logic is tested somewhere else.
    // We do simple tests here
    QContactManager::Error error;
    
    QContact a;
    QVERIFY(m_engine->saveContact(&a, error));
    QContact b;
    QVERIFY(m_engine->saveContact(&b, error));
    
    QContactRelationship rel;
    rel.setFirst(a.id());
    rel.setSecond(b.id());
    rel.setRelationshipType(QContactRelationship::HasSpouse);
    
    QList<QContactRelationship> list;
    list.append(rel);
    bool isValid(false);
    
    // Add relationships
    QList<QContactManager::Error> errors = m_engine->saveRelationships(&list, error);
    QVERIFY(&errors != NULL);
    foreach(QContactManager::Error err, errors) {
        if (err == QContactManager::NoError ||
                err == QContactManager::NotSupportedError)
            isValid = true;
        else
            isValid = false;
        QVERIFY(isValid);
    }
    
    // fetch relationships
    QContactRelationshipFilter::Role role;
    role = QContactRelationshipFilter::First;
    list.clear();
    list = m_engine->relationships(QContactRelationship::HasSpouse, a.id(), role, error);
    QVERIFY(&list != NULL);
    if (error == QContactManager::NoError ||
        error == QContactManager::NotSupportedError)
        isValid = true;
    else
        isValid = false;
    QVERIFY(isValid);
    
    // Remove relationships
    errors = m_engine->removeRelationships(list, error);
    QVERIFY(&errors != NULL);
    foreach(QContactManager::Error err, errors) {
        if (err == QContactManager::NoError ||
                err == QContactManager::NotSupportedError)
            isValid = true;
        else
            isValid = false;
        QVERIFY(isValid);
    }
}

void TestSymbianEngine::dataTypeSupport()
{
    QList<QVariant::Type> support = m_engine->supportedDataTypes();
    QVERIFY(support.contains(QVariant::String));
}

void TestSymbianEngine::synthesizeDisplaylable()
{
    QContactManager::Error err = QContactManager::NoError;
    
    QContact empty;
    QString label = m_engine->synthesizeDisplayLabel(empty, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(label == QString("Unnamed"));
    
    QContact first;
    QContactName fn;
    fn.setFirst("Alice");
    first.saveDetail(&fn);
    label = m_engine->synthesizeDisplayLabel(first, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(label == QString("Alice"));
    
    QContact last;
    QContactName ln;
    ln.setLast("Jones");
    last.saveDetail(&ln);
    label = m_engine->synthesizeDisplayLabel(last, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(label == QString("Jones"));
    
    QContact orgContact;
    QContactOrganization org;
    org.setName("Nokia");
    org.setDepartment(QStringList("Services"));
    org.setTitle("Assistant Manager");
    org.setLocation("Nokia Cyber Park");
    orgContact.saveDetail(&org);
    label = m_engine->synthesizeDisplayLabel(orgContact, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(label == QString("Nokia"));
    
    QContact jargon;
    jargon.setType("jargon");
    label = m_engine->synthesizeDisplayLabel(jargon, err);
    QVERIFY(err == QContactManager::InvalidContactTypeError);
    QVERIFY(label.isEmpty());
    
    QContact group;
    group.setType(QContactType::TypeGroup);
    QContactName gn;
    gn.setCustomLabel("grouplable");
    group.saveDetail(&gn);
    label = m_engine->synthesizeDisplayLabel(group, err);
    QVERIFY(err == QContactManager::NoError);
    QVERIFY(label == QString("grouplable"));
}

void TestSymbianEngine::definitionDetails()
{
    QMap<QString, QContactDetailDefinition> defs;
    QContactManager::Error err;
    
    // Wrong contact type
    defs = m_engine->detailDefinitions("aerafa", err);
    QVERIFY(err = QContactManager::InvalidContactTypeError);
    QVERIFY(defs.count() == 0);
    
    // Valid defs
    defs = m_engine->detailDefinitions(QContactType::TypeContact, err);
    QVERIFY(err == QContactManager::NoError);
    defs = m_engine->detailDefinitions(QContactType::TypeGroup, err);
    QVERIFY(err == QContactManager::NoError);
}

QTEST_MAIN(TestSymbianEngine);