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
#include "cnttransformsynctarget.h"
#include "cntmodelextuids.h"

QList<CContactItemField *> CntTransformSyncTarget::transformDetailL(const QContactDetail &detail)
{
    if(detail.definitionName() != QContactSyncTarget::DefinitionName)
       User::Leave(KErrArgument);

    QList<CContactItemField *> fieldList;

	//cast to sync target
	const QContactSyncTarget &syncTarget(static_cast<const QContactSyncTarget&>(detail));

    //create new fields without contexts
    transformToTextFieldL(syncTarget, fieldList, syncTarget.syncTarget(), KUidContactFieldClass, KUidContactFieldVCardMapClass, false);

	return fieldList;
}

QContactDetail *CntTransformSyncTarget::transformItemField(const CContactItemField& field, const QContact &contact)
{
	Q_UNUSED(contact);

	QContactSyncTarget *syncTarget = new QContactSyncTarget();

	CContactTextField* storage = field.TextStorage();
	QString syncTargetString = QString::fromUtf16(storage->Text().Ptr(), storage->Text().Length());

	syncTarget->setSyncTarget(syncTargetString);
	return syncTarget;
}

bool CntTransformSyncTarget::supportsDetail(QString detailName) const
{
    bool ret = false;
    if (detailName == QContactSyncTarget::DefinitionName) {
        ret = true;
    }
    return ret;
}

QList<TUid> CntTransformSyncTarget::supportedFields() const
{
    return QList<TUid>()
        << KUidContactFieldClass;
}

QList<TUid> CntTransformSyncTarget::supportedSortingFieldTypes(QString detailFieldName) const
{
    QList<TUid> uids;
    if (detailFieldName == QContactSyncTarget::FieldSyncTarget)
        uids << KUidContactFieldClass;
    return uids;
}


/*!
 * Checks whether the subtype is supported
 *
 * \a subType The subtype to be checked
 * \return True if this subtype is supported
 */
bool CntTransformSyncTarget::supportsSubType(const QString& subType) const
{
    Q_UNUSED(subType);
    return false;
}

/*!
 * Returns the filed id corresponding to a field
 *
 * \a fieldName The name of the supported field
 * \return fieldId for the fieldName, 0  if not supported
 */
quint32 CntTransformSyncTarget::getIdForField(const QString& fieldName) const
{
    if (QContactSyncTarget::FieldSyncTarget == fieldName)
        return KUidContactFieldClass.iUid;
    else
        return 0;
}

/*!
 * Modifies the detail definitions. The default detail definitions are
 * queried from QContactManagerEngine::schemaDefinitions and then modified
 * with this function in the transform leaf classes.
 *
 * \a definitions The detail definitions to modify.
 * \a contactType The contact type the definitions apply for.
 */
void CntTransformSyncTarget::detailDefinitions(QMap<QString, QContactDetailDefinition> &definitions, const QString& contactType) const
{
    Q_UNUSED(contactType);

    if(definitions.contains(QContactSyncTarget::DefinitionName)) {
        QContactDetailDefinition d = definitions.value(QContactSyncTarget::DefinitionName);
        QMap<QString, QContactDetailFieldDefinition> fields = d.fields();
        QContactDetailFieldDefinition f;

        // Not all fields are supported, replace:
        fields.clear();
        f.setDataType(QVariant::String);
        f.setAllowableValues(QVariantList());
        fields.insert(QContactSyncTarget::FieldSyncTarget, f);

        d.setFields(fields);
        d.setUnique(true);

        // Replace original definitions
        definitions.insert(d.name(), d);
    }
}
