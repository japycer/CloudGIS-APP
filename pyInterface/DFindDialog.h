#pragma once

#include <QWidget>
#include"ui_DFindDialog.h"

class DCodeEditor;

class DFindDialog : public QWidget
{
	Q_OBJECT

public:
	DFindDialog(QWidget *parent = Q_NULLPTR, DCodeEditor *editor=nullptr);
	~DFindDialog();
	DCodeEditor *getEditor();
	void setEditor(DCodeEditor *editor);

private:
	Ui::mFindDialog ui;
	DCodeEditor * mTextEditor = nullptr;

private slots:
	void onFindClicked();
	void onReplaceClicked();
	void onReplaceAllClicked();
};
