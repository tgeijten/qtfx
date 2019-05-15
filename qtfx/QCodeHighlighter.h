#pragma once

#include <QSyntaxHighlighter>
#include <QtCore/QRegularExpression>

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

protected:
	virtual void highlightBlock( const QString& text );
	bool isBetweenQuotes( const QString& text, int index );

private:
	struct HighlightRule {
		HighlightRule( const char* e, QTextCharFormat f ) : regExp( e ), format( f ) {}
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

	QRegularExpression commentStartRegex;
	QRegularExpression commentEndRegex;
};
