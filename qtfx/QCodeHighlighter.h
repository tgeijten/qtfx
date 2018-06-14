#pragma once

#include <QSyntaxHighlighter>
#include <QtCore/qregularexpression.h>

class QCodeHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT
public:
	enum Language { XML, ZML };

	QCodeHighlighter( QObject* parent, Language f );
	QCodeHighlighter( QTextDocument* parent, Language f );

	void setLanguage( Language l );
	static Language detectLanguage( const QString& filename );
	virtual ~QCodeHighlighter() {}

protected:
	virtual void highlightBlock( const QString& text );
	bool isBetweenQuotes( const QString& text, int index );

private:
	void highlightByRegex( const QTextCharFormat& format, const QRegularExpression& regex, const QString& text );
	QRegularExpressionMatch match( const QRegularExpression& regex, const QString& text, int index );
	void setRegexes();
	void setFormats();

private:
	Language language;
	struct HighlightRule {
		HighlightRule( const char* e, QTextCharFormat f ) : regExp( e ), format( f ) {}
		QRegularExpression regExp;
		QTextCharFormat format;
	};
	std::vector< HighlightRule > rules;


	QTextCharFormat m_OperatorFormat;
	QTextCharFormat m_ElementFormat;
	QTextCharFormat m_AttributeFormat;
	QTextCharFormat m_ValueFormat;
	QTextCharFormat m_CommentFormat;
	QTextCharFormat m_NumberFormat;
	QTextCharFormat m_SpecialFormat;

	QRegularExpression m_OperatorRegex;
	QRegularExpression m_xmlElementRegex;
	QRegularExpression m_AttributeRegex;
	QRegularExpression m_StringRegex;
	QRegularExpression m_CommentRegex;
	QRegularExpression m_NumberRegex;
	QRegularExpression m_SpecialRegex;
	QRegularExpression commentStartRegex;
	QRegularExpression commentEndRegex;
};
