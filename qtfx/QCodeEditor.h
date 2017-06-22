#pragma once

#include <QWidget>
#include <QFileInfo>
#include <QSyntaxHighlighter>
#include <QPlainTextEdit>

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
	void saveAsDialog( const QString& folder, const QString& fileTypes );

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

	class BasicXMLSyntaxHighlighter* xmlSyntaxHighlighter;
    class QCodeTextEdit *textEdit;
};

//
// BasicXMLSyntaxHighlighter
//

class BasicXMLSyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT
public:
	BasicXMLSyntaxHighlighter( QObject * parent );
	BasicXMLSyntaxHighlighter( QTextDocument * parent );
	BasicXMLSyntaxHighlighter( QTextEdit * parent );

	virtual ~BasicXMLSyntaxHighlighter() {}

protected:
	virtual void highlightBlock( const QString & text );


private:
	void highlightByRegex( const QTextCharFormat & format, const QRegExp & regex, const QString & text );
	void setRegexes();
	void setFormats();

private:
	QTextCharFormat     m_xmlKeywordFormat;
	QTextCharFormat     m_xmlElementFormat;
	QTextCharFormat     m_xmlAttributeFormat;
	QTextCharFormat     m_xmlValueFormat;
	QTextCharFormat     m_xmlCommentFormat;

	QList<QRegExp>      m_xmlKeywordRegexes;
	QRegExp             m_xmlElementRegex;
	QRegExp             m_xmlAttributeRegex;
	QRegExp             m_xmlValueRegex;
	QRegExp             m_xmlCommentRegex;
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
