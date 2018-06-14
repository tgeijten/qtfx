#include "QCodeHighlighter.h"

#include "xo/system/assert.h"
#include "xo/filesystem/path.h"
#include "xo/numerical/math.h"

QCodeHighlighter::QCodeHighlighter( QObject* parent, Language l ) : QSyntaxHighlighter( parent )
{
	setLanguage( l );
}

QCodeHighlighter::QCodeHighlighter( QTextDocument* parent, Language l ) : QSyntaxHighlighter( parent )
{
	setLanguage( l );
}

void QCodeHighlighter::highlightBlock( const QString &text )
{
	if ( language == XML )
	{
		// Special treatment for xml element regex as we use captured text to emulate lookbehind
		auto m = match( m_xmlElementRegex, text, 0 );
		while ( m.hasMatch() )
		{
			setFormat( m.capturedStart( 1 ), m.capturedLength( 1 ), m_ElementFormat );
			m = match( m_xmlElementRegex, text, m.capturedEnd( 1 ) );
		}
		highlightByRegex( m_NumberFormat, m_NumberRegex, text );
		highlightByRegex( m_AttributeFormat, m_AttributeRegex, text );
	}
	else
	{
		highlightByRegex( m_NumberFormat, m_NumberRegex, text );
		highlightByRegex( m_AttributeFormat, m_AttributeRegex, text );
		highlightByRegex( m_ElementFormat, m_xmlElementRegex, text );
	}

	// Highlight xml keywords *after* xml elements to fix any occasional / captured into the enclosing element
	highlightByRegex( m_OperatorFormat, m_OperatorRegex, text );
	highlightByRegex( m_ValueFormat, m_StringRegex, text );
	highlightByRegex( m_SpecialFormat, m_SpecialRegex, text );
	highlightByRegex( m_CommentFormat, m_CommentRegex, text );

	// finally, apply multi-line comments
	setCurrentBlockState( 0 );
	int startIndex = previousBlockState() != 1 ? match( commentStartRegex, text, 0 ).capturedStart() : 0;
	while ( startIndex >= 0 )
	{
		QRegularExpressionMatch mend = match( commentEndRegex, text, startIndex );
		if ( !mend.hasMatch() )
		{
			setCurrentBlockState( 1 );
			setFormat( startIndex, text.length() - startIndex, m_CommentFormat );
			break;
		}
		else
		{
			setFormat( startIndex, mend.capturedEnd() - startIndex, m_CommentFormat );
			startIndex = match( commentStartRegex, text, mend.capturedEnd() ).capturedStart();
		}
	}
}

bool QCodeHighlighter::isBetweenQuotes( const QString & text, int index )
{
	int quotes_before = 0;
	for ( int i = 0; i < index; ++i )
		quotes_before += int( text[ i ] == '\"' );
	return xo::is_odd( quotes_before );
}

QRegularExpressionMatch QCodeHighlighter::match( const QRegularExpression & regex, const QString & text, int index )
{
	auto m = regex.match( text, index );
	while ( m.hasMatch() && isBetweenQuotes( text, m.capturedStart() ) )
		m = regex.match( text, m.capturedEnd() );
	return m;
}

void QCodeHighlighter::highlightByRegex( const QTextCharFormat & format, const QRegularExpression& regex, const QString & text )
{
	auto m = match( regex, text, 0 );
	while ( m.hasMatch() )
	{
		setFormat( m.capturedStart(), m.capturedLength(), format );
		m = match( regex, text, m.capturedEnd() );
	}
}

void QCodeHighlighter::setRegexes()
{
	switch ( language )
	{
	case XML:
		// TODO: convert to rules
		m_xmlElementRegex.setPattern( "<[\\s]*[/]?[\\s]*([^\\n]\\w*)(?=[\\s/>])" );
		m_AttributeRegex.setPattern( "\\w+(?=\\=)" );
		m_StringRegex.setPattern( "\"[^\\n\"]+\"(?=[\\s/>])" );
		m_CommentRegex.setPattern( "<!--[^\\n]*-->" );
		m_SpecialRegex.setPattern( "/.^/" );
		m_OperatorRegex.setPattern( "(<\\?|/>|>|<|</|\\?>" );
		commentStartRegex.setPattern( "<!--" );
		commentEndRegex.setPattern( "-->" );
		break;

	case ZML:
		// TODO: convert to rules
		//rules.emplace_back( "\\w+\\s*\\=?\\s*[\\{\\[]", m_ElementFormat );
		m_xmlElementRegex.setPattern( "\\w+\\s*\\=?\\s*[\\{\\[]" );
		m_AttributeRegex.setPattern( "\\w+\\s*(\\=)" );
		m_StringRegex.setPattern( "\"[^\\n\"]*\"" );
		m_CommentRegex.setPattern( "(;|//)[^\\n]*" );
		m_SpecialRegex.setPattern( "#\\w+" );
		m_OperatorRegex.setPattern( "[\\{\\}\\[\\]\\=]" );
		commentStartRegex.setPattern( "/\\*" );
		commentEndRegex.setPattern( "\\*/" );
		break;

	default:
		xo_error( "Unsupported language" );
	}

	m_NumberRegex.setPattern( "\\b([-+]?[\\.\\d]+)" );
}

void QCodeHighlighter::setFormats()
{
	m_OperatorFormat.setForeground( Qt::darkGray );
	m_ElementFormat.setForeground( Qt::darkBlue );
	m_ElementFormat.setFontWeight( QFont::Bold );
	m_AttributeFormat.setForeground( Qt::darkBlue );
	//m_AttributeFormat.setFontItalic( true );
	m_ValueFormat.setForeground( Qt::darkRed );
	m_CommentFormat.setForeground( Qt::darkGreen );
	m_CommentFormat.setFontItalic( true );
	m_NumberFormat.setForeground( Qt::darkCyan );
	m_SpecialFormat.setForeground( Qt::blue );
	m_SpecialFormat.setFontItalic( true );
	m_SpecialFormat.setFontWeight( QFont::Bold );
}

void QCodeHighlighter::setLanguage( Language l )
{
	language = l;
	setRegexes();
	setFormats();
}

QCodeHighlighter::Language QCodeHighlighter::detectLanguage( const QString& filename )
{
	auto ext = xo::path( filename.toStdString() ).extension();
	if ( ext == "xml" )
		return XML;
	else if ( ext == "zml" )
		return ZML;
	else return ZML;
}
