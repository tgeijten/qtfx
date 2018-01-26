#include "QCodeEditor.h"
#include "xo/system/system_tools.h"
#include "xo/system/assert.h"
#include "xo/string/string_tools.h"
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>

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
	QCodeSyntaxHighlighter* xmlSyntaxHighlighter = new QCodeSyntaxHighlighter( textEdit->document(), QCodeSyntaxHighlighter::detectLanguage( filename ) );

	QFile f( filename );
	if ( f.open( QFile::ReadOnly | QFile::Text ) )
	{
		QTextStream str( &f );
		QString data = str.readAll();
		textEdit->setPlainText( data );
		fileName = filename;
		textChangedFlag = false;
	}
	else xo_error( "Could not open file: " + filename.toStdString() );
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
		xo::prop_node pn;
		stri >> xo::prop_node_deserializer( getFileFormat( fileName ), pn );
		std::stringstream stro;
		stro << xo::prop_node_serializer( getFileFormat( fn ), pn );

		QCodeSyntaxHighlighter* xmlSyntaxHighlighter = new QCodeSyntaxHighlighter( textEdit->document(), QCodeSyntaxHighlighter::detectLanguage( fn ) );
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

xo::file_format QCodeEditor::getFileFormat( const QString& filename ) const
{
	auto ext = xo::path( filename.toStdString() ).extension();
	if ( ext == "xml" )
		return xo::file_format::xml;
	else if ( ext == "zml" )
		return xo::file_format::zml;
	else return xo::file_format::unknown;
}

//
// BasicXMLSyntaxHighlighter
//

QCodeSyntaxHighlighter::QCodeSyntaxHighlighter( QObject* parent, Language l ) : QSyntaxHighlighter( parent )
{
	setLanguage( l );
}

QCodeSyntaxHighlighter::QCodeSyntaxHighlighter( QTextDocument* parent, Language l ) : QSyntaxHighlighter( parent )
{
	setLanguage( l );
}

void QCodeSyntaxHighlighter::highlightBlock( const QString &text )
{
	if ( language == XML )
	{
		// Special treatment for xml element regex as we use captured text to emulate lookbehind
		int xmlElementIndex = m_xmlElementRegex.indexIn( text );
		while ( xmlElementIndex >= 0 )
		{
			int matchedPos = m_xmlElementRegex.pos( 1 );
			int matchedLength = m_xmlElementRegex.cap( 1 ).length();
			setFormat( matchedPos, matchedLength, m_ElementFormat );
			xmlElementIndex = m_xmlElementRegex.indexIn( text, matchedPos + matchedLength );
		}
	}
	else
	{
		highlightByRegex( m_ElementFormat, m_xmlElementRegex, text );
	}

	highlightByRegex( m_NumberFormat, m_NumberRegex, text );
	highlightByRegex( m_AttributeFormat, m_xmlAttributeRegex, text );
	highlightByRegex( m_ValueFormat, m_xmlValueRegex, text );

	// Highlight xml keywords *after* xml elements to fix any occasional / captured into the enclosing element
	typedef QList<QRegExp>::const_iterator Iter;
	Iter xmlKeywordRegexesEnd = m_xmlKeywordRegexes.end();
	for ( Iter it = m_xmlKeywordRegexes.begin(); it != xmlKeywordRegexesEnd; ++it ) {
		const QRegExp & regex = *it;
		highlightByRegex( m_KeywordFormat, regex, text );
	}

	if ( language == ZML )
	{
		int i = m_xmlCommentRegex.indexIn( text );
		while  ( i >= 0 )
		{
			int quotes_before = 0;
			for ( int x = 0; x <= i; ++x )
				quotes_before += int( text[ x ] == '\"' );
			if ( quotes_before % 2 == 0 )
			{
				setFormat( i, m_xmlCommentRegex.matchedLength(), m_CommentFormat );
				break;
			}
			else i = m_xmlCommentRegex.indexIn( text, i + 1 );
		}
	}
	else 
	{
		highlightByRegex( m_CommentFormat, m_xmlCommentRegex, text );
	}
}

void QCodeSyntaxHighlighter::highlightByRegex( const QTextCharFormat & format, const QRegExp & regex, const QString & text )
{
	int index = regex.indexIn( text );
	while ( index >= 0 )
	{
		int matchedLength = regex.matchedLength();
		setFormat( index, matchedLength, format );
		index = regex.indexIn( text, index + matchedLength );
	}
}

void QCodeSyntaxHighlighter::setRegexes()
{
	switch ( language )
	{
	case XML:
		m_xmlElementRegex.setPattern( "<[\\s]*[/]?[\\s]*([^\\n]\\w*)(?=[\\s/>])" );
		m_xmlAttributeRegex.setPattern( "\\w+(?=\\=)" );
		m_xmlValueRegex.setPattern( "\"[^\\n\"]+\"(?=[\\s/>])" );
		m_xmlCommentRegex.setPattern( "<!--[^\\n]*-->" );
		m_xmlKeywordRegexes = QList<QRegExp>() << QRegExp( "<\\?" ) << QRegExp( "/>" ) << QRegExp( ">" ) << QRegExp( "<" ) << QRegExp( "</" ) << QRegExp( "\\?>" );
		break;
	case ZML:
		m_xmlElementRegex.setPattern( "\\w+\\s*\\{" );
		m_xmlAttributeRegex.setPattern( "\\w+\\s*(\\=)" );
		m_xmlValueRegex.setPattern( "\"[^\\n\"]+\"" );
		m_xmlCommentRegex.setPattern( ";[^\\n]*" );
		m_xmlKeywordRegexes = QList<QRegExp>() << QRegExp( "\\{" ) << QRegExp( "\\}" ) << QRegExp( "\\=" ) << QRegExp( "#\\w+" );
		break;
	default:
		xo_error( "Unsupported language" );
	}

	m_NumberRegex.setPattern( "\\b([-+]?[\\.\\d]+)" );
}

void QCodeSyntaxHighlighter::setFormats()
{
	m_KeywordFormat.setForeground( Qt::blue );
	m_ElementFormat.setForeground( Qt::darkBlue );
	m_ElementFormat.setFontWeight( QFont::Bold );
	m_AttributeFormat.setForeground( Qt::darkBlue );
	//m_AttributeFormat.setFontWeight( QFont::Bold );
	m_ValueFormat.setForeground( Qt::darkRed );
	m_CommentFormat.setForeground( Qt::darkGreen );
	m_CommentFormat.setFontItalic( true );
	m_NumberFormat.setForeground( Qt::darkMagenta );
}

void QCodeSyntaxHighlighter::setLanguage( Language l )
{
	language = l;
	setRegexes();
	setFormats();
}

QCodeSyntaxHighlighter::Language QCodeSyntaxHighlighter::detectLanguage( const QString& filename )
{
	auto ext = xo::path( filename.toStdString() ).extension();
	if ( ext == "xml" )
		return XML;
	else if ( ext == "zml" )
		return ZML;
	else return XML;
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
