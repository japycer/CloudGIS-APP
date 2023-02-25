#include "DPyInterface.h"
#include "qfiledialog.h"
#include "DFindDialog.h"
#include"DCodeEditor.h"
#include"qstackedwidget.h"
#include"qfileinfo.h"
#include"qmessagebox.h"
#include"qtextstream.h"
#include"qsplitter.h"
#include"DPythonConsole.h"
#include"../DGisApp.h"
#include"../subCode//DPythonUtils.h"

DPyInterface::DPyInterface(QWidget *parent, DGisApp *mainapp)
    : QWidget(parent)
{
    ui.setupUi(this);
	//mCodeEditor = this->ui.mCodeEditor;

	mMainApp = mainapp;
	mFindDialog = new DFindDialog();
	//mFindDialog->setEditor(mCodeEditor);
	mFindDialog->setWindowOpacity(0.9);

	//进行信号槽的连接
	connect(ui.mComboxFiles, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) { updateInterface(index); });
	connect(ui.actionmActionNewFile, &QAction::triggered, [=]{
		DFile *newFile = new DFile{ QString("untitle%1").arg(newFileimes++), "",false };
		addNewFile(newFile);
	});


	//初始化主窗体所有部件
	mMianContainer = new QStackedWidget();
	mMainSplitter = new QSplitter();
	mPythonConsole = new DPythonConsole();
	mMainSplitter->setStyleSheet("QSplitter::handle{background-color:rgb(100, 100, 100);}");
	mMainSplitter->setOrientation(Qt::Vertical);
	mMainSplitter->addWidget(ui.mWidgetFileInfo);
	mMainSplitter->addWidget(mMianContainer);
	mMainSplitter->addWidget(mPythonConsole);
	ui.mVerticalLayoutMain->addWidget(mMainSplitter);

	mPythonConsole->setVisible(false);

	DFile *newFile = new DFile{QString("untitle%1").arg(newFileimes++), "",false };
	addNewFile(newFile);

	//复制、粘贴、剪贴等功能信号槽的连接
	connect(this->ui.actionmActionClear, &QAction::triggered, [&] {static_cast<DCodeEditor *>(mMianContainer->currentWidget())->clear(); });
	connect(this->ui.actionmActionCopy, &QAction::triggered, [&] {static_cast<DCodeEditor *>(mMianContainer->currentWidget())->copy(); });
	connect(this->ui.actionmActionPaste, &QAction::triggered, [&] {static_cast<DCodeEditor *>(mMianContainer->currentWidget())->paste(); });
	connect(this->ui.actionmActionOenFile, &QAction::triggered, [=] {openFile(); });
	connect(this->ui.mBtnCloseCurrent, &QPushButton::pressed, this, &DPyInterface::closeCurrentFile);
	connect(this->ui.actionmActionCut, &QAction::triggered, [&] {static_cast<DCodeEditor *>(mMianContainer->currentWidget())->cut(); });
	connect(this->ui.actionmActionComment, &QAction::triggered, [&] {static_cast<DCodeEditor *>(mMianContainer->currentWidget())->commentSlection(); });
	connect(this->ui.actionmActionSearch, &QAction::triggered, [&] {
		mFindDialog->isVisible() ? mFindDialog->hide() : mFindDialog->show();
	});
	connect(this->ui.actionmActionSaveEdit, &QAction::triggered, this, &DPyInterface::saveFileSlot);
	connect(this->ui.actionmActionSaveAs, &QAction::triggered, this, &DPyInterface::saveAsFileSlot);

	//test
	connect(ui.actionmActionOpenConsole, &QAction::triggered, this, &DPyInterface::testAddDockWidget);
	//connect(ui.actionmActionRun, &QAction::triggered, [&] {testOutPut(); });

	//run python command
	connect(ui.actionmActionRun, &QAction::triggered, this, &DPyInterface::onRunBtnClicked);
	connect(mPythonConsole, &DPythonConsole::runACommand, this, &DPyInterface::runAcommandSlot);
}

DPyInterface::~DPyInterface()
{
	int nums = mMianContainer->count();
	for (size_t i = 0; i < nums; i++)
	{
		auto toDelete = mMianContainer->widget(i);
		delete toDelete;
	}
	delete mMianContainer;
	delete mMainSplitter;
	//DCodeEditor *mCodeEditor;
	delete mFindDialog;
	qDeleteAll(mFiles);
}

void DPyInterface::openFile(const QString &path)
{
	QString fileFullName = path;

	if (fileFullName.isNull())
		fileFullName = QFileDialog::getOpenFileName(this, tr("Open Python File"), "", "*.py;; *.*");
	if (!fileFullName.isEmpty()) {
		QFile file(fileFullName);
		if (file.open(QFile::ReadOnly | QFile::Text))
		{
			QFileInfo fileInfo = QFileInfo(file);
			QString filePath = fileInfo.filePath();
			QString fileName = fileInfo.fileName();
			DFile *newFile = new DFile{ fileInfo.fileName(),fileInfo.filePath(),true };
			addNewFile(newFile);
			static_cast<DCodeEditor *>(mMianContainer->currentWidget())->setPlainText(file.readAll());
		}
		file.close();
	}
}

void DPyInterface::addNewFile(DFile * file)
{
	mFiles.append(file);
	ui.mLineEditFilePath->setText(file->filePath);
	ui.mComboxFiles->addItem(file->fileName);
	
	DCodeEditor * editor = new DCodeEditor();
	mMianContainer->addWidget(editor);
	int count = mMianContainer->currentIndex();
	ui.mComboxFiles->setCurrentIndex(ui.mComboxFiles->count()-1);

	connect(editor, &QPlainTextEdit::textChanged, [&] {
		if (mFiles.at(ui.mComboxFiles->currentIndex())->hasSaved)
		{
			mFiles.at(ui.mComboxFiles->currentIndex())->hasSaved = false;
			ui.mLineEditFilePath->setText((mFiles.at(ui.mComboxFiles->currentIndex())->filePath) + "*");
		}
	});
}

void DPyInterface::updateInterface(int index)
{
	if (index >= 0)
	{
		mMianContainer->setCurrentIndex(index);
		mFindDialog->setEditor(static_cast<DCodeEditor *>(mMianContainer->currentWidget()));

		ui.mLineEditFilePath->setText((mFiles.at(index)->filePath) + (mFiles.at(index)->hasSaved ? "":"*"));
	}
}

void DPyInterface::closeCurrentFile()
{
	if (ui.mComboxFiles->count() == 1)
	{
		QMessageBox msg(this);

		msg.setWindowTitle("Find");
		msg.setText(tr("Can not delete the last file"));
		msg.setIcon(QMessageBox::Information);
		msg.setStandardButtons(QMessageBox::Ok);
		msg.exec();
		return;
	}
	int currentIndex = ui.mComboxFiles->currentIndex();
	deleteWork(currentIndex);
	return;
}

void DPyInterface::deleteWork(int index)
{
	if (index >= 0)
	{
		ui.mComboxFiles->removeItem(index);
		auto toRemove = mMianContainer->widget(index);
		mMianContainer->removeWidget(toRemove);
		delete toRemove;
		auto toDelete = mFiles.at(index);
		mFiles.removeAt(index);
		delete toDelete;
	}
}



void DPyInterface::saveAsFileSlot()
{
	QString saveFileName = QFileDialog::getSaveFileName(this, "Save File", QDir::currentPath());

	if (saveFileName.isEmpty())
		return;

	QFile * file = new QFile;
	file->setFileName(saveFileName);
	bool ok = file->open(QIODevice::WriteOnly);
	if (ok)
	{
		QTextStream out(file);
		QFileInfo fileInfo(*file);
		out << static_cast<DCodeEditor *>(mMianContainer->currentWidget())->toPlainText();//去除DCodeEditor当中的纯文本信息
		file->close();

		QString fileName = fileInfo.fileName();
		QString filePath = fileInfo.filePath();
		int currentIndex = ui.mComboxFiles->currentIndex();
		mFiles.at(currentIndex)->filePath = filePath;
		mFiles.at(currentIndex)->fileName = fileName;
		mFiles.at(currentIndex)->hasSaved = true;
		ui.mComboxFiles->setEditable(true);
		ui.mComboxFiles->setItemText(currentIndex, fileName);
		ui.mComboxFiles->setEditable(false);
		ui.mLineEditFilePath->setText(filePath);
		delete file;
	}
}

void DPyInterface::saveFileSlot()
{
	if (!ui.mLineEditFilePath->text().endsWith("*"))
		return;
	QString saveFileName = getSaveFileName();
	if (saveFileName.isEmpty())
		return;
	
	QFile * file = new QFile;
	file->setFileName(saveFileName);
	bool ok = file->open(QIODevice::WriteOnly);
	if (ok)
	{
		QTextStream out(file);
		QFileInfo fileInfo(*file);
		out << static_cast<DCodeEditor *>(mMianContainer->currentWidget())->toPlainText();//去除DCodeEditor当中的纯文本信息
		file->close();

		QString fileName = fileInfo.fileName();
		QString filePath = fileInfo.filePath();
		int currentIndex = ui.mComboxFiles->currentIndex();
		mFiles.at(currentIndex)->filePath = filePath;
		mFiles.at(currentIndex)->fileName = fileName;
		mFiles.at(currentIndex)->hasSaved = true;
		ui.mComboxFiles->setEditable(true);
		ui.mComboxFiles->setItemText(currentIndex, fileName);
		ui.mComboxFiles->setEditable(false);
		ui.mLineEditFilePath->setText(filePath);
		delete file;
	}
}


QString DPyInterface::getSaveFileName()
{
	int currentIndex = ui.mComboxFiles->currentIndex();
	if(mFiles.at(currentIndex)->filePath.isEmpty())
		return QFileDialog::getSaveFileName(this, "Save File", QDir::currentPath(), tr("Images (*.py)"));
	return mFiles.at(currentIndex)->filePath;
}

void DPyInterface::testAddDockWidget()
{
	mPythonConsole->setVisible(!mPythonConsole->isVisible());
}

void DPyInterface::testOutPut()
{
	mPythonConsole->setOutPut(QString("Successfully run a command"), true);
}

// getpython console pointer
DPythonConsole* DPyInterface::getPythonConsole()
{
	return mPythonConsole;
}

void DPyInterface::setMainGisApp(DGisApp * mapp)
{
	mMainApp = mapp;
}

void DPyInterface::onRunBtnClicked()
{
	int currentIndex = ui.mComboxFiles->currentIndex();
	if (!mFiles.at(currentIndex)->hasSaved)
		saveFileSlot();
	QString contents = static_cast<DCodeEditor *>(mMianContainer->currentWidget())->toPlainText();
	if (contents.isEmpty())
		return;

	QString currentFileName = mFiles.at(currentIndex)->filePath;
	QString command = QString("exec(open('%1'.encode('utf-8')).read())").arg(currentFileName);

	if (command.isEmpty())
		return;
	QString errorMes;
	bool res = mMainApp->runString(command, errorMes, false);

	QString outP;
	if (res)
		outP = mMainApp->getPythonRunner()->getOutPut();
	else
		outP = mMainApp->getPythonRunner()->getLastRes();

	mPythonConsole->setOutPut(outP, res, command);
}


void DPyInterface::runAcommandSlot(QString &command)
{
	if (command.isEmpty())
		return;
	QString errorMes;
	bool res = mMainApp->runString(command, errorMes, true);

	//QString muRes = mMainApp->getPythonRunner()->getOutPut();
	QString outP;
	// 运送成功或者输出（若有），否则或者错误
	if (res)
		outP = mMainApp->getPythonRunner()->getOutPut();
	else 
		outP = mMainApp->getPythonRunner()->getLastRes();

	mPythonConsole->setOutPut(outP, res, command);
}