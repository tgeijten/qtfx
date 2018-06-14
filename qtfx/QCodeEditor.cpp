#include "QCodeEditor.h"

#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QBoxLayout>

#include "xo/system/system_tools.h"
#include "xo/system/assert.h"
#include "xo/string/string_tools.h"
#include "xo/system/log.h"
#include "xo/serialization/serialize.h"
#include "xo/numerical/math.h"

QCodeEditor::QCodeEditor( QWidget* parent ) :
QWidget( parent ),
textChangedFlag( false )
{
	QVBoxLayout* verticalLayout = new QVBoxLayout( this );
	verticalLayout->setContentsMargins( 0, 0, 0, 0 );
	setLayout( verticalLayout );
	textEdit = new QCodeTextEdit( this );

	QFont font;
	font.setFamily( QStringLiteral( "Consolas" ) );
	font.setPointSize( 9 );
	textEdit->setFont( font );
	textEdit->setLineWrapMode( QPlainTextEdit::NoWrap );
	textEdit->setTabStopWidth( 16 );
	textEdit->setWordWrapMode( QTextOption::NoWrap );
	verticalLayout->addWidget( textEdit );

	connect( textEdit, SIGNAL( textChanged() ), this, SLOT( textEditChanged() ) );
}

QCodeEditor::~QCodeEditor()
{}

QString QCodeEditor::getPlainText() const
{
	return textEdit->toPlainText();
}

void QCodeEditor::open( const QString& filename )
{
	QCodeHighlighter* xmlSyntaxHighlighter = new QCodeHighlighter( textEdit->document(), QCodeHighlighter::detectLanguage( filename ) );

	QFile f( filename );
	if ( f.open( QFile::ReadOnly | QFile::Text ) )
	{
		QTextStream str( &f );
		QString data = str.readAll();
		textEdit->setPlainText( data );
		fileName = filename;
		textChangedFlag = false;
	}
	else xo_error( "Could not open " + filename.toStdString() );
}

void QCodeEditor::openDialog( const QString& folder, const QString& fileTypes )
{
	auto fn = QFileDialog::getOpenFileName( this, "Open File", folder, fileTypes );
	if ( !fn.isEmpty() )
		open( fn );
}

void QCodeEditor::save()
{
	QFile file( fileName );
	if ( !file.open( QIODevice::WriteOnly ) )
	{
		QMessageBox::critical( this, "Error writing file", "Could not open file " + fileName );
		return;
	}
	else
	{
		QTextStream stream( &file );
		stream << textEdit->toPlainText();
		stream.flush();
		file.close();
		textChangedFlag = false;
	}
}

void QCodeEditor::saveAs( const QString& fn )
{
	if ( getFileFormat( fn ) != getFileFormat( fileName ) )
	{
		std::stringstream stri( textEdit->toPlainText().toStdString() );
		xo::prop_node pn = xo::make_serializer( getFileFormat( fileName ) )->read_stream( stri );
		std::stringstream stro;
		xo::make_serializer( getFileFormat( fn ) )->write_stream( stro, pn );

		QCodeHighlighter* xmlSyntaxHighlighter = new QCodeHighlighter( textEdit->document(), QCodeHighlighter::detectLanguage( fn ) );
		textEdit->setPlainText( QString( stro.str().c_str() ) );
	}

	fileName = fn;
	save();
}

QString QCodeEditor::getTitle()
{
	return QFileInfo( fileName ).fileName() + ( hasTextChanged() ? "*" : "" );
}

void QCodeEditor::textEditChanged()
{
	if ( !textChangedFlag )
	{
		textChangedFlag = true;
		emit textChanged();
	}
}

std::string QCodeEditor::getFileFormat( const QString& filename ) const
{
	return xo::path( filename.toStdString() ).extension().string();
}

//
// QCodeTextEdit
//

QCodeTextEdit::QCodeTextEdit( QWidget* parent ) : QPlainTextEdit( parent )
{
	lineNumberArea = new LineNumberArea( this );

	connect( this, SIGNAL( blockCountChanged( int ) ), this, SLOT( updateLineNumberAreaWidth( int ) ) );
	connect( this, SIGNAL( updateRequest( QRect, int ) ), this, SLOT( updateLineNumberArea( QRect, int ) ) );

	updateLineNumberAreaWidth( 0 );
}

void QCodeTextEdit::lineNumberAreaPaintEvent( QPaintEvent *event )
{
	QPainter painter( lineNumberArea );
	painter.fillRect( event->rect(), Qt::lightGray );

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry( block ).translated( contentOffset() ).top();
	int bottom = top + (int)blockBoundingRect( block ).height();

	while ( block.isValid() && top <= event->rect().bottom() ) {
		if ( block.isVisible() && bottom >= event->rect().top() ) {
			QString number = QString::number( blockNumber + 1 );
			painter.setPen( Qt::black );
			painter.drawText( 0, top, lineNumberArea->width() - 2, fontMetrics().height(),
				Qt::AlignRight, number );
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect( block ).height();
		++blockNumber;
	}
}

int QCodeTextEdit::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax( 1, blockCount() );
	while ( max >= 10 ) {
		max /= 10;
		++digits;
	}

	int space = 4 + fontMetrics().width( QLatin1Char( '9' ) ) * digits;

	return space;
}

void QCodeTextEdit::updateLineNumberAreaWidth( int newBlockCount )
{
	setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 );
}

void QCodeTextEdit::updateLineNumberArea( const QRect& rect, int dy )
{
	if ( dy )
		lineNumberArea->scroll( 0, dy );
	else
		lineNumberArea->update( 0, rect.y(), lineNumberArea->width(), rect.height() );

	if ( rect.contains( viewport()->rect() ) )
		updateLineNumberAreaWidth( 0 );
}

void QCodeTextEdit::resizeEvent( QResizeEvent *event )
{
	QPlainTextEdit::resizeEvent( event );
	QRect cr = contentsRect();
	lineNumberArea->setGeometry( QRect( cr.left(), cr.top(), lineNumberAreaWidth(), cr.height() ) );
}

void QCodeTextEdit::keyPressEvent( QKeyEvent *e )
{
	if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
	{
		auto line = textCursor().block().text().toStdString();
		int tabs = 0;
		while ( tabs < line.size() && line[ tabs ] == '\t' )
			++tabs;
		QPlainTextEdit::keyPressEvent( e );
		QPlainTextEdit::insertPlainText( QString( tabs, '\t' ) );
	}
	else QPlainTextEdit::keyPressEvent( e );
}
