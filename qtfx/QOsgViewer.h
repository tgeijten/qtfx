#pragma once

#include <QWidget>
#include <QTimer>
#include <QApplication>
#include <QGridLayout>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgQt/GraphicsWindowQt>
#include <osg/PositionAttitudeTransform>
#include <osgUtil/LineSegmentIntersector>
#include <osgDB/Options>
#include "osg_camera_man.h"

#include "xo/filesystem/path.h"
#include "xo/geometry/vec3_type.h"
#include "osgGA/GUIEventAdapter"
#include "xo/time/timer.h"

#include <string>

class QOsgViewer : public QWidget, public osgViewer::CompositeViewer
{
	Q_OBJECT

public:
	QOsgViewer( QWidget* parent = 0, Qt::WindowFlags f = 0, osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::CompositeViewer::SingleThreaded );
	osgQt::GLWidget* addViewWidget( osgQt::GraphicsWindowQt* gw );
	osgQt::GraphicsWindowQt* createGraphicsWindow( int x, int y, int w, int h, const std::string& name = "", bool windowDecoration = false );

	virtual void paintEvent( QPaintEvent* event ) override;
	virtual bool event( QEvent* event ) override;

	void setScene( osg::Group* s );
	void createHud( const xo::path& file );
	void setClearColor( const osg::Vec4& col );
	void moveCamera( const osg::Vec3d& delta_pos );
	void setFocusPoint( const osg::Vec3d& p );
	void setLightOffset( const xo::vec3f& l );
	void startCapture( const std::string& filename );
	void stopCapture();
	void captureCurrentFrame( const std::string& filename );
	bool isCapturing() { return capture_handler_ != nullptr; }
	void stopPlaybackMode() { timer_.start( 10 ); getCameraMan().setPlaybackMode( false ); }
	void startPlaybackMode() { timer_.stop(); getCameraMan().setPlaybackMode( true ); }
	bool isPlaybackMode() const { return timer_.isActive(); }
	vis::osg_camera_man& getCameraMan() { return *camera_man_; }
	void setFrameTime( double t );
	void updateCameraAnimation( double t, float dt );
	bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );
	void updateIntersections( const osgGA::GUIEventAdapter& ea );
	const osgUtil::LineSegmentIntersector::Intersections& getIntersections() const { return intersections_; }
	size_t getIntersectionCount() const { return intersections_.size(); }
	const osgUtil::LineSegmentIntersector::Intersection* getTopNamedIntersection( const std::string& skipName = "!" ) const;
	const osg::Node* getTopNamedIntersectionNode( const std::string& skipName = "!" ) const;
	osgQt::GLWidget* viewWidget() { return view_widget_; }
	void enableObjectCache( bool enable );
	osgViewer::View& getMainView() { return *view_; }

signals:
	void pressed();
	void clicked();
	void hover();
	void tooltip();

public slots:
	void timerUpdate();

protected:
	bool eventFilter( QObject* obj, QEvent* event );
	osgDB::Options* getOrCreateOptions();

	void updateHudPos();
	void updateLightPos();
	virtual void viewerInit() override;

	size_t frame_count_;
	QTimer timer_;
	int width_, height_;
	osg::ref_ptr< vis::osg_camera_man > camera_man_;
	osg::ref_ptr< osgViewer::ScreenCaptureHandler > capture_handler_;
	osg::ref_ptr< osgViewer::View > view_;
	osg::ref_ptr< osg::Group > scene_;
	osgQt::GLWidget* view_widget_;

	osg::ref_ptr< osg::Light > scene_light_;
	xo::vec3f scene_light_offset_;
	bool directional_scene_light_ = true;

	osg::ref_ptr< osg::PositionAttitudeTransform > hud_node_;
	float hud_size = 0.1f;

	double current_frame_time_;
	double last_drawn_frame_time_;

	size_t mouse_drag_count_;
	bool mouse_hover_allowed_;
	xo::timer mouse_hover_timer_;
	xo::time mouse_hover_duration_;

	osgUtil::LineSegmentIntersector::Intersections intersections_;
};
