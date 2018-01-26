#pragma once

#include <QWidget>
#include <QFileInfo>
#include <QSyntaxHighlighter>
#include <QPlainTextEdit>
#include "xo/serialization/serialize.h"

class QCodeEditor : public QWidget
{
	Q_OBJECT

public:
	QCodeEditor( QWidget* parent = 0 );
	virtual ~QCodeEditor();
	bool hasTextChanged() { return textChangedFlag; }
	QString getPlainText() const;

public slots:
	void open( const QString& filename );
	void openDialog( const QString& folder, const QString& fileTypes );
	void save();
	void saveAs( const QString& filename );

	QString getTitle();
	void textEditChanged();

signals:
	void textChanged();

public:
	QString defaultFolder;
	QString fileTypes;
	QString fileName;
	bool textChangedFlag = false;

private:
	xo::file_format getFileFormat( const QString& filename ) const;
	class QCodeSyntaxHighlighter* xmlSyntaxHighlighter;
    class QCodeTextEdit *textEdit;
};

//
// BasicXMLSyntaxHighlighter
//

class QCodeSyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT
public:
	enum Language { XML, ZML };

	QCodeSyntaxHighlighter( QObject* parent, Language f );
	QCodeSyntaxHighlighter( QTextDocument* parent, Language f );

	void setLanguage( Language l );
	static Language detectLanguage( const QString& filename );

	virtual ~QCodeSyntaxHighlighter() {}

protected:
	virtual void highlightBlock( const QString& text );

private:
	void highlightByRegex( const QTextCharFormat& format, const QRegExp& regex, const QString& text );
	void setRegexes();
	void setFormats();

private:
	Language language;
	struct Rule {
		QRegExp regExp;
		QTextCharFormat format;
	};

	QTextCharFormat     m_KeywordFormat;
	QTextCharFormat     m_ElementFormat;
	QTextCharFormat     m_AttributeFormat;
	QTextCharFormat     m_ValueFormat;
	QTextCharFormat     m_CommentFormat;
	QTextCharFormat     m_NumberFormat;

	QList<QRegExp>      m_xmlKeywordRegexes;
	QRegExp             m_xmlElementRegex;
	QRegExp             m_xmlAttributeRegex;
	QRegExp             m_xmlValueRegex;
	QRegExp             m_xmlCommentRegex;
	QRegExp             m_NumberRegex;
};

//
// QCodeTextEdit
//

class QCodeTextEdit : public QPlainTextEdit
{
	Q_OBJECT

public:
	class LineNumberArea : public QWidget
	{
	public:
		LineNumberArea( QCodeTextEdit *editor ) : QWidget( editor ) { codeEditor = editor; }
		QSize sizeHint() const Q_DECL_OVERRIDE { return QSize( codeEditor->lineNumberAreaWidth(), 0 ); }
	protected:
		void paintEvent( QPaintEvent *event ) Q_DECL_OVERRIDE { codeEditor->lineNumberAreaPaintEvent( event ); }
	private:
		QCodeTextEdit *codeEditor;
	};

	QCodeTextEdit( QWidget* parent = 0 );
	void lineNumberAreaPaintEvent( QPaintEvent *event );
	int lineNumberAreaWidth();

public slots:
	void updateLineNumberAreaWidth( int newBlockCount );
	void updateLineNumberArea( const QRect& rect, int dy );

protected:
	void resizeEvent( QResizeEvent *event );

private:
	QWidget *lineNumberArea;
};
