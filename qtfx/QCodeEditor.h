#pragma once

#include "QCodeHighlighter.h"

#include <QWidget>
#include <QPlainTextEdit>
#include <QVector>
#include <QPair>
#include <QDateTime>

#include "xo/serialization/serialize.h"
#include "xo/filesystem/path.h"
#include "qt_convert.h"

class QCodeEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	QCodeEditor( QWidget* parent = 0 );
	virtual ~QCodeEditor();

	bool isEmpty() const { return fileName.isEmpty(); }
	QString getTitle();
	QCodeHighlighter::Language language() { return syntaxHighlighter->language; }

public slots:
	void open( const QString& filename );
	void save();
	void saveAs( const QString& filename );
	bool reload();
	void findDialog();
	bool findNext( bool backwards = false );
	void toggleComments();
	void duplicateText();
	void formatDocument();

	void updateLineNumberAreaWidth( int newBlockCount );
	void updateLineNumberArea( const QRect& rect, int dy );

	xo::path filePath() const { return path_from_qt( fileName ); }

public:
	QString defaultFolder;
	QString fileName;
	QString findText;
	QDateTime lastModified;
	bool insertAutoBrackets;

private:
	class QCodeHighlighter* syntaxHighlighter;

	void lineNumberAreaPaintEvent( QPaintEvent* event );
	int lineNumberAreaWidth();

protected:
	virtual void resizeEvent( QResizeEvent* event ) Q_DECL_OVERRIDE;
	virtual void keyPressEvent( QKeyEvent* e ) Q_DECL_OVERRIDE;

private:
	class LineNumberArea : public QWidget
	{
	public:
		LineNumberArea( QCodeEditor* editor ) : QWidget( editor ) { codeEditor = editor; }
		QSize sizeHint() const Q_DECL_OVERRIDE { return QSize( codeEditor->lineNumberAreaWidth(), 0 ); }
	protected:
		void paintEvent( QPaintEvent* event ) Q_DECL_OVERRIDE { codeEditor->lineNumberAreaPaintEvent( event ); }
	private:
		QCodeEditor* codeEditor;
	};

	QWidget* lineNumberArea;
	QRect previousRect;
	QVector<int> triggerFormatKeys;
	QVector<QPair<char, char>> autoBrackets;
};
