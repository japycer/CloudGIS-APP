#include "DCodeEditor.h"
#include"DHighLighter.h"
#include <QPainter>
#include <QTextBlock>
#include<QFont>
#include<QTextList>



//构造函数
DCodeEditor::DCodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
	lineNumberArea = new LineNumberArea(this);

	connect(this, &DCodeEditor::blockCountChanged, this, &DCodeEditor::updateLineNumberAreaWidth);
	connect(this, &DCodeEditor::updateRequest, this, &DCodeEditor::updateLineNumberArea);
	connect(this, &DCodeEditor::cursorPositionChanged, this, &DCodeEditor::highlightCurrentLine);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();

	// 白色主题
	//QFont font;
	//font.setFamily("Courier");
	//font.setFixedPitch(true);
	//font.setPointSize(10);
	// 黑色主题
	QFont font;
	font.setFamily("NSimSun");
	font.setFixedPitch(true);
	font.setPointSize(10);

	this->setFont(font);
	this->setStyleSheet("background-color: rgb(30, 30, 30);color: rgb(248, 248, 242);");

	highlighter = new DHighLighter(this->document());

	QPalette p;
	p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
	p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));
	this->setPalette(p);
}

DCodeEditor::~DCodeEditor()
{

}

//计算并返回行号区域的宽度
int DCodeEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}

	int space = 6 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

	return space;
}


//更新行号区域的宽度
void DCodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


//更新行号区域
void DCodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}


//当窗体大小改变时，触发该事件
void DCodeEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

//由于python编译器的特殊，需要对输入tab和enter进行处理
void DCodeEditor::keyPressEvent(QKeyEvent * e)
{
	//如果输入的是tab键，则需要将tab效果键替换为输入“    ”（4空格）
	if (e->key() == Qt::Key_Tab) {
		this->insertPlainText("    ");
		return;
	}
	//如果输入的是enter键，则需要换行并根据本行代码块的内容进行缩进
	if (e->key()+1 == Qt::Key_Enter) {
		QTextCursor cursor = this->textCursor();
		//如果光标位于行首，则不进行缩进，直接换行
		if(cursor.positionInBlock()==0)
			return QPlainTextEdit::keyPressEvent(e);

		//获取当前光标所在的代码块和文本内容
		int lineNumber = cursor.blockNumber();
		QString currentLineStr = this->document()->findBlockByLineNumber(lineNumber).text();
		QRegExp rx1("\\S");		//构建正则表达式
		int spaceNum = rx1.indexIn(currentLineStr);	//判断第一个非空格键的位置，确定下一行缩进
		QRegExp rx2(":");	//构建正则表达式2，判断本行文本皆为是不是为“：”，如果是，下一行缩进再加“    ”
		QString str("\n");
		if (spaceNum > 0) {
			for (int i = 0; i < spaceNum; i += 4)
				str.append("    ");
			spaceNum = 0;
		}
		spaceNum = rx2.lastIndexIn(currentLineStr);
		if (spaceNum > 0 && spaceNum >= currentLineStr.length() - 1)
			str.append("    ");
		this->insertPlainText(str);		//插入字符串
		return;
	}
	if (e->key() == Qt::Key_Alt)	keyAltPressed = true;
	return QPlainTextEdit::keyPressEvent(e);
}

void DCodeEditor::keyReleaseEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Alt)
	{
		keyAltPressed = false;
		return;
	}
	return QPlainTextEdit::keyReleaseEvent(e);
}

// Alt + wheel放大缩小
void DCodeEditor::wheelEvent(QWheelEvent * e)
{
	if (keyAltPressed)
	{
		if (e->delta() > 0)
			zoomIn();
		else
			zoomOut();
		return;
	}
	return QPlainTextEdit::wheelEvent(e);
}



//高亮当前行
void DCodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		// 白色主题
		//QColor lineColor = QColor(Qt::yellow).lighter(160);
		// 黑色主题
		QColor lineColor = QColor(40, 40, 40).lighter(160);
		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

//行号区域绘制事件，由lineNumber.paint事件调用
void DCodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(lineNumberArea);
	QFont font("SimSun", 8, QFont::Bold, true);
	painter.setFont(font);
	// 白色主题
	//painter.fillRect(event->rect(), Qt::lightGray);
	// 黑色主题
	painter.fillRect(event->rect(), QColor(35, 35, 35));
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
	int bottom = top + qRound(blockBoundingRect(block).height());

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			//painter.setPen(Qt::blue); 白色主题
			painter.setPen(QColor(37, 163, 208));
			painter.drawText(0, top+3, lineNumberArea->width()-6, fontMetrics().height(),
				Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + qRound(blockBoundingRect(block).height());
		++blockNumber;
	}
}

//注释功能
void DCodeEditor::commentSlection()
{
	QTextCursor csOld = this->textCursor();
	int startS = csOld.selectionStart();
	int endS = csOld.selectionEnd();

	csOld.setPosition(endS);
	int num2 = csOld.blockNumber();
	csOld.setPosition(startS);
	int num1 = csOld.blockNumber();

	QRegExp rx("\\S");
	QRegExp rx2("#");		//构建正则表达式2
	csOld.movePosition(QTextCursor::StartOfLine);
	int minNum = rx.indexIn(csOld.block().text());
	//int minNum = csOld.positionInBlock();
	for (size_t i = num1+1; i < num2 + 1; i++)
	{
		csOld.movePosition(QTextCursor::NextBlock);
		csOld.movePosition(QTextCursor::StartOfLine);

		int currentNum = rx.indexIn(csOld.block().text());
		if (currentNum < minNum)
			minNum = currentNum;
	}

	csOld.setPosition(startS);
	for (size_t i = num1; i < num2+1; i++)
	{
		csOld.movePosition(QTextCursor::StartOfLine);
		QString currentLineStr = csOld.block().text();
		int index1 = rx.indexIn(currentLineStr);
		int index2 = rx2.indexIn(currentLineStr);
		if (index1 == index2 && index1 >= 0) {
			csOld.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, index1);
			csOld.deleteChar();
		}
		else
		{
			csOld.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, minNum);
			csOld.insertText("#");
		}
		csOld.movePosition(QTextCursor::NextBlock);
	}
}


void DCodeEditor::commentDerection(QTextCursor &cs, bool toUp)
{
	if (toUp) {
		cs.movePosition(QTextCursor::PreviousRow);
		if (cs.hasSelection())
		{
			starToComment(cs,1);
			commentDerection(cs, true);
		}
		else return;
	}
	else
	{
		cs.movePosition(QTextCursor::NextRow);
		if (cs.hasSelection())
		{
			starToComment(cs, 1);
			commentDerection(cs, false);
		}
		else return;
	}
}

//返回0本行、-1代表往上、1代表往下
int DCodeEditor::assureCommentDirection(QTextCursor & cs)
{
	if (!cs.hasSelection())
		return 0;

	cs.movePosition(QTextCursor::PreviousRow);
	if (cs.hasSelection())
		return -1;
	cs.movePosition(QTextCursor::NextRow);
	cs.movePosition(QTextCursor::NextRow);
	if (cs.hasSelection())
		return 1;
	return 0;
}

void DCodeEditor::starToComment(QTextCursor & cs, size_t line)
{
	//int lineNumber = 0;
	//cs.movePosition(QTextCursor::StartOfLine);

	//lineNumber = cs.blockNumber();
	QString currentLineStr = this->document()->findBlockByLineNumber(line).text();
	QRegExp rx1("\\S");		//构建正则表达式1
	QRegExp rx2("#");		//构建正则表达式2
	int index1 = rx1.indexIn(currentLineStr);
	int index2 = rx2.indexIn(currentLineStr);
	if (index1 == index2 && index1 >= 0)
	{
		cs.setPosition(index1);
		cs.deleteChar();
		//cs.movePosition(QTextCursor::)
	}
	else if(index1 >= 0)
	{
		cs.movePosition(QTextCursor::StartOfBlock);
		cs.insertText("#");
	}
}
