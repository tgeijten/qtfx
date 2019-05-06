#pragma once

#include <QWidget>
#include <QFileInfo>

#include "QCodeHighlighter.h"
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
	bool isEmpty() const { return fileName.isEmpty(); }
	QString getTitle();
	QCodeHighlighter::Language language() { return syntaxHighlighter->language; }

public slots:
	void open( const QString& filename );
	void openDialog( const QString& folder, const QString& fileTypes );
	void save();
	void saveAs( const QString& filename );

	void textEditChanged();

signals:
	// TODO: use isModified / setModified! from QTextEdit
	void textChanged();

public:
	QString defaultFolder;
	QString fileTypes;
	QString fileName;
	bool textChangedFlag = false;

private:
	std::string getFileFormat( const QString& filename ) const;
	class QCodeHighlighter* syntaxHighlighter;
    class QCodeTextEdit *textEdit;
};

class QCodeTextEdit : public QPlainTextEdit
{
	Q_OBJECT

public:
	QCodeTextEdit( QCodeEditor* parent = 0 );
	void lineNumberAreaPaintEvent( QPaintEvent *event );
	int lineNumberAreaWidth();

public slots:
	void updateLineNumberAreaWidth( int newBlockCount );
	void updateLineNumberArea( const QRect& rect, int dy );
	void formatDocument();

protected:
	virtual void resizeEvent( QResizeEvent *event ) Q_DECL_OVERRIDE;
	virtual void keyPressEvent( QKeyEvent *e ) Q_DECL_OVERRIDE;

private:
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

	QCodeEditor* codeEditor;
	QWidget *lineNumberArea;
	QRect previousRect;
};
