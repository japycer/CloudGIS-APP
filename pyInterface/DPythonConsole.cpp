#include "DPythonConsole.h"
#include<QKeyEvent>
#include"qtextcursor.h"

DPythonConsole::DPythonConsole(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.mBtnRun, &QPushButton::pressed, this, &DPythonConsole::onRunBtnClicked);
	connect(ui.mBtnPrevious, &QPushButton::pressed, this, &DPythonConsole::onPreviousBtnClicked);
	connect(ui.mBtnNext, &QPushButton::pressed, this, &DPythonConsole::onNextBtnClicked);
	connect(ui.mBtnClearOutput, &QPushButton::pressed, this, &DPythonConsole::onClearBtnClicked);
}

DPythonConsole::~DPythonConsole()
{
}

void DPythonConsole::keyPressEvent(QKeyEvent * event)
{
	if (ui.mLineEditCommand->hasFocus()) {
		QString command = ui.mLineEditCommand->text();
		if (event->key()+1 == Qt::Key_Enter && !command.isEmpty()) {
			historys.append(command);
			currentIndex = historys.count();
			ui.mLineEditCommand->setText("");
			emit runACommand(command);
			return;
		}
		if (event->key() == Qt::Key_Up&&currentIndex > 0)
		{
			ui.mLineEditCommand->setText(historys.at(--currentIndex));
			return;
		}
		if (event->key() == Qt::Key_Down&&currentIndex < historys.count()-1)
		{
			ui.mLineEditCommand->setText(historys.at(++currentIndex));
			return;
		}
	}
	return QWidget::keyPressEvent(event);
}


void DPythonConsole::onRunBtnClicked()
{
	QString command = ui.mLineEditCommand->text();
	if (!command.isEmpty()) {
		historys.append(command);
		currentIndex = historys.count();
		ui.mLineEditCommand->setText("");
		emit runACommand(command);
		return;
	}
}

void DPythonConsole::onPreviousBtnClicked()
{
	if (currentIndex > 0)
		ui.mLineEditCommand->setText(historys.at(--currentIndex));

}


void DPythonConsole::onNextBtnClicked()
{
	if (currentIndex < historys.count()-1)
		ui.mLineEditCommand->setText(historys.at(++currentIndex));
}

void DPythonConsole::onClearBtnClicked()
{
	ui.mTextEditOutput->clear();
}

//向输出窗口输出内容
void DPythonConsole::setOutPut(QString &outP, bool successFlag, QString &tips)	//tips=command
{
	QTextCursor tCursor= ui.mTextEditOutput->textCursor();
	tCursor.movePosition(QTextCursor::End);

	if (successFlag)
	{
		QColor color = QColor(0, 255, 0);
		QTextCharFormat fmt;
		fmt.setForeground(color);
		tCursor.insertText("   >>> " + tips + "\n" + outP, fmt);
	}
	else
	{
		QColor color = QColor(255, 0, 0);
		QTextCharFormat fmt;
		fmt.setForeground(color);
		tCursor.insertText("   >>> " + tips + "\n" + outP, fmt);
	}
}