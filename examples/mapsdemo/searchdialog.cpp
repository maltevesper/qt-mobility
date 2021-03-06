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

#include "searchdialog.h"

#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>

SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent)
{
    QFormLayout *formLayout = new QFormLayout;
    QVBoxLayout *vbox = new QVBoxLayout;

    searchTermEdit = new QLineEdit;
    formLayout->addRow("Search for", searchTermEdit);

    whereCombo = new QComboBox;
    whereCombo->addItem(tr("Nearby (<10km)"), 10000);
    whereCombo->addItem(tr("Within 30 mins drive of me (<25km)"), 25000);
    whereCombo->addItem(tr("Within 100km of me"), 100000);
    whereCombo->addItem(tr("Anywhere in the world"), -1);
    whereCombo->setCurrentIndex(1);
    formLayout->addRow(tr("Where"), whereCombo);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                QDialogButtonBox::Cancel,
                                                Qt::Horizontal);
    connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bb, SIGNAL(rejected()), this, SLOT(reject()));

    vbox->addLayout(formLayout);
    vbox->addWidget(bb);
    setLayout(vbox);
    setWindowTitle("Search for location");
}

qreal SearchDialog::radius() const
{
    const int i = whereCombo->currentIndex();
    return whereCombo->itemData(i).toReal();
}

SearchDialog::~SearchDialog()
{
}

QString SearchDialog::searchTerms() const
{
    return searchTermEdit->text();
}
