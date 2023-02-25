#include "DToolBox.h"
#include <QLabel>
#include <QSettings>

DToolBox::DToolBox(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	initGui();
	bingSignalsSlots();
}

DToolBox::~DToolBox()
{
}

void DToolBox::initGui()
{
	QVBoxLayout *vLayout = new QVBoxLayout();
	vLayout->setSpacing(6);
	vLayout->setContentsMargins(3, 3, 3, 3);
	ui.mPageAnalyze->setLayout(vLayout);

	vLayout = new QVBoxLayout();
	vLayout->setSpacing(6);
	vLayout->setContentsMargins(3, 3, 3, 3);
	ui.mPageExport->setLayout(vLayout);

	vLayout = new QVBoxLayout();
	vLayout->setSpacing(6);
	vLayout->setContentsMargins(3, 3, 3, 3);
	ui.mPageHydro->setLayout(vLayout);

	vLayout = new QVBoxLayout();
	vLayout->setSpacing(6);
	vLayout->setContentsMargins(3, 3, 3, 3);
	ui.mPageLandsat->setLayout(vLayout);

	vLayout = new QVBoxLayout();
	vLayout->setSpacing(6);
	vLayout->setContentsMargins(3, 3, 3, 3);
	ui.mPageOthers->setLayout(vLayout);

	vLayout = new QVBoxLayout();
	vLayout->setSpacing(6);
	vLayout->setContentsMargins(3, 3, 3, 3);
	ui.mPageRaster->setLayout(vLayout);

	vLayout = new QVBoxLayout();
	vLayout->setSpacing(6);
	vLayout->setContentsMargins(3, 3, 3, 3);
	ui.mPageVector->setLayout(vLayout);

	vLayout = new QVBoxLayout();
	vLayout->setSpacing(6);
	vLayout->setContentsMargins(3, 3, 3, 3);
	ui.mPagePython->setLayout(vLayout);

	loadToolsFromConfigFile();
}


void DToolBox::bingSignalsSlots()
{

}

void DToolBox::addOneTool(DToolBox::ToolType type, const QString& toolName, const QString& description)
{
	QHBoxLayout *hLayout = new QHBoxLayout();
	QLabel *pToolPixmapLabel = new QLabel();
	pToolPixmapLabel->setFixedSize(32, 32);
	QLabel *pToolNameLabel = new QLabel(toolName);
	QLabel *pToolDescLabel = new QLabel(description);
	QPushButton *btnOpen = new QPushButton("Open");
	QPixmap pixmap(":/images/Mine/tool.png");
	//pixmap.
	pToolPixmapLabel->setPixmap(pixmap.scaled(32, 32));
	QVBoxLayout *vLayout = new QVBoxLayout();
	pToolNameLabel->setStyleSheet(QString("color: white;"));
	pToolDescLabel->setStyleSheet("color: gray;");
	vLayout->setSpacing(3);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->addWidget(pToolNameLabel);
	vLayout->addWidget(pToolDescLabel);

	hLayout->addWidget(pToolPixmapLabel);
	hLayout->addLayout(vLayout);
	hLayout->addStretch();
	hLayout->addWidget(btnOpen);
	// hLayout->addStretch();

	bool toAdd = true;
	switch (type)
	{
	case DToolBox::Vector:
	{
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui.mPageVector->layout());
		layout->addLayout(hLayout);
	}
		break;
	case DToolBox::Raster:
	{
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui.mPageRaster->layout());
		layout->addLayout(hLayout);
	}
		break;
	case DToolBox::Analysis:
	{
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui.mPageAnalyze->layout());
		layout->addLayout(hLayout);
	}
		break;
	case DToolBox::Hydrology:
	{
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui.mPageHydro->layout());
		layout->addLayout(hLayout);
	}
		break;
	case DToolBox::Landsat:
	{
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui.mPageLandsat->layout());
		layout->addLayout(hLayout);
	}
		break;
	case DToolBox::Python:
	{
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui.mPagePython->layout());
		layout->addLayout(hLayout);
	}
		break;
	case DToolBox::Export:
	{
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui.mPageExport->layout());
		layout->addLayout(hLayout);
	}
		break;
	case DToolBox::Others:
	{
		QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui.mPageOthers->layout());
		layout->addLayout(hLayout);
	}
		break;
	default:
		toAdd = false;
		break;
	}
	if (toAdd)
	{
		connect(btnOpen, &QPushButton::clicked, [=] {
			emit toolClicked(type, toolName);
		});
	}
}


void DToolBox::loadToolsFromConfigFile()
{
	QSettings setting(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
	QString toolNames = setting.value("/toolBox/tools/INSTALLED").toString();
	QStringList toolNamesLst = toolNames.split("&");
	int count = toolNamesLst.count();
	if (count == 0) return;

	for each (QString str in toolNamesLst)
	{
		QStringList toolInfos = str.split(":");
		QString toolType = toolInfos.at(0);
		QString toolName = toolInfos.at(1);

		if (toolType.compare("Vector") == 0)
			addOneTool(ToolType::Vector, toolName, QStringLiteral("..."));
		else if (toolType.compare("Raster") == 0)
			addOneTool(ToolType::Raster, toolName, QStringLiteral("..."));
		else if (toolType.compare("Analysis") == 0)
			addOneTool(ToolType::Analysis, toolName, QStringLiteral("..."));
		else if (toolType.compare("Hydrology") == 0)
			addOneTool(ToolType::Hydrology, toolName, QStringLiteral("..."));
		else if (toolType.compare("Landsat") == 0)
			addOneTool(ToolType::Landsat, toolName, QStringLiteral("..."));
		else if (toolType.compare("Python") == 0)
			addOneTool(ToolType::Python, toolName, QStringLiteral("..."));
		else if (toolType.compare("Export") == 0)
			addOneTool(ToolType::Export, toolName, QStringLiteral("..."));
		else if (toolType.compare("Others") == 0)
			addOneTool(ToolType::Others, toolName, QStringLiteral("..."));
	}
}