#include "DToolBoxManager.h"
#include "DClassifyManager.h"
#include <QMessageBox>
#include "DCustomWindow.h"
#include "DImgCollectionViewer.h"
#include "DImageViewer.h"
#include "DTasselledCapTransformation.h"
#include "DLandsatCalibration.h"
#include "DDarkPixelAtomosphereCalibration.h"
#include "DGeeExporter.h"

DToolBoxManager::DToolBoxManager(QObject *parent)
	: QObject(parent)
{
}


DToolBoxManager::~DToolBoxManager()
{
	delete mCurrentOpenTool;
}


void DToolBoxManager::openToolBox(DToolBox::ToolType type, const QString& toolName)
{
	if (toolName.isEmpty()) return;
	if (mCurrentOpenTool)
	{
		if (mCurrentOpenTool->isVisible())
		{
			QMessageBox::critical(nullptr, "Open Error", "Pleaser close current toolWindow, try again", QMessageBox::Ok);
			return;
		}
		delete mCurrentOpenTool;
	}

	switch (type)
	{
	case DToolBox::Vector:
		openVectorTool(toolName);
		break;
	case DToolBox::Raster:
		openRasterTool(toolName);
		break;
	case DToolBox::Analysis:
		openAnalysisTool(toolName);
		break;
	case DToolBox::Hydrology:
		openHydrologyTool(toolName);
		break;
	case DToolBox::Landsat:
		openLandsatTool(toolName);
		break;
	case DToolBox::Python:
		openPythonTool(toolName);
		break;
	case DToolBox::Export:
		openExportTool(toolName);
		break;
	case DToolBox::Others:
		openOthersTool(toolName);
		break;
	default:
		break;
	}
}

void DToolBoxManager::openVectorTool(const QString& toolName)
{

}


void DToolBoxManager::openRasterTool(const QString& toolName)
{
	if (toolName.compare("Classify") == 0)
	{
		mCurrentOpenTool = new DCustomWindow();
		mCurrentOpenTool->setWindowIcon(QIcon(":/images/Mine/tool.png"));
		auto toolWin = new DClassifyManager();
		mCurrentOpenTool->addMainWidget(toolWin);
		mCurrentOpenTool->setCustomTitle(toolName);
		mCurrentOpenTool->show();
		return;
	}
	else if (toolName.compare("Image Visualize") == 0)
	{
		mCurrentOpenTool = new DCustomWindow();
		mCurrentOpenTool->setWindowIcon(QIcon(":/images/Mine/tool.png"));
		auto toolWin = new DImageViewer();
		mCurrentOpenTool->addMainWidget(toolWin);
		mCurrentOpenTool->setCustomTitle(toolName);
		mCurrentOpenTool->show();
		return;
	}
	else if(toolName.compare("ImageCollection Visualize") == 0)
	{
		mCurrentOpenTool = new DCustomWindow();
		mCurrentOpenTool->setWindowIcon(QIcon(":/images/Mine/tool.png"));
		auto toolWin = new DImgCollectionViewer();
		mCurrentOpenTool->addMainWidget(toolWin);
		mCurrentOpenTool->setCustomTitle(toolName);
		mCurrentOpenTool->show();
		return;
	}
	else
	{
		QMessageBox::critical(nullptr, "Open Error", "Can't Find this tool", QMessageBox::Ok);
		return;
	}
}

void DToolBoxManager::openAnalysisTool(const QString& toolName)
{
	
}

void DToolBoxManager::openHydrologyTool(const QString& toolName)
{
	// TODO
	if (toolName.compare("Water Pollution Calculate") == 0)
	{
		//mCurrentOpenTool = new DCustomWindow();
		//auto toolWin = new DClassifyManager();
		//mCurrentOpenTool->addMainWidget(toolWin);
		//mCurrentOpenTool->show();
		return;
	}
}

void DToolBoxManager::openLandsatTool(const QString& toolName)
{
	if (toolName.compare("Tasselled Cap Transformation") == 0)
	{
		mCurrentOpenTool = new DCustomWindow();
		mCurrentOpenTool->setWindowIcon(QIcon(":/images/Mine/tool.png"));
		auto toolWin = new DTasselledCapTransformation();
		mCurrentOpenTool->addMainWidget(toolWin);
		mCurrentOpenTool->setCustomTitle(toolName);
		mCurrentOpenTool->show();
		return;
	}

	if (toolName.compare("Calibration") == 0)
	{
		mCurrentOpenTool = new DCustomWindow();
		mCurrentOpenTool->setWindowIcon(QIcon(":/images/Mine/tool.png"));
		auto toolWin = new DLandsatCalibration();
		mCurrentOpenTool->addMainWidget(toolWin);
		mCurrentOpenTool->setCustomTitle(toolName);
		mCurrentOpenTool->show();
		return;
	}

	if (toolName.compare("Dark Pixel Atomosphere Calibartion") == 0)
	{
		mCurrentOpenTool = new DCustomWindow();
		mCurrentOpenTool->setWindowIcon(QIcon(":/images/Mine/tool.png"));
		auto toolWin = new DDarkPixelAtomosphereCalibration();
		mCurrentOpenTool->addMainWidget(toolWin);
		mCurrentOpenTool->setCustomTitle(toolName);
		mCurrentOpenTool->show();
		return;
	}
}


void DToolBoxManager::openPythonTool(const QString&)
{

}

void DToolBoxManager::openExportTool(const QString& toolName)
{
	if (toolName.compare("Exporter") == 0)
	{
		mCurrentOpenTool = new DCustomWindow();
		mCurrentOpenTool->setWindowIcon(QIcon(":/images/Mine/tool.png"));
		auto toolWin = new DGeeExporter();
		mCurrentOpenTool->addMainWidget(toolWin);
		mCurrentOpenTool->setCustomTitle(toolName);
		mCurrentOpenTool->show();
		return;
	}
}

void DToolBoxManager::openOthersTool(const QString&)
{

}