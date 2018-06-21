#pragma once

#include <QWidget>
#include <QTimer>
#include <QApplication>
#include <QGridLayout>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgQt/GraphicsWindowQt>
#include <iostream>

#include "simvis/osg_camera_man.h"
#include "simvis/plane.h"
#include "xo/filesystem/path.h"

class QOsgViewer : public QWidget, public osgViewer::CompositeViewer
{
public:
	QOsgViewer(QWidget* parent = 0, Qt::WindowFlags f = 0, osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded);
	QWidget* addViewWidget( osgQt::GraphicsWindowQt* gw );
	osgQt::GraphicsWindowQt* createGraphicsWindow( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false );

	virtual void paintEvent( QPaintEvent* event ) override;

	void setScene( vis::scene* s );
	void setHud( const xo::path& file );
	void setClearColor( const osg::Vec4& col );
	void moveCamera( const osg::Vec3d& delta_pos );
	void startCapture( const std::string& filename );
	void stopCapture();
	void captureCurrentFrame( const std::string& filename );
	bool isCapturing() { return capture_handler_ != nullptr; }
	void startTimer() { timer_.start( 10 ); }
	void stopTimer() { timer_.stop(); }
	vis::osg_camera_man& getCameraMan() { return *camera_man_; }
	void setFrameTime( double t );

protected:
	bool eventFilter( QObject* obj, QEvent* event );
	size_t frame_count_;
	QTimer timer_;
	osg::ref_ptr< vis::osg_camera_man > camera_man_;
	osg::ref_ptr< osgViewer::ScreenCaptureHandler > capture_handler_;
	osg::ref_ptr< osgViewer::View > view_;
	vis::scene* scene_;
	vis::plane hud_;
	double current_frame_time_;
	double last_drawn_frame_time_;
};
