#pragma once

#include <QtWidgets/QWidget>
#include "ui_DPyInterface.h"

class DFindDialog;
class QStackedWidget;
class DCodeEditor;
class QSplitter;
class DPythonConsole;
class DGisApp;

class DPyInterface : public QWidget
{
    Q_OBJECT
private:
	struct DFile
	{
		QString fileName;
		QString filePath;
		bool hasSaved;
	};

public:
    DPyInterface(QWidget *parent = Q_NULLPTR, DGisApp *mainapp=nullptr);
	~DPyInterface();
	DPythonConsole* getPythonConsole();
	void setMainGisApp(DGisApp *mapp);

signals:
	void currentFileChanged(DFile *file);

private:
    Ui::DPyInterfaceClass ui;
	QStackedWidget *mMianContainer = nullptr;
	QSplitter *mMainSplitter = nullptr;
	DPythonConsole *mPythonConsole = nullptr;
	//DCodeEditor *mCodeEditor;
	DFindDialog *mFindDialog = nullptr;
	QList<DFile *> mFiles;
	int newFileimes = 1;
	bool isOpenPythonConsole = false;

private slots:
	void openFile(const QString &path = QString());
	void addNewFile(DFile *file);
	void updateInterface(int index);
	void closeCurrentFile();
	void saveAsFileSlot();
	void saveFileSlot();
	void onRunBtnClicked();
	void runAcommandSlot(QString &command);

	void testAddDockWidget();
	void testOutPut();

private:
	void deleteWork(int index);
	QString getSaveFileName();
	DGisApp *mMainApp = nullptr;
};
