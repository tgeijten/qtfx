#include "QCodeEditor.h"

#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QBoxLayout>
#include <QPainter>

#include "xo/system/system_tools.h"
#include "xo/system/assert.h"
#include "xo/string/string_tools.h"
#include "xo/system/log.h"
#include "xo/serialization/serialize.h"
#include "xo/numerical/math.h"
#include "xo/container/prop_node.h"
#include "QInputDialog"

QCodeEditor::QCodeEditor( QWidget* parent ) :
QPlainTextEdit( parent )
{
	QVBoxLayout* verticalLayout = new QVBoxLayout( this );
	verticalLayout->setContentsMargins( 0, 0, 0, 0 );
	setLayout( verticalLayout );

	QFont font;
	font.setFamily( QStringLiteral( "Consolas" ) );
	font.setPointSize( 9 );
	setFont( font );
	setLineWrapMode( QPlainTextEdit::NoWrap );
	setTabStopWidth( 16 );
	setWordWrapMode( QTextOption::NoWrap );
	//verticalLayout->addWidget( textEdit );

	setFrameStyle( QFrame::NoFrame );
	lineNumberArea = new LineNumberArea( this );

	connect( this, SIGNAL( blockCountChanged( int ) ), this, SLOT( updateLineNumberAreaWidth( int ) ) );
	connect( this, SIGNAL( updateRequest( QRect, int ) ), this, SLOT( updateLineNumberArea( QRect, int ) ) );

	updateLineNumberAreaWidth( 0 );

}

QCodeEditor::~QCodeEditor()
{}

void QCodeEditor::open( const QString& filename )
{
	syntaxHighlighter = new QCodeHighlighter( document(), QCodeHighlighter::detectLanguage( filename ) );

	QFile f( filename );
	if ( f.open( QFile::ReadOnly | QFile::Text ) )
	{
		QTextStream str( &f );
		QString data = str.readAll();
		setPlainText( data );
		fileName = filename;
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
		stream << toPlainText();
		stream.flush();
		file.close();
		document()->setModified( false );
	}
}

void QCodeEditor::saveAs( const QString& fn )
{
	if ( getFileFormat( fn ) != getFileFormat( fileName ) )
	{
		std::stringstream stri( toPlainText().toStdString() );
		xo::prop_node pn;
		stri >> *xo::make_serializer( getFileFormat( fileName ), pn );

		std::stringstream stro;
		stro << *xo::make_serializer( getFileFormat( fn ), pn );

		syntaxHighlighter = new QCodeHighlighter( document(), QCodeHighlighter::detectLanguage( fn ) );
		setPlainText( QString( stro.str().c_str() ) );
	}

	fileName = fn;
	save();
}

void QCodeEditor::findDialog()
{
	QString text = QInputDialog::getText( this, "Find Text", "Text to find:", QLineEdit::Normal, findText );
	if ( !text.isNull() )
	{
		findText = text;
		moveCursor( QTextCursor::Start );
		if ( !findNext() )
			QMessageBox::warning( this, "Could not find text", "Could not find '" + findText + "'" );
	}
}

bool QCodeEditor::findNext( bool backwards )
{
	auto cursor = document()->find( findText, textCursor(), backwards ? QTextDocument::FindBackward : QTextDocument::FindFlags() );
	if ( !cursor.isNull() )
		setTextCursor( cursor );
	return !cursor.isNull();
}

QString QCodeEditor::getTitle()
{
	return QFileInfo( fileName ).fileName() + ( document()->isModified() ? "*" : "" );
}

std::string QCodeEditor::getFileFormat( const QString& filename ) const
{
	return xo::path( filename.toStdString() ).extension().str();
}

void QCodeEditor::lineNumberAreaPaintEvent( QPaintEvent *event )
{
	QPainter painter( lineNumberArea );
	QColor c = palette().color( QWidget::backgroundRole() );
	painter.fillRect( event->rect(), c );

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int)blockBoundingGeometry( block ).translated( contentOffset() ).top();
	int bottom = top + (int)blockBoundingRect( block ).height();

	while ( block.isValid() && top <= event->rect().bottom() ) {
		if ( block.isVisible() && bottom >= event->rect().top() ) {
			QString number = QString::number( blockNumber + 1 );
			painter.setPen( Qt::gray );
			painter.drawText( 0, top, lineNumberArea->width() - 2, fontMetrics().height(),
				Qt::AlignRight, number );
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect( block ).height();
		++blockNumber;
	}
}

int QCodeEditor::lineNumberAreaWidth()
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

void QCodeEditor::formatDocument()
{
	auto cursor = textCursor();
	cursor.beginEditBlock();
	cursor.movePosition( QTextCursor::Start );

	auto indents = 0;
	while ( !cursor.atEnd() )
	{
		QString line = cursor.block().text();

		// count current amount of tabs
		auto tab_count = 0;
		while ( tab_count < line.size() && line[ tab_count ] == '\t' )
			++tab_count;

		auto leading_whitespace = tab_count;
		while ( leading_whitespace < line.size() && line[ leading_whitespace ].isSpace() )
			++leading_whitespace;

		QChar first_char = tab_count < line.size() ? line[ tab_count ] : QChar();
		auto desired_tabs = xo::max( 0, indents - int( first_char == '}' || first_char == ']' ) );

		if ( leading_whitespace != desired_tabs )
		{
			cursor.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor, leading_whitespace );
			cursor.removeSelectedText();
			cursor.insertText( QString().fill( '\t', desired_tabs ) );
		}

		// update indents
		indents += line.count( '{' ) + line.count( '[' ) - line.count( '}' ) - line.count( ']' );

		if ( !cursor.movePosition( QTextCursor::NextBlock ) )
			break; // prevent infinite loop if this fails
	}
	cursor.endEditBlock();
}

void QCodeEditor::updateLineNumberAreaWidth( int newBlockCount )
{
	setViewportMargins( lineNumberAreaWidth(), 0, 0, 0 );
}

void QCodeEditor::updateLineNumberArea( const QRect& rect, int dy )
{
	if ( dy )
		lineNumberArea->scroll( 0, dy );
	else
		lineNumberArea->update( 0, rect.y(), lineNumberArea->width(), rect.height() );

	if ( rect.contains( viewport()->rect() ) )
		updateLineNumberAreaWidth( 0 );
}

void QCodeEditor::resizeEvent( QResizeEvent *event )
{
	QRect cr = contentsRect();
	if ( cr != previousRect ) // this is a hack to prevent a Qt bug causing infinite QResizeEvents
	{
		previousRect = cr;
		QPlainTextEdit::resizeEvent( event );
		lineNumberArea->setGeometry( QRect( cr.left(), cr.top(), lineNumberAreaWidth(), cr.height() ) );
	}
}

void QCodeEditor::keyPressEvent( QKeyEvent *e )
{
	QPlainTextEdit::keyPressEvent( e );

	switch ( language() )
	{
	case QCodeHighlighter::Language::zml:
		if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter || e->key() == '{' || e->key() == '}' )
			formatDocument();
	default:
		break;
	}
}
