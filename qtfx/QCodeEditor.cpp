#include "QCodeEditor.h"

#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QBoxLayout>
#include <QPainter>
#include <QTextDocumentFragment>
#include <QInputDialog>

#include "xo/system/system_tools.h"
#include "xo/system/assert.h"
#include "xo/string/string_tools.h"
#include "xo/system/log.h"
#include "xo/serialization/serialize.h"
#include "xo/numerical/math.h"
#include "xo/container/prop_node.h"
#include "qt_convert.h"
#include "qtfx.h"
#include <sstream>

QCodeEditor::QCodeEditor( QWidget* parent ) :
	QPlainTextEdit( parent )
{
	QVBoxLayout* verticalLayout = new QVBoxLayout( this );
	verticalLayout->setContentsMargins( 0, 0, 0, 0 );
	setLayout( verticalLayout );

	auto font = getMonospaceFont( 9 );
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
	triggerFormatKeys = { Qt::Key_Return, Qt::Key_Enter }; // #todo: set based on language
	autoBrackets = { { '{', '}' }, { '[', ']' }, { '(', ')' }, { '"', '"' } }; // #todo: set based on language

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

void QCodeEditor::save()
{
	QFile file( fileName );
	if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
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
	auto new_fmt = path_from_qt( fn ).extension_no_dot().str();
	auto old_fmt = path_from_qt( fileName ).extension_no_dot().str();
	if ( new_fmt != old_fmt )
	{
		std::stringstream stri( toPlainText().toStdString() );
		xo::prop_node pn;
		stri >> *xo::make_serializer( old_fmt, pn );

		std::stringstream stro;
		stro << *xo::make_serializer( new_fmt, pn );

		syntaxHighlighter = new QCodeHighlighter( document(), QCodeHighlighter::detectLanguage( fn ) );
		setPlainText( QString( stro.str().c_str() ) );
	}

	fileName = fn;
	save();
}

void QCodeEditor::reload()
{
	if ( document()->isModified() )
	{
		QString msg = fileName + " contains unsaved changes. These changes will be lost when reloading the document.";
		if ( QMessageBox::Cancel == QMessageBox::warning( this, "Reload " + fileName, msg, QMessageBox::Discard | QMessageBox::Cancel ) )
			return;
	}

	if ( QFile f( fileName ); f.open( QFile::ReadOnly | QFile::Text ) )
	{
		setPlainText( QTextStream( &f ).readAll() );
		document()->setModified( false );
	}
}

void QCodeEditor::findDialog()
{
	QString text = QInputDialog::getText( this, "Enter Text to Find", "Hint: use F3 for Find Next, Shift + F3 for Find Previous", QLineEdit::Normal, findText );
	if ( !text.isNull() )
	{
		findText = text;
		if ( !findNext() )
		{
			moveCursor( QTextCursor::Start );
			if ( !findNext() )
				QMessageBox::warning( this, "Could not find text", "Could not find '" + findText + "'" );
		}
	}
}

bool QCodeEditor::findNext( bool backwards )
{
	auto cursor = document()->find( findText, textCursor(), backwards ? QTextDocument::FindBackward : QTextDocument::FindFlags() );
	if ( !cursor.isNull() )
		setTextCursor( cursor );
	return !cursor.isNull();
}

void QCodeEditor::toggleComments()
{
	if ( auto cursor = textCursor(); !cursor.hasSelection() )
	{
		cursor.select( QTextCursor::LineUnderCursor );
		setTextCursor( cursor );
	}

	auto s = textCursor().selection().toPlainText().toStdString();
	auto endsWithNewLine = xo::str_ends_with( s, '\n' );
	auto lines = xo::split_str( s, "\n" );
	auto first_line = xo::find_if( lines, []( const auto& s ) { return s.find_first_not_of( "\t " ) != std::string::npos; } );
	if ( first_line != lines.end() )
	{
		s.clear();
		auto comment = syntaxHighlighter->languageComment.toStdString();
		if ( xo::str_begins_with( xo::trim_left_str( *first_line ), comment ) )
		{
			// remove comments
			for ( auto& l : lines )
			{
				auto pos = l.find_first_not_of( "\t " );
				if ( pos != std::string::npos && xo::str_begins_with( l, comment, pos ) )
					s += xo::left_str( l, pos ) + xo::mid_str( l, pos + comment.size() ) + '\n';
				else s += l + '\n';
			}
		}
		else {
			// add comments
			for ( const auto& l : lines )
			{
				if ( auto pos = l.find_first_not_of( "\t " ); pos != std::string::npos )
					s += xo::left_str( l, pos ) + comment + xo::mid_str( l, pos ) + '\n';
				else s += l + '\n';
			}
		}
		if ( !endsWithNewLine )
			s.resize( s.size() - 1 );
		textCursor().insertText( s.c_str() );
	}
}

void QCodeEditor::duplicateText()
{
	if ( auto cursor = textCursor(); !cursor.hasSelection() )
	{
		cursor.select( QTextCursor::BlockUnderCursor );
		setTextCursor( cursor );
	}
	copy();
	paste();
	paste();
}

QString QCodeEditor::getTitle()
{
	return QFileInfo( fileName ).fileName() + ( document()->isModified() ? "*" : "" );
}

void QCodeEditor::lineNumberAreaPaintEvent( QPaintEvent* event )
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

		auto leadingWhitespace = tab_count;
		while ( leadingWhitespace < line.size() && line[ leadingWhitespace ].isSpace() )
			++leadingWhitespace;

		auto decStart = line.indexOf( syntaxHighlighter->decreaseIndentRegex );
		bool startsWithDecrease = decStart >= 0 && decStart <= leadingWhitespace;
		auto desired_tabs = indents > 0 && startsWithDecrease ? indents - 1 : indents;

		if ( leadingWhitespace != desired_tabs )
		{
			cursor.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor, leadingWhitespace );
			cursor.removeSelectedText();
			cursor.insertText( QString().fill( '\t', desired_tabs ) );
		}

		// update indents
		auto incIndent = line.count( syntaxHighlighter->increaseIndentRegex );
		auto decIndent = line.count( syntaxHighlighter->decreaseIndentRegex );
		indents += incIndent - decIndent;

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

void QCodeEditor::resizeEvent( QResizeEvent* event )
{
	QRect cr = contentsRect();
	if ( cr != previousRect ) // this is a hack to prevent a Qt bug causing infinite QResizeEvents
	{
		previousRect = cr;
		QPlainTextEdit::resizeEvent( event );
		lineNumberArea->setGeometry( QRect( cr.left(), cr.top(), lineNumberAreaWidth(), cr.height() ) );
	}
}

void QCodeEditor::keyPressEvent( QKeyEvent* e )
{
	QPlainTextEdit::keyPressEvent( e );

	// insert closing bracket
	for ( const auto& bp : autoBrackets )
	{
		if ( e->key() == bp.first )
		{
			textCursor().insertText( QString( bp.second ) );
			moveCursor( QTextCursor::Left );
			break;
		}
	}

	if ( triggerFormatKeys.contains( e->key() ) )
		formatDocument();
}
