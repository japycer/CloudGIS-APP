#pragma once

#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class DHighLighter;
QT_END_NAMESPACE

class LineNumberArea;

class DCodeEditor : public QPlainTextEdit
{
	Q_OBJECT
public:
	DCodeEditor(QWidget *parent = nullptr);
	~DCodeEditor();
	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();
public slots:
	void commentSlection();

	//TODO ´úÂëÕÛµþ¹¦ÄÜ
	// void visibleLines(int fromLine, int toLine)

protected:
	void resizeEvent(QResizeEvent *event) override;
	void keyPressEvent(QKeyEvent *e) override;
	void keyReleaseEvent(QKeyEvent *e) override;
	
	void wheelEvent(QWheelEvent *e) override;

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &rect, int dy);

private:
	QWidget *lineNumberArea = nullptr;
	DHighLighter *highlighter = nullptr;
	void commentDerection(QTextCursor &cs, bool toUp = true);
	int assureCommentDirection(QTextCursor &cs);
	void starToComment(QTextCursor &cs, size_t line);
	bool keyAltPressed = false;
};

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(DCodeEditor *editor) : QWidget(editor), codeEditor(editor)
	{}

	QSize sizeHint() const override
	{
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) override
	{
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	DCodeEditor *codeEditor = nullptr;
};