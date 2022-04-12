#pragma once

#include <QSyntaxHighlighter>
#include <QtCore/QRegularExpression>

struct QCodeBlockUserData : public QTextBlockUserData
{
	QCodeBlockUserData( int start = -1, int end = -1 ) : commentStart( start ), commentEnd( end ) {}
	int commentStart;
	int commentEnd;
};

class QCodeHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT
public:
	enum class Language { unknown, xml, zml, lua };

	QCodeHighlighter( QObject* parent, Language f );
	QCodeHighlighter( QTextDocument* parent, Language f );
	virtual ~QCodeHighlighter() {}

	void setLanguage( Language l );
	static Language detectLanguage( const QString& filename );
	static void registerLanguage( const QString& extension, Language l );

	Language language;
	QString lineCommentString;

	QRegularExpression commentLine;
	QRegularExpression increaseIndentRegex;
	QRegularExpression decreaseIndentRegex;
	QRegularExpression commentStartRegex;
	QRegularExpression commentEndRegex;

protected:
	virtual void highlightBlock( const QString& text );
	bool isBetweenQuotes( const QString& text, int index );

private:
	struct HighlightRule {
		HighlightRule( const char* e, QTextCharFormat f ) : regExp( e ), format( f ) {}
		HighlightRule( const QRegularExpression& e, QTextCharFormat f ) : regExp( e ), format( f ) {}
		QRegularExpression regExp;
		QTextCharFormat format;
	};

	void applyRule( const QString& text, const HighlightRule& r );
	QRegularExpressionMatch match( const QRegularExpression& regex, const QString& text, int index );
	void setRegexes();
	void setFormats();

	std::vector< HighlightRule > rules;

	QTextCharFormat operatorFormat;
	QTextCharFormat elementFormat;
	QTextCharFormat attributeFormat;
	QTextCharFormat valueFormat;
	QTextCharFormat commentFormat;
	QTextCharFormat numberFormat;
	QTextCharFormat specialFormat;
	QTextCharFormat macroFormat;
};
