#pragma once

#include <QObject>
#include "DToolBox.h"

class DCustomWindow;

class DToolBoxManager : public QObject
{
	Q_OBJECT

public:
	DToolBoxManager(QObject *parent = nullptr);
	~DToolBoxManager();


public slots:
	void openToolBox(DToolBox::ToolType, const QString &);

private:
	DCustomWindow* mCurrentOpenTool = nullptr;

private:
	void openVectorTool(const QString&);
	void openRasterTool(const QString&);
	void openAnalysisTool(const QString&);
	void openHydrologyTool(const QString&);
	void openLandsatTool(const QString&);
	void openPythonTool(const QString&);
	void openExportTool(const QString&);
	void openOthersTool(const QString&);
};
