/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
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

#ifndef MOCKAUDIOENDPOINTSELECTOR_H
#define MOCKAUDIOENDPOINTSELECTOR_H

#include "qaudioendpointselector.h"

class MockAudioEndpointSelector : public QAudioEndpointSelector
{
    Q_OBJECT
public:
    MockAudioEndpointSelector(QObject *parent):
        QAudioEndpointSelector(parent)
    {
        m_names << "device1" << "device2" << "device3";
        m_descriptions << "dev1 comment" << "dev2 comment" << "dev3 comment";
        m_audioInput = "device1";
        emit availableEndpointsChanged();
    }
    ~MockAudioEndpointSelector() {}

    QList<QString> availableEndpoints() const
    {
        return m_names;
    }

    QString endpointDescription(const QString& name) const
    {
        QString desc;

        for (int i = 0; i < m_names.count(); i++) {
            if (m_names.at(i).compare(name) == 0) {
                desc = m_descriptions.at(i);
                break;
            }
        }
        return desc;
    }

    QString defaultEndpoint() const
    {
        return m_names.at(0);
    }

    QString activeEndpoint() const
    {
        return m_audioInput;
    }

public Q_SLOTS:

    void setActiveEndpoint(const QString& name)
    {
        m_audioInput = name;
        emit activeEndpointChanged(name);
    }

    void addEndpoints()
    {
        m_names << "device4";
        emit availableEndpointsChanged();
    }

    void removeEndpoints()
    {
        m_names.clear();
        emit availableEndpointsChanged();
    }

private:
    QString         m_audioInput;
    QList<QString>  m_names;
    QList<QString>  m_descriptions;
};



#endif // MOCKAUDIOENDPOINTSELECTOR_H
