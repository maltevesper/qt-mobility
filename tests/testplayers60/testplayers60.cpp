/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include "testplayers60.h"

#include "playercontrols.h"
#include "playlistmodel.h"
#include "videowidget.h"

#include <qmediaservice.h>
#include <qmediaplaylist.h>
#include <qaudioendpointselector.h>
#include <QMediaStreamsControl.h>

#include <QtGui>

#ifdef Q_OS_SYMBIAN
#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtNetwork/QHttp>
#include <QDomDocument>

#include "mediakeysobserver.h"
#endif

TestPlayers60::TestPlayers60(QWidget *parent)
    : QWidget(parent)
    , videoWidget(0)
    , coverLabel(0)
    , slider(0)
    , audioEndpointSelector(0)
    , streamControl(0)
#ifdef Q_OS_SYMBIAN
    , mediaKeysObserver(0)
    , playlistDialog(0)
    , toggleAspectRatio(0)
    , showYoutubeDialog(0)
    , youtubeDialog(0)
#else
    , colorDialog(0)
#endif
{
//! [create-objs]
    player = new QMediaPlayer(this);
    // owned by PlaylistModel
    playlist = new QMediaPlaylist();
    player->setPlaylist(playlist);
//! [create-objs]

    connect(player, SIGNAL(durationChanged(qint64)), SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), SLOT(positionChanged(qint64)));
    connect(player, SIGNAL(metaDataChanged()), SLOT(metaDataChanged()));
    connect(playlist, SIGNAL(currentIndexChanged(int)), SLOT(playlistPositionChanged(int)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(player, SIGNAL(bufferStatusChanged(int)), this, SLOT(bufferingProgress(int)));
    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(displayErrorMessage()));

//! [2]
    videoWidget = new VideoWidget(this);
    player->setVideoOutput(videoWidget);

    playlistModel = new PlaylistModel(this);
    playlistModel->setPlaylist(playlist);
//! [2]

    playlistView = new QListView(this);
    playlistView->setModel(playlistModel);
    playlistView->setCurrentIndex(playlistModel->index(playlist->currentIndex(), 0));

    connect(playlistView, SIGNAL(activated(QModelIndex)), this, SLOT(jump(QModelIndex)));

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, player->duration() / 1000);

    connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(seek(int)));

    QMediaService *service = player->service();
    if (service) {
        QMediaControl *aEcontrol = service->requestControl(QAudioEndpointSelector_iid);
        if (aEcontrol) {
            audioEndpointSelector = qobject_cast<QAudioEndpointSelector*>(aEcontrol);
            if (audioEndpointSelector) {
                connect(audioEndpointSelector, SIGNAL(activeEndpointChanged(const QString&)),
                        this, SLOT(handleAudioOutputChangedSignal(const QString&)));
            } else {
                service->releaseControl(aEcontrol);
            }
        }
    }

    if (service) {
        QMediaControl *MScontrol = service->requestControl(QMediaStreamsControl_iid);
        if (MScontrol) {
            streamControl = qobject_cast<QMediaStreamsControl*>(MScontrol);
            if (streamControl) {
                connect(streamControl, SIGNAL(streamsChanged()),
                        this, SLOT(handleMediaStreamChanged()));
            } else {
                service->releaseControl(MScontrol);
            }
        }
    }

#ifndef Q_OS_SYMBIAN
    QPushButton *openButton = new QPushButton(tr("Open"), this);

    connect(openButton, SIGNAL(clicked()), this, SLOT(open()));
#endif

    PlayerControls *controls = new PlayerControls(this);
    controls->setState(player->state());
    controls->setVolume(player->volume());
    controls->setMuted(controls->isMuted());

    connect(controls, SIGNAL(play()), player, SLOT(play()));
    connect(controls, SIGNAL(pause()), player, SLOT(pause()));
    connect(controls, SIGNAL(stop()), player, SLOT(stop()));
    connect(controls, SIGNAL(next()), playlist, SLOT(next()));
    connect(controls, SIGNAL(previous()), this, SLOT(previousClicked()));
    connect(controls, SIGNAL(changeVolume(int)), player, SLOT(setVolume(int)));
    connect(controls, SIGNAL(changeMuting(bool)), player, SLOT(setMuted(bool)));
    connect(controls, SIGNAL(changeRate(qreal)), player, SLOT(setPlaybackRate(qreal)));

    connect(controls, SIGNAL(stop()), videoWidget, SLOT(update()));

    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)),
            controls, SLOT(setState(QMediaPlayer::State)));
    connect(player, SIGNAL(volumeChanged(int)), controls, SLOT(setVolume(int)));
    connect(player, SIGNAL(mutedChanged(bool)), controls, SLOT(setMuted(bool)));

#ifndef Q_OS_SYMBIAN
    QPushButton *fullScreenButton = new QPushButton(tr("FullScreen"), this);
    fullScreenButton->setCheckable(true);

    if (videoWidget != 0) {
        connect(fullScreenButton, SIGNAL(clicked(bool)), videoWidget, SLOT(setFullScreen(bool)));
        connect(videoWidget, SIGNAL(fullScreenChanged(bool)),
                fullScreenButton, SLOT(setChecked(bool)));
    } else {
        fullScreenButton->setEnabled(false);
    }

    QPushButton *colorButton = new QPushButton(tr("Color Options..."), this);
    if (videoWidget)
        connect(colorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));
    else
        colorButton->setEnabled(false);

#endif

#ifdef Q_OS_SYMBIAN
    // Set some sensible default volume.
    player->setVolume(50);

    QLabel *label = new QLabel(tr("Playlist"), this);
    QVBoxLayout *playlistDialogLayout = new QVBoxLayout;
    playlistDialogLayout->addWidget(label);
    playlistDialogLayout->addWidget(playlistView);
    playlistDialog = new QDialog(this);
    playlistDialog->setWindowTitle(tr("Playlist"));
    playlistDialog->setLayout(playlistDialogLayout);
    playlistDialog->setContextMenuPolicy(Qt::NoContextMenu);

    QAction *close = new QAction(tr("Close"), this);
    close->setSoftKeyRole(QAction::NegativeSoftKey);
    playlistDialog->addAction(close);

    mediaKeysObserver = new MediaKeysObserver(this);

    coverLabel = new QLabel(this);
    coverLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    coverLabel->setMinimumSize(1, 1);
    coverLabel->hide();

    slider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    slider->setMinimumSize(1, 1);

    connect(controls, SIGNAL(open()), this, SLOT(open()));
    connect(controls, SIGNAL(fullScreen(bool)), this, SLOT(handleFullScreen(bool)));
    connect(controls, SIGNAL(openPlayList()), this, SLOT(showPlayList()));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(handleStateChange(QMediaPlayer::State)));
    connect(mediaKeysObserver, SIGNAL(mediaKeyPressed(MediaKeysObserver::MediaKeys)), this, SLOT(handleMediaKeyEvent(MediaKeysObserver::MediaKeys)));
    connect(close, SIGNAL(triggered()), playlistDialog, SLOT(reject()));

    QBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->addWidget(videoWidget, 7);
    layout->addWidget(coverLabel, 7);
    layout->addWidget(slider, 1);
    layout->addWidget(controls, 2);

    createMenus();
#else
    QBoxLayout *displayLayout = new QHBoxLayout;
    if (videoWidget)
        displayLayout->addWidget(videoWidget, 2);
    else
        displayLayout->addWidget(coverLabel, 2);
    displayLayout->addWidget(playlistView);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setMargin(0);
    controlLayout->addWidget(openButton);
    controlLayout->addStretch(1);
    controlLayout->addWidget(controls);
    controlLayout->addStretch(1);
    controlLayout->addWidget(fullScreenButton);
    controlLayout->addWidget(colorButton);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(displayLayout);
    layout->addWidget(slider);
    layout->addLayout(controlLayout);
#endif

    setLayout(layout);

    metaDataChanged();

    QStringList arguments = qApp->arguments();
    arguments.removeAt(0);
    foreach (QString const &argument, arguments) {
        QFileInfo fileInfo(argument);
        if (fileInfo.exists()) {
            QUrl url = QUrl::fromLocalFile(fileInfo.absoluteFilePath());
            if (fileInfo.suffix().toLower() == QLatin1String("m3u")) {
                playlist->load(url);
            } else
                playlist->addMedia(url);
        } else {
            QUrl url(argument);
            if (url.isValid()) {
                playlist->addMedia(url);
            }
        }
    }
}

TestPlayers60::~TestPlayers60()
{
}

void TestPlayers60::open()
{
#ifdef Q_WS_MAEMO_5
    QStringList fileNames;
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Files"), "/");
    if (!fileName.isEmpty())
        fileNames << fileName;
#else
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Files"));
#endif

    foreach (QString const &fileName, fileNames)
        playlist->addMedia(QUrl::fromLocalFile(fileName));
}

void TestPlayers60::durationChanged(qint64 duration)
{
    slider->setMaximum(duration / 1000);
}

void TestPlayers60::positionChanged(qint64 progress)
{
    if (!slider->isSliderDown()) {
        slider->setValue(progress / 1000);
    }
}

void TestPlayers60::metaDataChanged()
{
#ifdef Q_OS_SYMBIAN
    if (player->isMetaDataAvailable()) {
        setTrackInfo(QString("(%1/%2) %3 - %4")
                .arg(playlist->currentIndex()+1)
                .arg(playlist->mediaCount())
                .arg(player->metaData(QtMultimediaKit::AlbumArtist).toString())
                .arg(player->metaData(QtMultimediaKit::Title).toString()));

        if (!player->isVideoAvailable()) {
            QUrl uri = player->metaData(QtMultimediaKit::CoverArtUrlLarge).value<QUrl>();
            QPixmap pixmap = NULL;

            if (uri.isEmpty()) {
                QVariant picture = player->metaData(QtMultimediaKit::CoverArtImage);
                // Load picture from metadata
                if (!picture.isNull() && picture.canConvert<QByteArray>())
                    pixmap.loadFromData(picture.value<QByteArray>());

                // Load some jpg from same dir as media
                else {
                    QUrl url = player->media().canonicalUrl();
                    QString path = url.path();
                    path = path.mid(1,path.lastIndexOf("/"));
                    QRegExp rx("*.jpg");
                    rx.setCaseSensitivity(Qt::CaseInsensitive);
                    rx.setPatternSyntax(QRegExp::Wildcard);
                    QDir directory(path);
                    QStringList allFiles = directory.entryList(QDir::Files | QDir::NoSymLinks);

                    foreach (QString file, allFiles)
                        if (rx.exactMatch(file)) {
                            path.append(file);
                            break;
                        }
                   pixmap.load(path);
                }
                // Load picture from file pointed by uri
            } else
                pixmap.load(uri.toString());

            coverLabel->setPixmap((!pixmap.isNull())?pixmap:QPixmap());
            coverLabel->setAlignment(Qt::AlignCenter);
            coverLabel->setScaledContents(true);
            }
    hideOrShowCoverArt();
    }
#else
    //qDebug() << "update metadata" << player->metaData(QtMultimediaKit::Title).toString();
    if (player->isMetaDataAvailable()) {
        setTrackInfo(QString("%1 - %2")
                .arg(player->metaData(QtMultimediaKit::AlbumArtist).toString())
                .arg(player->metaData(QtMultimediaKit::Title).toString()));

        if (coverLabel) {
            QUrl url = player->metaData(QtMultimediaKit::CoverArtUrlLarge).value<QUrl>();

            coverLabel->setPixmap(!url.isEmpty()
                    ? QPixmap(url.toString())
                    : QPixmap());
        }
    }
#endif
}

void TestPlayers60::previousClicked()
{
    // Go to previous track if we are within the first 5 seconds of playback
    // Otherwise, seek to the beginning.
    if(player->position() <= 5000)
        playlist->previous();
    else
        player->setPosition(0);
}

void TestPlayers60::jump(const QModelIndex &index)
{
#ifdef Q_OS_SYMBIAN
    if (playlistDialog->isVisible())
        playlistDialog->accept();
#endif
    if (index.isValid()) {
        playlist->setCurrentIndex(index.row());
        player->play();
    }
}

void TestPlayers60::playlistPositionChanged(int currentItem)
{
    playlistView->setCurrentIndex(playlistModel->index(currentItem, 0));
}

void TestPlayers60::seek(int seconds)
{
    player->setPosition(seconds * 1000);
}

void TestPlayers60::statusChanged(QMediaPlayer::MediaStatus status)
{
    handleCursor(status);

    // handle status message
    switch (status) {
    case QMediaPlayer::UnknownMediaStatus:
    case QMediaPlayer::NoMedia:
    case QMediaPlayer::LoadedMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        setStatusInfo(QString());
        break;
    case QMediaPlayer::LoadingMedia:
        setStatusInfo(tr("Loading..."));
        break;
    case QMediaPlayer::StalledMedia:
        setStatusInfo(tr("Media Stalled"));
        break;
    case QMediaPlayer::EndOfMedia:
        QApplication::alert(this);
        break;
    case QMediaPlayer::InvalidMedia:
        displayErrorMessage();
        break;
    }
}

void TestPlayers60::handleCursor(QMediaPlayer::MediaStatus status)
{
#ifndef QT_NO_CURSOR
    if( status == QMediaPlayer::LoadingMedia ||
        status == QMediaPlayer::BufferingMedia ||
        status == QMediaPlayer::StalledMedia)
        setCursor(QCursor(Qt::BusyCursor));
    else
        unsetCursor();
#endif
}

void TestPlayers60::bufferingProgress(int progress)
{
    setStatusInfo(tr("Buffering %4%").arg(progress));
}

void TestPlayers60::setTrackInfo(const QString &info)
{
    trackInfo = info;
#ifdef Q_OS_SYMBIAN
    QMainWindow *main = qobject_cast<QMainWindow *>(this->parent());
    if (!statusInfo.isEmpty())
        main->setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        main->setWindowTitle(trackInfo);
#else
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
#endif
}

void TestPlayers60::setStatusInfo(const QString &info)
{
    statusInfo = info;
#ifdef Q_OS_SYMBIAN
    QMainWindow *main = qobject_cast<QMainWindow *>(this->parent());
    if (!statusInfo.isEmpty())
        main->setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        main->setWindowTitle(trackInfo);
#else
    if (!statusInfo.isEmpty())
        setWindowTitle(QString("%1 | %2").arg(trackInfo).arg(statusInfo));
    else
        setWindowTitle(trackInfo);
#endif
}

void TestPlayers60::displayErrorMessage()
{
#ifdef Q_OS_SYMBIAN
    if(player->error()!=QMediaPlayer::NoError)
        QMessageBox::critical(NULL, tr("Error"), player->errorString(), QMessageBox::Ok);
#else
    setStatusInfo(player->errorString());
#endif


}

#ifndef Q_OS_SYMBIAN
void TestPlayers60::showColorDialog()
{
    if (!colorDialog) {
        QSlider *brightnessSlider = new QSlider(Qt::Horizontal);
        brightnessSlider->setRange(-100, 100);
        brightnessSlider->setValue(videoWidget->brightness());
        connect(brightnessSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setBrightness(int)));
        connect(videoWidget, SIGNAL(brightnessChanged(int)), brightnessSlider, SLOT(setValue(int)));

        QSlider *contrastSlider = new QSlider(Qt::Horizontal);
        contrastSlider->setRange(-100, 100);
        contrastSlider->setValue(videoWidget->contrast());
        connect(contrastSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setContrast(int)));
        connect(videoWidget, SIGNAL(contrastChanged(int)), contrastSlider, SLOT(setValue(int)));

        QSlider *hueSlider = new QSlider(Qt::Horizontal);
        hueSlider->setRange(-100, 100);
        hueSlider->setValue(videoWidget->hue());
        connect(hueSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setHue(int)));
        connect(videoWidget, SIGNAL(hueChanged(int)), hueSlider, SLOT(setValue(int)));

        QSlider *saturationSlider = new QSlider(Qt::Horizontal);
        saturationSlider->setRange(-100, 100);
        saturationSlider->setValue(videoWidget->saturation());
        connect(saturationSlider, SIGNAL(sliderMoved(int)), videoWidget, SLOT(setSaturation(int)));
        connect(videoWidget, SIGNAL(saturationChanged(int)), saturationSlider, SLOT(setValue(int)));

        QFormLayout *layout = new QFormLayout;
        layout->addRow(tr("Brightness"), brightnessSlider);
        layout->addRow(tr("Contrast"), contrastSlider);
        layout->addRow(tr("Hue"), hueSlider);
        layout->addRow(tr("Saturation"), saturationSlider);

        colorDialog = new QDialog(this);
        colorDialog->setWindowTitle(tr("Color Options"));
        colorDialog->setLayout(layout);
    }
    colorDialog->show();
}
#endif
#ifdef Q_OS_SYMBIAN
void TestPlayers60::createMenus()
{
    toggleAspectRatio = new QAction(tr("Ignore Aspect Ratio"), this);
    toggleAspectRatio->setCheckable(true);
    qobject_cast<QMainWindow *>(this->parent())->menuBar()->addAction(toggleAspectRatio);
    connect(toggleAspectRatio, SIGNAL(toggled(bool)), this, SLOT(handleAspectRatio(bool)));

    showYoutubeDialog = new QAction(tr("Youtube Search"), this);
    qobject_cast<QMainWindow *>(this->parent())->menuBar()->addAction(showYoutubeDialog);
    connect(showYoutubeDialog, SIGNAL(triggered()), this, SLOT(launchYoutubeDialog()));

    setAudioOutputDefault = new QAction(tr("Default output"), this);
    connect(setAudioOutputDefault, SIGNAL(triggered()), this, SLOT(handleAudioOutputDefault()));

    setAudioOutputAll = new QAction(tr("All outputs"), this);
    connect(setAudioOutputAll, SIGNAL(triggered()), this, SLOT(handleAudioOutputAll()));

    setAudioOutputNone = new QAction(tr("No output"), this);
    connect(setAudioOutputNone, SIGNAL(triggered()), this, SLOT(handleAudioOutputNone()));

    setAudioOutputEarphone = new QAction(tr("Earphone output"), this);
    connect(setAudioOutputEarphone, SIGNAL(triggered()), this, SLOT(handleAudioOutputEarphone()));

    setAudioOutputSpeaker = new QAction(tr("Speaker output"), this);
    connect(setAudioOutputSpeaker, SIGNAL(triggered()), this, SLOT(handleAudioOutputSpeaker()));

    audioOutputMenu = new QMenu(tr("Set Audio Output"), this);
    audioOutputMenu->addAction(setAudioOutputDefault);
    audioOutputMenu->addAction(setAudioOutputAll);
    audioOutputMenu->addAction(setAudioOutputNone);
    audioOutputMenu->addAction(setAudioOutputEarphone);
    audioOutputMenu->addAction(setAudioOutputSpeaker);

    qobject_cast<QMainWindow *>(this->parent())->menuBar()->addMenu(audioOutputMenu);
}

void TestPlayers60::handleFullScreen(bool isFullscreen)
{
    QMainWindow* mainWindow = qobject_cast<QMainWindow *>(this->parent());
    if(isFullscreen) {
        if(player->state()==QMediaPlayer::StoppedState)
            videoWidget->setFullScreen(false);
        else
            videoWidget->setFullScreen(true);

        qobject_cast<QMainWindow *>(this->parent())->showFullScreen();
    } else
        qobject_cast<QMainWindow *>(this->parent())->showMaximized();
}

void TestPlayers60::handleAspectRatio(bool aspectRatio)
{
    if(aspectRatio) {
        toggleAspectRatio->setText(tr("Keep Aspect Ratio"));
        videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);

    } else {
        toggleAspectRatio->setText(tr("Ignore Aspect Ratio"));
        videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    }
}

void TestPlayers60::handleAudioOutputDefault()
{
    audioEndpointSelector->setActiveEndpoint("Default");
}

void TestPlayers60::handleAudioOutputAll()
{
    audioEndpointSelector->setActiveEndpoint("All");
}

void TestPlayers60::handleAudioOutputNone()
{
    audioEndpointSelector->setActiveEndpoint("None");
}

void TestPlayers60::handleAudioOutputEarphone()
{
    audioEndpointSelector->setActiveEndpoint("Earphone");
}

void TestPlayers60::handleAudioOutputSpeaker()
{
    audioEndpointSelector->setActiveEndpoint("Speaker");
}

void TestPlayers60::handleAudioOutputChangedSignal(const QString&)
{
    QMessageBox msgBox;
    msgBox.setText("Output changed: " + audioEndpointSelector->activeEndpoint());
    msgBox.exec();
}

void TestPlayers60::handleMediaStreamChanged()
{
    QMessageBox msgBox;
    msgBox.setText("Media stream changed, stream count: " + QString().setNum(streamControl->streamCount())); //QString().setNum(streamControl->streamCount()));
    msgBox.exec();
}

void TestPlayers60::hideOrShowCoverArt()
{
    if(player->isVideoAvailable()) {
        coverLabel->hide();
        videoWidget->show();
        videoWidget->repaint();
    } else {
        videoWidget->hide();
        QApplication::setActiveWindow(this);
        coverLabel->show();
    }
}

void TestPlayers60::handleStateChange(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::PausedState)
        return;

    handleFullScreen(qobject_cast<QMainWindow *>(this->parent())->isFullScreen());
}

void TestPlayers60::handleMediaKeyEvent(MediaKeysObserver::MediaKeys key)
{
    switch (key) {
        case MediaKeysObserver::EVolIncKey:
            player->setVolume(player->volume() + 10);
            break;
        case MediaKeysObserver::EVolDecKey:
            player->setVolume(player->volume() - 10);
            break;
        default:
        break;
    }
}
void TestPlayers60::showPlayList()
{
    if (!playlistDialog)
        return;

    playlistDialog->exec();
}

void TestPlayers60::launchYoutubeDialog()
{
    if(!youtubeDialog)  {
        youtubeDialog = new QDialog(this);

        QLineEdit *input= new QLineEdit(youtubeDialog);
        QPushButton *searchButton = new QPushButton("Search", youtubeDialog);
        QListWidget *resultList = new QListWidget(youtubeDialog);
        QAction *add = new QAction(tr("Add"), youtubeDialog);
        QAction *close = new QAction(tr("Close"), youtubeDialog);

        add->setSoftKeyRole(QAction::PositiveSoftKey);
        close->setSoftKeyRole(QAction::NegativeSoftKey);
        youtubeDialog->addAction(add);
        youtubeDialog->addAction(close);

        connect(searchButton, SIGNAL(clicked()), this, SLOT(searchYoutubeVideo()));
        connect(add, SIGNAL(triggered()), this, SLOT(addYoutubeVideo()));
        connect(close, SIGNAL(triggered()), youtubeDialog, SLOT(close()));
        connect(&http, SIGNAL(requestFinished(int, bool)), this, SLOT(youtubeHttpRequestFinished(int, bool)));
        connect(&http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader&)), this, SLOT(youtubeReadResponseHeader(const QHttpResponseHeader&)));

        QHBoxLayout *topLayout = new QHBoxLayout;
        topLayout->addWidget(input);
        topLayout->addWidget(searchButton);

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(resultList);
        youtubeDialog->setLayout(mainLayout);
    }
    youtubeDialog->showMaximized();
}

void TestPlayers60::youtubeReadResponseHeader(const QHttpResponseHeader& responseHeader)
{
    switch (responseHeader.statusCode())
    {
        case 200:   // Ok
        case 301:   // Moved Permanently
        case 302:   // Found
        case 303:   // See Other
        case 307:   // Temporary Redirect
            // these are not error conditions
            break;
        default: {
            http.abort();
            QMessageBox::critical(NULL, tr("Error"), tr("Download failed: %1.").arg(responseHeader.reasonPhrase()));
            break;
        }
    }
}

void TestPlayers60::addYoutubeVideo()
{
    if(!youtubeDialog)
        return;

    QListWidget *resultList = youtubeDialog->findChild<QListWidget *>();
    if(!resultList || resultList->count() == 0)
        return;

    playlist->addMedia(resultList->currentItem()->data(Qt::UserRole).toUrl());
}

void TestPlayers60::searchYoutubeVideo()
{
    if(!youtubeDialog)
        return;

    QLineEdit *input = youtubeDialog->findChild<QLineEdit *>();
    QListWidget *resultList = youtubeDialog->findChild<QListWidget *>();
    QString urlstring = QString("http://gdata.youtube.com/feeds/api/videos?q=%1&max-results=25&v=2&format=6").arg(input->text().replace(' ', '+'));
    QUrl url(urlstring);
    http.setHost(url.host(), QHttp::ConnectionModeHttp, url.port() == -1 ? 0 : url.port());
    resultList->clear();
    httpGetId = http.get(urlstring);
}

void TestPlayers60::youtubeHttpRequestFinished(int requestId, bool error)
{
    if(!youtubeDialog || requestId != httpGetId)
        return;

    if (error) {
        QMessageBox::critical(NULL, tr("Error"), tr("Download failed: %1.").arg(http.errorString()));
        return;
    }

    QTemporaryFile file;
    if (!file.open()) {
        QMessageBox::critical(NULL, tr("Error"), tr("Could not open temporary file"));
        return;
    }

    QString data = http.readAll();
    QTextStream out(&file);
    out << data;
    file.close();

    QDomDocument domDocument;
    QString errorMessage;
    if (!domDocument.setContent(&file, true, &errorMessage)) {
        QMessageBox::critical(NULL, tr("Error"), errorMessage);
        return;
    }

    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "feed")
        return;

    QListWidget *resultList = youtubeDialog->findChild<QListWidget *>();
    QDomElement entryElement = root.firstChildElement("entry");
    while(!entryElement.isNull())
    {
        QString title = entryElement.firstChildElement("title").text();
        QDomElement groupElement = entryElement.firstChildElement("group");
        QDomElement incidentElement2 = groupElement.firstChildElement("content");
        while(!incidentElement2.isNull())
        {
            // "6" = MPEG-4 SP video (up to 176x144) and AAC audio.
            if (incidentElement2.attribute("format") == "6") {
                QListWidgetItem* item = new QListWidgetItem(title, resultList);
                item->setData(Qt::UserRole, incidentElement2.attribute("url"));
                break;
            }
            incidentElement2 = incidentElement2.nextSiblingElement("content");
        }
        entryElement = entryElement.nextSiblingElement("entry");
    }
}
#endif