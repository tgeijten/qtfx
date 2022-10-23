#include "QOsgViewer.h"

#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>
#include "xo/system/log.h"
#include "qevent.h"
#include "osg/MatrixTransform"
#include "xo/geometry/quat.h"
#include "osg/Material"
#include "osg/PositionAttitudeTransform"
#include "osgUtil/LineSegmentIntersector"
#include "xo/system/assert.h"
#include "osgDB/FileUtils"
#include "xo/filesystem/filesystem.h"
#include "xo/container/container_tools.h"

// fix OSG plugin folder path
void fix_osg_library_file_path() {
#if defined( __linux__ )
	const auto search_path = ( xo::get_application_dir().parent_path() / "lib" ).str();
	auto path_list = osgDB::getLibraryFilePathList();
	if ( !xo::contains( path_list, search_path ) ) {
		path_list.push_back( search_path );
		osgDB::setLibraryFilePathList( path_list );
		xo::log::info( "Added path to OSG list: ", search_path );
	}
#endif
}

// class for routing GUI events to QOsgViewer
// This is needed because QOsgViewer can't derive from GUIEventHandler directly
// because both are derived from osg::Object (basically, because of bad design)
class QOsgEventHandler : public osgGA::GUIEventHandler
{
public:
	QOsgEventHandler( QOsgViewer& viewer ) : viewer_( viewer ) {}
	virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) {
		return viewer_.handle( ea, aa );
	}
private:
	QOsgViewer& viewer_;
};

QOsgViewer::QOsgViewer( QWidget* parent /*= 0*/, Qt::WindowFlags f /*= 0*/, osgViewer::ViewerBase::ThreadingModel threadingModel/*=osgViewer::CompositeViewer::SingleThreaded*/ ) :
	QWidget( parent, f ),
	frame_count_( 0 ),
	capture_handler_( nullptr ),
	scene_light_offset_( -2, 8, 3 ),
	current_frame_time_( -1 ),
	mouse_drag_count_( 0 ),
	mouse_hover_allowed_( false ),
	mouse_hover_duration_( xo::time_from_milliseconds( 100 ) )
{
	fix_osg_library_file_path();

	setThreadingModel( threadingModel );

	// disable the default setting of viewer.done() by pressing Escape.
	setKeyEventSetsDone( 0 );

	// setup viewer grid
	view_widget_ = addViewWidget( createGraphicsWindow( 0, 0, 100, 100, "", true ) );
	auto* grid = new QGridLayout;
	grid->addWidget( view_widget_ );
	setLayout( grid );
	grid->setMargin( 1 );

	// start timer that checks for updates in the camera manager
	connect( &timer_, SIGNAL( timeout() ), this, SLOT( timerUpdate() ) );
	startTimer();

	// this allows us to detect events, and update the viewer accordingly
	// overriding mouseMoveEvent, etc. does not work, because they are send directly to GLWidget
	//QCoreApplication::instance()->installEventFilter( this );
}

osgQt::GLWidget* QOsgViewer::addViewWidget( osgQt::GraphicsWindowQt* gw )
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
	view_->addEventHandler( new QOsgEventHandler( *this ) );

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

	width_ = w;
	height_ = h;

	return new osgQt::GraphicsWindowQt( traits.get() );
}

void QOsgViewer::paintEvent( QPaintEvent* event )
{
	// prevent capturing duplicate frames
	if ( isCapturing() && current_frame_time_ == last_drawn_frame_time_ )
		return; // this frame was already captured, skip

	++frame_count_;

	// advance and handle camera events
	advance();
	eventTraversal();

	// update camera light position
	updateLightPos();

	// update and render
	updateTraversal();
	renderingTraversals();

	last_drawn_frame_time_ = current_frame_time_;
}

bool QOsgViewer::event( QEvent* e )
{
	if ( e->type() == QEvent::ToolTip )
		emit tooltip();
	return QWidget::event( e );
}

void QOsgViewer::setScene( osg::Group* s )
{
	scene_ = s;
	for ( size_t i = 0; i < getNumViews(); ++i )
		getView( i )->setSceneData( s );

	// init light
	scene_light_ = new osg::Light;
	scene_light_->setLightNum( 0 );
	scene_light_->setPosition( osg::Vec4f( scene_light_offset_.x, scene_light_offset_.y, scene_light_offset_.z, 1 ) );
	scene_light_->setDiffuse( osg::Vec4( 1, 1, 1, 1 ) );
	scene_light_->setSpecular( osg::Vec4( 1, 1, 1, 1 ) );
	scene_light_->setAmbient( osg::Vec4( 1, 1, 1, 1 ) );

	auto light_source = new osg::LightSource;
	light_source->setLight( scene_light_ );
	light_source->setLocalStateSetModes( osg::StateAttribute::ON );
	light_source->setReferenceFrame( osg::LightSource::RELATIVE_RF );

	// directly add the light to the parent node
	s->addChild( light_source );
	s->getOrCreateStateSet()->setMode( GL_LIGHT0, osg::StateAttribute::ON );
	scene_light_->setConstantAttenuation( 1.0f );
}

void QOsgViewer::createHud( const xo::path& file )
{
	hud_node_ = new osg::PositionAttitudeTransform;

	osg::ref_ptr<osg::Image> img = osgDB::readImageFile( file.str() );
	xo_error_if( !img.valid(), "Could not open " + file.str() );
	auto width = osg::Vec3f( hud_size, 0, 0 );
	auto height = osg::Vec3f( 0, hud_size, 0 );

	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage( img );
	texture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
	texture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );

	osg::ref_ptr<osg::Geometry> geom = osg::createTexturedQuadGeometry(
		width * -0.5f - height * 0.5f,
		width, height, 0, 0, 1.0f, 1.0f );

	geom->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
	geom->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
	geom->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	geode->addDrawable( geom );

	hud_node_->addChild( geode );

	hud_node_->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
	view_->getCamera()->addChild( hud_node_ );
	updateHudPos();
}

void QOsgViewer::updateHudPos()
{
	if ( hud_node_ )
	{
		double fovy, aspect, nearplane, farplane;
		view_->getCamera()->getProjectionMatrixAsPerspective( fovy, aspect, nearplane, farplane );
		auto hh = tan( xo::degreed( fovy / 2 ) );
		auto hw = tan( atan( hh * aspect ) );
		//xo::log::info( "Updating HUD position to ", hw - 0.55f * hud_size, ", ", -hh + 0.55f * hud_size, "; aspect ratio = ", aspect );
		hud_node_->setPosition( osg::Vec3( hw - 0.55f * hud_size, -hh + 0.55f * hud_size, -1 ) );
	}
}

void QOsgViewer::updateLightPos()
{
	auto center = camera_man_->getCenter();
	auto ori = xo::quat_from_axis_angle( xo::vec3f::unit_y(), camera_man_->getYaw() );
	auto v = ori * scene_light_offset_;
	auto p = center + osg::Vec3d( v.x, v.y, v.z );
	scene_light_->setPosition( osg::Vec4( p, 1 ) );
}

void QOsgViewer::viewerInit()
{
	updateHudPos();
}

void QOsgViewer::setClearColor( const osg::Vec4& col )
{
	view_->getCamera()->setClearColor( col );
}

void QOsgViewer::moveCamera( const osg::Vec3d& delta_pos )
{
	if ( !delta_pos.isNaN() )
		camera_man_->setCenter( camera_man_->getCenter() + delta_pos );
}

bool QOsgViewer::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
	switch ( ea.getEventType() )
	{
	case osgGA::GUIEventAdapter::RESIZE:
		width_ = ea.getWindowWidth();
		height_ = ea.getWindowHeight();
		xo::log::trace( "Viewer resized to ", width_, "x", height_ );
		updateHudPos();
		return true;
	case osgGA::GUIEventAdapter::PUSH:
		if ( ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON ) {
			updateIntersections( ea );
			emit pressed();
		}
		mouse_drag_count_ = 0;
		mouse_hover_allowed_ = false;
		break;
	case osgGA::GUIEventAdapter::MOVE:
		mouse_hover_timer_.restart();
		mouse_hover_allowed_ = true;
		break;
	case osgGA::GUIEventAdapter::DRAG:
		++mouse_drag_count_;
		mouse_hover_allowed_ = false;
		break;
	case osgGA::GUIEventAdapter::RELEASE:
		if ( mouse_drag_count_ <= 2 && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON ) {
			updateIntersections( ea );
			emit clicked();
			return true;
		}
		break;
	default:
		break;
	}

	if ( mouse_hover_allowed_ && mouse_hover_timer_() >= mouse_hover_duration_ ) {
		mouse_hover_allowed_ = false;
		updateIntersections( ea );
		emit hover();
	}

	return false;
}

void QOsgViewer::updateIntersections( const osgGA::GUIEventAdapter& ea )
{
	view_->computeIntersections( ea, intersections_ );
}

osg::Node* QOsgViewer::getTopNamedIntersectionNode( const std::string& skipName ) const
{
	for ( auto& is : intersections_ )
	{
		for ( auto it = is.nodePath.rbegin(); it != is.nodePath.rend(); it++ ) {
			const auto& name = ( *it )->getName();
			if ( name.empty() )
				continue;
			else if ( name != skipName )
				return *it;
			else break;
		}
	}
	return nullptr;
}

void QOsgViewer::enableObjectCache( bool enable )
{
	getOrCreateOptions()->setObjectCacheHint( enable ? osgDB::Options::CACHE_ALL : osgDB::Options::CACHE_ARCHIVES );
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
	last_drawn_frame_time_ = xo::constants<double>::lowest();
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
		new osgViewer::ScreenCaptureHandler::WriteToFile( filename, "png", osgViewer::ScreenCaptureHandler::WriteToFile::OVERWRITE ) );
	view_->addEventHandler( capture_handler_ );
	capture_handler_->startCapture();

	// schedule and handle paint event
	// NOTE: calling frame() directly messes up the event handler
	repaint();
	QApplication::processEvents();

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

void QOsgViewer::timerUpdate()
{
	// check if update is needed
	eventTraversal();
	camera_man_->handleKeyboardAnimation();
	if ( camera_man_->hasCameraStateChanged() )
		update();
}

bool QOsgViewer::eventFilter( QObject* obj, QEvent* event )
{
	if ( obj == view_widget_ )
	{
		// detect event inside view_widget_
		if ( event->type() == QEvent::MouseMove )
			update();
	}
	return QObject::eventFilter( obj, event );
}

osgDB::Options* QOsgViewer::getOrCreateOptions()
{
	if ( !osgDB::Registry::instance()->getOptions() )
		osgDB::Registry::instance()->setOptions( new osgDB::Options() );
	return osgDB::Registry::instance()->getOptions();
}
