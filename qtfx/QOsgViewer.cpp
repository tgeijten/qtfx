#include "QOsgViewer.h"

#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include "simvis/osg_camera_man.h"
#include "xo/system/log.h"
#include "qevent.h"
#include "simvis/group.h"
#include "simvis/scene.h"

class QOsgEventHandler : public osgGA::GUIEventHandler 
{ 
public: 
	virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* o, osg::NodeVisitor* nv )
	{
		if ( ea.getEventType() == osgGA::GUIEventAdapter::RESIZE )
		{
			xo::log::info( "Viewer resized to ", ea.getWindowWidth(), "x", ea.getWindowHeight() );
			return true;
		}
		else return false;
	}
};

QOsgViewer::QOsgViewer( QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/, osgViewer::ViewerBase::ThreadingModel threadingModel/*=osgViewer::CompositeViewer::SingleThreaded*/ ) :
QWidget( parent, f ),
capture_handler_( nullptr ),
frame_count_( 0 )
{
	QCoreApplication::instance()->installEventFilter( this );
	setThreadingModel( threadingModel );

	// disable the default setting of viewer.done() by pressing Escape.
	setKeyEventSetsDone( 0 );

	// setup viewer grid
	QWidget* widget1 = addViewWidget( createGraphicsWindow( 0, 0, 100, 100, "", true ) );
	auto* grid = new QGridLayout;
	grid->addWidget( widget1 );
	setLayout( grid );
	grid->setMargin( 1 );

	// start timer
	// TODO: remove this -- only update after something has changed
	connect( &timer_, SIGNAL( timeout() ), this, SLOT( update() ) );
	timer_.start( 1000 / 120 );
}

QWidget* QOsgViewer::addViewWidget( osgQt::GraphicsWindowQt* gw )
{
	view_ = new osgViewer::View;
	addView( view_ );

	osg::Camera* cam = view_->getCamera();
	cam->setGraphicsContext( gw );

	const osg::GraphicsContext::Traits* traits = gw->getTraits();

	cam->setClearColor( osg::Vec4( 0.55, 0.55, 0.55, 1.0 ) );
	cam->setViewport( new osg::Viewport( 0, 0, traits->width, traits->height ) );
	cam->setProjectionMatrixAsPerspective( 30.0f, static_cast<double>( traits->width ) / static_cast<double>( traits->height ), 1.0f, 10000.0f );

	// add event handlers
	view_->addEventHandler( new osgViewer::StatsHandler );
	view_->addEventHandler( new QOsgEventHandler );

	// disable lighting by default
	view_->setLightingMode( osg::View::NO_LIGHT );

	// setup camera manipulator
	gw->setTouchEventsEnabled( true );

	camera_man_ = new vis::osg_camera_man();
	camera_man_->setVerticalAxisFixed( false );
	view_->setCameraManipulator( camera_man_ );

	return gw->getGLWidget();
}

osgQt::GraphicsWindowQt* QOsgViewer::createGraphicsWindow( int x, int y, int w, int h, const std::string& name/*=""*/, bool windowDecoration/*=false */ )
{
	osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
	ds->setNumMultiSamples( 2 );

	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->windowName = name;
	traits->windowDecoration = windowDecoration;
	traits->x = x;
	traits->y = y;
	traits->width = w;
	traits->height = h;
	traits->doubleBuffer = true;
	traits->alpha = ds->getMinimumNumAlphaBits();
	traits->stencil = ds->getMinimumNumStencilBits();
	traits->sampleBuffers = ds->getMultiSamples();
	traits->samples = ds->getNumMultiSamples();

	return new osgQt::GraphicsWindowQt( traits.get() );
}

void QOsgViewer::paintEvent( QPaintEvent* event )
{
	// prevent capturing duplicate frames
	if ( isCapturing() && current_frame_time_ == last_drawn_frame_time_ )
		return; // this frame was already captured, skip

	++frame_count_;
	frame();
	last_drawn_frame_time_ = current_frame_time_;
}

void QOsgViewer::setScene( vis::scene* s )
{
	scene_ = s;
	for ( size_t i = 0; i < getNumViews(); ++i )
		getView( i )->setSceneData( s->osg_node() );
}

void QOsgViewer::setHud( const xo::path& file )
{
	//hud_ = vis::plane( xo::vec3f( 1, 0, 0 ), xo::vec3f( 0, 1, 0 ), file, 1.0f, 1.0f );
	//hud_ = vis::plane( xo::vec3f( 1, 0, 0 ), xo::vec3f( 0, 1, 0 ), file, 1.0f, 1.0f );
	//scene_->attach( hud_ );
	//view_->getCamera()->addChild( hud_.osg_node() );
}

void QOsgViewer::setClearColor( const osg::Vec4& col )
{
	view_->getCamera()->setClearColor( col );
}

void QOsgViewer::moveCamera( const osg::Vec3d& delta_pos )
{
	camera_man_->setCenter( camera_man_->getCenter() + delta_pos );
}

void QOsgViewer::startCapture( const std::string& filename )
{
	// create capture handler
	xo::log::info( "Started capturing video to ", filename );
	capture_handler_ = new osgViewer::ScreenCaptureHandler( 
		new osgViewer::ScreenCaptureHandler::WriteToFile( filename, "png", osgViewer::ScreenCaptureHandler::WriteToFile::SEQUENTIAL_NUMBER ), -1 );
	view_->addEventHandler( capture_handler_ );
	capture_handler_->startCapture();
}

void QOsgViewer::stopCapture()
{
	last_drawn_frame_time_ = ~size_t ( 0 );
	if ( capture_handler_ )
	{
		xo::log::info( "Video capture stopped" );
		capture_handler_->stopCapture();
		capture_handler_ = nullptr;
	}
}

void QOsgViewer::captureCurrentFrame( const std::string& filename )
{
	stopCapture();

	capture_handler_ = new osgViewer::ScreenCaptureHandler(
		new osgViewer::ScreenCaptureHandler::WriteToFile( filename, "png", osgViewer::ScreenCaptureHandler::WriteToFile::OVERWRITE ), -1 );
	view_->addEventHandler( capture_handler_ );
	capture_handler_->startCapture();
	frame();
	capture_handler_->stopCapture();
	capture_handler_ = nullptr;
	xo::log::info( "Written image to ", filename );
}

void QOsgViewer::setFrameTime( double t )
{
	if ( current_frame_time_ != t )
	{
		current_frame_time_ = t;
		repaint();
	}
}

bool QOsgViewer::eventFilter( QObject* obj, QEvent* event )
{
	return QObject::eventFilter( obj, event );
}
