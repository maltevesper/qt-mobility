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

#import <QTKit/QTKit.h>

#include "qt7backend.h"

#include "qt7playercontrol.h"
#include "qt7movieviewrenderer.h"
#include "qt7playersession.h"
#include <QtCore/qdebug.h>
#include <QtCore/qcoreevent.h>
#include <QtCore/qcoreapplication.h>

#include <QtMultimedia/qabstractvideobuffer.h>
#include <QtMultimedia/qabstractvideosurface.h>
#include <QtMultimedia/qvideosurfaceformat.h>


QT_USE_NAMESPACE

class NSBitmapVideoBuffer : public QAbstractVideoBuffer
{
public:
    NSBitmapVideoBuffer(NSBitmapImageRep *buffer)
        : QAbstractVideoBuffer(NoHandle)
        , m_buffer(buffer)
        , m_mode(NotMapped)
    {
        [m_buffer retain];
    }

    virtual ~NSBitmapVideoBuffer()
    {
        [m_buffer release];
    }

    MapMode mapMode() const { return m_mode; }

    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine)
    {
        if (mode != NotMapped && m_mode == NotMapped) {
            if (numBytes)
                *numBytes = [m_buffer bytesPerPlane];

            if (bytesPerLine)
                *bytesPerLine = [m_buffer bytesPerRow];

            m_mode = mode;

            return [m_buffer bitmapData];
        } else {
            return 0;
        }
    }

    void unmap() { m_mode = NotMapped; }

private:
    NSBitmapImageRep *m_buffer;
    MapMode m_mode;
};


#define VIDEO_TRANSPARENT(m) -(void)m:(NSEvent *)e{[[self superview] m:e];}

@interface HiddenQTMovieView : QTMovieView
{
@private
    QWidget *m_window;
    QT7MovieViewRenderer *m_renderer;
}

- (HiddenQTMovieView *) initWithRenderer:(QT7MovieViewRenderer *)renderer;
- (void) setRenderer:(QT7MovieViewRenderer *)renderer;
- (void) setDrawRect:(const QRect &)rect;
@end

@implementation HiddenQTMovieView

- (HiddenQTMovieView *) initWithRenderer:(QT7MovieViewRenderer *)renderer
{
    self = [super initWithFrame:NSZeroRect];
    if (self) {
        [self setControllerVisible:NO];
        [self setDelegate:self];

        self->m_renderer = renderer;

        self->m_window = new QWidget;
        self->m_window->setWindowOpacity(0.0);
        self->m_window->show();
        self->m_window->hide();

        [(NSView *)(self->m_window->winId()) addSubview:self];
        [self setDrawRect:QRect(0,0,1,1)];
    }
    return self;
}

- (void) dealloc
{
    [super dealloc];
}

- (void) setRenderer:(QT7MovieViewRenderer *)renderer
{
    m_renderer = renderer;
}

- (void) setDrawRect:(const QRect &)rect
{
    NSRect nsrect;
    nsrect.origin.x = rect.x();
    nsrect.origin.y = rect.y();
    nsrect.size.width = rect.width();
    nsrect.size.height = rect.height();
    [self setFrame:nsrect];
}

- (CIImage *) view:(QTMovieView *)view willDisplayImage:(CIImage *)img
{
    // This method is called from QTMovieView just
    // before the image will be drawn.
    Q_UNUSED(view);
    if (m_renderer) {
        NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithCIImage:img];
        CGRect bounds = [img extent];
        int w = bounds.size.width;
        int h = bounds.size.height;

        // Swap red and blue (same as QImage::rgbSwapped, but without copy)
        uchar *data = [bitmap bitmapData];
        //qDebug() << data << w << h;
        int bytesPerLine = [bitmap bytesPerRow];
        for (int i=0; i<h; ++i) {
            quint32 *p = (quint32*)data;
            data += bytesPerLine;
            quint32 *end = p + w;
            while (p < end) {
                *p = ((*p << 16) & 0xff0000) | ((*p >> 16) & 0xff) | (*p & 0xff00ff00);
                p++;
            }
        }

        QVideoFrame frame( new NSBitmapVideoBuffer(bitmap), QSize(w,h), QVideoFrame::Format_RGB32 );

        //static int i=0;
        //i++;
        //QImage img([bitmap bitmapData], w, h, QImage::Format_RGB32);
        //img.save(QString("img%1.jpg").arg(i));

        [bitmap release];

        if (m_renderer)
            m_renderer->renderFrame(frame);
    }

    return img;
}

// Override this method so that the movie doesn't stop if
// the window becomes invisible
- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
    Q_UNUSED(newWindow);
}


VIDEO_TRANSPARENT(mouseDown);
VIDEO_TRANSPARENT(mouseDragged);
VIDEO_TRANSPARENT(mouseUp);
VIDEO_TRANSPARENT(mouseMoved);
VIDEO_TRANSPARENT(mouseEntered);
VIDEO_TRANSPARENT(mouseExited);
VIDEO_TRANSPARENT(rightMouseDown);
VIDEO_TRANSPARENT(rightMouseDragged);
VIDEO_TRANSPARENT(rightMouseUp);
VIDEO_TRANSPARENT(otherMouseDown);
VIDEO_TRANSPARENT(otherMouseDragged);
VIDEO_TRANSPARENT(otherMouseUp);
VIDEO_TRANSPARENT(keyDown);
VIDEO_TRANSPARENT(keyUp);
VIDEO_TRANSPARENT(scrollWheel)

@end


QT7MovieViewRenderer::QT7MovieViewRenderer(QObject *parent)
   :QT7VideoRendererControl(parent),
    m_movie(0),
    m_movieView(0),
    m_surface(0)
{    
}

QT7MovieViewRenderer::~QT7MovieViewRenderer()
{
    [(HiddenQTMovieView*)m_movieView setRenderer:0];

    QMutexLocker locker(&m_mutex);
    m_currentFrame = QVideoFrame();
    [(HiddenQTMovieView*)m_movieView release];
}

void QT7MovieViewRenderer::setupVideoOutput()
{
    AutoReleasePool pool;

    qDebug() << "QT7MovieViewRenderer::setupVideoOutput" << m_movie << m_surface;

    HiddenQTMovieView *movieView = (HiddenQTMovieView*)m_movieView;

    if (movieView && !m_movie) {
        [movieView setMovie:nil];
    }

    if (m_movie) {
        NSSize size = [[(QTMovie*)m_movie attributeForKey:@"QTMovieNaturalSizeAttribute"] sizeValue];

        m_nativeSize = QSize(size.width, size.height);

        if (!movieView) {
            movieView = [[HiddenQTMovieView alloc] initWithRenderer:this];
            m_movieView = movieView;
            [movieView setControllerVisible:NO];
        }

        [movieView setMovie:(QTMovie*)m_movie];
        //[movieView setDrawRect:QRect(QPoint(0,0), m_nativeSize)];
    }

    if (m_surface && !m_nativeSize.isEmpty()) {
        QVideoSurfaceFormat format(m_nativeSize, QVideoFrame::Format_RGB32);

        if (m_surface->isActive() && m_surface->surfaceFormat() != format) {
            qDebug() << "Surface format was changed, stop the surface.";
            m_surface->stop();
        }

        if (!m_surface->isActive()) {
            qDebug() << "Starting the surface with format" << format;
            if (!m_surface->start(format))
                qDebug() << "failed to start video surface" << m_surface->error();
        }
    }
}

void QT7MovieViewRenderer::setEnabled(bool)
{
}

void QT7MovieViewRenderer::setMovie(void *movie)
{
    if (movie == m_movie)
        return;

    QMutexLocker locker(&m_mutex);
    m_movie = movie;
    setupVideoOutput();
}

QAbstractVideoSurface *QT7MovieViewRenderer::surface() const
{
    return m_surface;
}

void QT7MovieViewRenderer::setSurface(QAbstractVideoSurface *surface)
{
    if (surface == m_surface)
        return;

    QMutexLocker locker(&m_mutex);

    if (m_surface && m_surface->isActive())
        m_surface->stop();

    m_surface = surface;
    setupVideoOutput();
}

void QT7MovieViewRenderer::renderFrame(const QVideoFrame &frame)
{
    {
        QMutexLocker locker(&m_mutex);
        m_currentFrame = frame;
    }

    qApp->postEvent(this, new QEvent(QEvent::User), Qt::HighEventPriority);
}

bool QT7MovieViewRenderer::event(QEvent *event)
{
    if (event->type() == QEvent::User) {
        QMutexLocker locker(&m_mutex);
        if (m_surface->isActive())
            m_surface->present(m_currentFrame);
    }

    return QT7VideoRendererControl::event(event);
}