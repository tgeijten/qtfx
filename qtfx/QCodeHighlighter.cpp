#include "QCodeHighlighter.h"

#include "xo/system/assert.h"
#include "xo/filesystem/path.h"
#include "xo/numerical/math.h"
#include "xo/string/string_tools.h"
#include "xo/utility/hash.h"
#include "xo/container/flat_map.h"
#include <string>

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
	if ( !commentStartRegex.pattern().isEmpty() )
	{
		int commentStart = 0, searchIndex = 0;
		bool isComment = previousBlockState() == 1;

		while ( searchIndex >= 0 )
		{
			if ( isComment )
			{
				auto matchEnd = match( commentEndRegex, text, searchIndex );
				if ( !matchEnd.hasMatch() )
				{
					// multiline comment goes beyond this line
					setFormat( commentStart, text.length() - commentStart, commentFormat );
					break;
				}
				else
				{
					// multiline comment ends here
					setFormat( commentStart, matchEnd.capturedEnd() - commentStart, commentFormat );
					searchIndex = matchEnd.capturedEnd();
					isComment = false;
				}
			}
			if ( !isComment )
			{
				auto matchStart = match( commentStartRegex, text, searchIndex );
				if ( matchStart.hasMatch() )
				{
					// multiline comment starts here
					commentStart = matchStart.capturedStart();
					searchIndex = matchStart.capturedEnd();
					isComment = true;
				}
				else break; // no more comments in this line
			}
		}
		setCurrentBlockState( isComment ? 1 : 0 );
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
	rules.clear();
	commentStartRegex.setPattern( "" );
	commentEndRegex.setPattern( "" );

	switch ( language )
	{
	case Language::xml:
		rules.emplace_back( "<[\\s]*[/]?[\\s]*([^\\n]\\w*)(?=[\\s/>])", elementFormat );
		rules.emplace_back( "\\w+(?=\\=)", attributeFormat );
		rules.emplace_back( "\"[^\\n\"]+\"(?=[\\s/>])", valueFormat );
		rules.emplace_back( "/.^/", specialFormat );
		rules.emplace_back( "(<\\?|/>|>|<|</|\\?>", operatorFormat );
		rules.emplace_back( "\\b([-+]?[\\.\\d]+)", numberFormat );
		commentStartRegex.setPattern( "<!--" );
		commentEndRegex.setPattern( "-->" );
		break;

	case Language::zml:
		rules.emplace_back( "\\w+\\s*\\=?\\s*[\\{\\[]", elementFormat );
		rules.emplace_back( "\\w+\\s*(\\=)", attributeFormat ); // key = value
		rules.emplace_back( "\\w+(:\\s+)", attributeFormat ); // key: value
		rules.emplace_back( "\"[^\\n\"]*\"", valueFormat );
		rules.emplace_back( "<<\\s.*\\s>>", specialFormat );
		rules.emplace_back( "([\\{\\}\\[\\]\\=]|:\\s)", operatorFormat );
		rules.emplace_back( "\\b([-+]?[\\.\\d]+)", numberFormat );
		rules.emplace_back( "\\@\\w+", macroFormat );
		rules.emplace_back( "\\$\\w+", macroFormat );
		rules.emplace_back( "#[^\\n]*", commentFormat ); // #
		commentStartRegex.setPattern( "#{3,}" );
		commentEndRegex.setPattern( "#{3,}" );
		break;

	case Language::lua:
		rules.emplace_back( "\\b(and|break|do|else|elseif|end|false|for|function|goto|if|in|local|nil|not|or|repeat|return|then|true|until|while)\\b", elementFormat );
		rules.emplace_back( "\"[^\\n\"]*\"", valueFormat );
		rules.emplace_back( "\\b([-+]?[\\.\\d]+)", numberFormat );
		rules.emplace_back( "[\\+\\-\\*\\/\\%\\\\#\\&\\~\\|\\<\\>\\(\\)\\{\\}\\[\\]\\=\\;\\:\\,\\.]", operatorFormat );
		rules.emplace_back( "--[^\\n]*", commentFormat );
		commentStartRegex.setPattern( "--\\[\\[" );
		commentEndRegex.setPattern( "\\]\\]" );
	default:
		break;
	}
}

void QCodeHighlighter::setFormats()
{
	operatorFormat.setForeground( Qt::darkGray );
	elementFormat.setForeground( Qt::darkBlue );
	elementFormat.setFontWeight( QFont::Bold );
	attributeFormat.setForeground( Qt::darkBlue );
	//attributeFormat.setFontItalic( true );
	//attributeFormat.setFontWeight( QFont::Bold );
	valueFormat.setForeground( Qt::darkRed );
	commentFormat.setForeground( Qt::gray );
	commentFormat.setFontItalic( true );
	numberFormat.setForeground( Qt::darkCyan );
	macroFormat.setForeground( Qt::darkMagenta );
	//macroFormat.setFontItalic( true );
	specialFormat.setForeground( Qt::darkMagenta );
	specialFormat.setFontItalic( true );
	//specialFormat.setFontWeight( QFont::Bold );
}

void QCodeHighlighter::setLanguage( Language l )
{
	language = l;
	setFormats();
	setRegexes();
}

xo::flat_map< std::string, QCodeHighlighter::Language > g_languages{
	{ "xml", QCodeHighlighter::Language::xml },
	{ "zml", QCodeHighlighter::Language::zml },
	{ "lua", QCodeHighlighter::Language::lua },
	{ "scone", QCodeHighlighter::Language::zml }
};

QCodeHighlighter::Language QCodeHighlighter::detectLanguage( const QString& filename )
{
	auto ext = xo::to_lower( xo::path( filename.toStdString() ).extension_no_dot().str() );
	if ( auto l = g_languages.find( ext ); l != g_languages.end() )
		return l->second;
	else return Language::unknown;
}

void QCodeHighlighter::registerLanguage( const QString& ext, Language lang )
{
	g_languages[ xo::to_lower( ext.toStdString() ) ] = lang;
}
