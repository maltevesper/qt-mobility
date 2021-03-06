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

#ifndef S60RADIOTUNERCONTROL_H
#define S60RADIOTUNERCONTROL_H

#include <qradiotunercontrol.h>
#include <qradiotuner.h>
#include <tuner.h>

class S60RadioTunerService;

QT_USE_NAMESPACE

class S60RadioTunerControl 
    : public QRadioTunerControl
    , public MMMTunerObserver
    , public MMMTunerStereoObserver
    , public MMMSignalStrengthObserver
    , public MMMTunerChangeObserver
    , public MMMTunerAudioPlayerObserver
{
    Q_OBJECT
public:
    S60RadioTunerControl(QObject *parent = 0);
    ~S60RadioTunerControl();
    
    QRadioTuner::State state() const;

    QRadioTuner::Band band() const;
    void setBand(QRadioTuner::Band b);
    bool isBandSupported(QRadioTuner::Band b) const;

    int frequency() const;
    int frequencyStep(QRadioTuner::Band b) const;
    QPair<int,int> frequencyRange(QRadioTuner::Band b) const;
    void setFrequency(int frequency);

    bool isStereo() const;
    QRadioTuner::StereoMode stereoMode() const;
    void setStereoMode(QRadioTuner::StereoMode mode);

    int signalStrength() const;
   
    int volume() const;
    void setVolume(int volume);

    bool isMuted() const;
    void setMuted(bool muted);

    bool isSearching() const;
    void searchForward();
    void searchBackward();
    void cancelSearch();

    bool isValid() const;

    bool isAvailable() const;
    QtMultimediaKit::AvailabilityError availabilityError() const;
    
    void start();
    void stop();

    QRadioTuner::Error error() const;
    QString errorString() const;
    
    // MMMTunerObserver
    void MToTuneComplete(TInt aError);
    
    // MMMTunerChangeObserver
    void MTcoFrequencyChanged(const TFrequency& aOldFrequency, const TFrequency& aNewFrequency);
    void MTcoStateChanged(const TUint32& aOldState, const TUint32& aNewState);
    void MTcoAntennaDetached();
    void MTcoAntennaAttached();
    void FlightModeChanged(TBool aFlightMode);
    
    // MMMTunerStereoObserver
    void MTsoStereoReceptionChanged(TBool aStereo);
    void MTsoForcedMonoChanged(TBool aForcedMono);
    
    // MMMSignalStrengthObserver
    void MssoSignalStrengthChanged(TInt aNewSignalStrength);
    
    // MMMTunerAudioPlayerObserver
    void MTapoInitializeComplete(TInt aError);
    void MTapoPlayEvent(TEventType aEvent, TInt aError, TAny* aAdditionalInfo);

private:
    bool initRadio();
    CMMTunerUtility::TTunerBand getNativeBand(QRadioTuner::Band b) const;

private:
    mutable int m_error;
    CMMTunerUtility *m_tunerUtility;
    CMMTunerAudioPlayerUtility *m_audioPlayerUtility;
    bool m_audioInitializationComplete;
    bool m_muted;
    bool m_isStereo;
    bool m_available;
    int  m_step;
    int  m_vol;
    mutable int m_signal;
    bool m_scanning;
    bool forward;
    QRadioTuner::Band m_currentBand;
    qint64 m_currentFreq;
    QRadioTuner::Error m_radioError;
    QRadioTuner::StereoMode m_stereoMode;
    QString m_errorString;
    TTunerCapabilities m_currentTunerCapabilities;
    long m_tunerState; 
    QRadioTuner::State m_apiTunerState;
};

#endif
