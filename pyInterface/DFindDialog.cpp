#include "DFindDialog.h"
#include "DCodeEditor.h"
#include"qmessagebox.h"

DFindDialog::DFindDialog(QWidget *parent, DCodeEditor *editor)
	: QWidget(parent), mTextEditor(editor)
{
	ui.setupUi(this);
	ui.mBtnNext->setChecked(true);
	ui.mBtnPrevious->setChecked(false);

	connect(ui.mBtnFind, &QPushButton::pressed, this, &DFindDialog::onFindClicked);
	connect(ui.mBtnReplace, &QPushButton::pressed, this, &DFindDialog::onReplaceClicked);
	connect(ui.mBtnReplaceAll, &QPushButton::pressed, this, &DFindDialog::onReplaceAllClicked);
	connect(ui.mBtnClose, &QPushButton::pressed, this, &QDialog::hide);
	connect(ui.mBtnNext, &QPushButton::pressed, [&] {ui.mBtnPrevious->setChecked(!ui.mBtnPrevious->isChecked()); });
	connect(ui.mBtnPrevious, &QPushButton::pressed, [&] {ui.mBtnNext->setChecked(!ui.mBtnNext->isChecked()); });
}


DFindDialog::~DFindDialog()
{
}


DCodeEditor * DFindDialog::getEditor()
{
	return mTextEditor;
}


void DFindDialog::setEditor(DCodeEditor * editor)
{
	mTextEditor = editor;
}


void DFindDialog::onFindClicked()
{
	QString target = ui.mLineEditFind->text();

	if ((mTextEditor != NULL) && (target != ""))
	{
		QString text = mTextEditor->toPlainText();
		QTextCursor c = mTextEditor->textCursor();
		int index = -1;

		if (ui.mBtnPrevious->isChecked())
		{
			index = text.indexOf(target, c.position(), ui.mRadioBtnCaseIntensive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);

			if (index >= 0)
			{
				c.setPosition(index);
				c.setPosition(index + target.length(), QTextCursor::KeepAnchor);

				mTextEditor->setTextCursor(c);
			}
		}

		if (ui.mBtnNext->isChecked())
		{
			index = text.lastIndexOf(target, c.position() - text.length() - 1, ui.mRadioBtnCaseIntensive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);

			if (index >= 0)
			{
				c.setPosition(index + target.length());
				c.setPosition(index, QTextCursor::KeepAnchor);

				mTextEditor->setTextCursor(c);
			}
		}

		if (index < 0)
		{
			QMessageBox msg(this);

			msg.setWindowTitle("Find");
			msg.setText("Can not find \"" + target + "\" any more...");
			msg.setIcon(QMessageBox::Information);
			msg.setStandardButtons(QMessageBox::Ok);

			msg.exec();
		}
	}
}


void DFindDialog::onReplaceClicked()
{
	QString target = ui.mLineEditFind->text();
	QString to = ui.mLineEditReplace->text();

	if ((mTextEditor != NULL) && (target != "") && (to != ""))
	{
		QString selText = mTextEditor->textCursor().selectedText();

		if (selText == target)
		{
			mTextEditor->insertPlainText(to);
		}

		onFindClicked();//点击第一次时不会替换，只是查找
	}
}


void DFindDialog::onReplaceAllClicked()
{
	QString target = ui.mLineEditFind->text();
	QString to = ui.mLineEditReplace->text();

	if ((mTextEditor != NULL) && (target != "") && (to != ""))
	{
		QString text = mTextEditor->toPlainText();

		text.replace(target, to, ui.mRadioBtnCaseIntensive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
		mTextEditor->clear();
		mTextEditor->insertPlainText(text);
	}
}
