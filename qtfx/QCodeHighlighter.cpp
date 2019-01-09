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
	for ( auto& r : rules )
		applyRule( text, r );

	// apply multi-line comments
	setCurrentBlockState( 0 );
	int startIndex = previousBlockState() != 1 ? match( commentStartRegex, text, 0 ).capturedStart() : 0;
	while ( startIndex >= 0 )
	{
		QRegularExpressionMatch mend = match( commentEndRegex, text, startIndex );
		if ( !mend.hasMatch() )
		{
			setCurrentBlockState( 1 );
			setFormat( startIndex, text.length() - startIndex, commentFormat );
			break;
		}
		else
		{
			setFormat( startIndex, mend.capturedEnd() - startIndex, commentFormat );
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

void QCodeHighlighter::applyRule( const QString& text, const HighlightRule& r )
{
	for ( auto m = match( r.regExp, text, 0 ); m.hasMatch(); m = match( r.regExp, text, m.capturedEnd() ) )
		setFormat( m.capturedStart(), m.capturedLength(), r.format );
}

void QCodeHighlighter::setRegexes()
{
	switch ( language )
	{
	case XML:
		rules.emplace_back( "<[\\s]*[/]?[\\s]*([^\\n]\\w*)(?=[\\s/>])", elementFormat );
		rules.emplace_back( "\\w+(?=\\=)", attributeFormat );
		rules.emplace_back( "\"[^\\n\"]+\"(?=[\\s/>])", valueFormat );
		rules.emplace_back( "/.^/", specialFormat );
		rules.emplace_back( "(<\\?|/>|>|<|</|\\?>", operatorFormat );
		rules.emplace_back( "\\b([-+]?[\\.\\d]+)", numberFormat );
		commentStartRegex.setPattern( "<!--" );
		commentEndRegex.setPattern( "-->" );
		break;

	case ZML:
		rules.emplace_back( "\\w+\\s*\\=?\\s*[\\{\\[]", elementFormat );
		rules.emplace_back( "\\w+\\s*(\\=)", attributeFormat ); // key = value
		rules.emplace_back( "\\w+(:\\s+)", attributeFormat ); // key: value
		rules.emplace_back( "\"[^\\n\"]*\"", valueFormat );
		rules.emplace_back( "#\\w+", specialFormat );
		rules.emplace_back( "<--\\s", macroFormat );
		rules.emplace_back( "([\\{\\}\\[\\]\\=]|:\\s)", operatorFormat );
		rules.emplace_back( "\\b([-+]?[\\.\\d]+)", numberFormat );
		rules.emplace_back( "\\@\\w+", macroFormat );
		rules.emplace_back( "(;|//|#\\s)[^\\n]*", commentFormat ); // ; // #
		commentStartRegex.setPattern( "/\\*" );
		commentEndRegex.setPattern( "\\*/" );
		break;

	default:
		xo_error( "Unsupported language" );
	}
}

void QCodeHighlighter::setFormats()
{
	operatorFormat.setForeground( Qt::darkGray );
	elementFormat.setForeground( Qt::darkBlue );
	elementFormat.setFontWeight( QFont::Bold );
	attributeFormat.setForeground( Qt::darkBlue );
	//attributeFormat.setFontItalic( true );
	valueFormat.setForeground( Qt::darkRed );
	commentFormat.setForeground( Qt::darkGreen );
	commentFormat.setFontItalic( true );
	numberFormat.setForeground( Qt::darkCyan );
	macroFormat.setForeground( Qt::darkMagenta );
	specialFormat.setForeground( Qt::blue );
	specialFormat.setFontItalic( true );
	specialFormat.setFontWeight( QFont::Bold );
}

void QCodeHighlighter::setLanguage( Language l )
{
	language = l;
	setFormats();
	setRegexes();
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
