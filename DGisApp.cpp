#include "DGisApp.h"
#include"subCode/DTitleBar.h"
#include "qsettings.h"
#include "qgsmapcanvas.h"
#include "qgslayertreelayer.h"
#include"qgslayertreeview.h"
#include"qgslayertreemodel.h"
#include "qgridlayout.h"
#include "qgsdockwidget.h"
#include"qshortcut.h"
#include"qgsapplication.h"
#include"qtoolbutton.h"
#include"qgslegendfilterbutton.h"
#include"qgsguiutils.h"
#include"qgslayertreemapcanvasbridge.h"
#include"qgscustomlayerorderwidget.h"
#include"qfiledialog.h"
#include"qgsvectorlayer.h"
#include"qgsrasterlayer.h"
#include"qmessagebox.h"
#include"qgslayertreeviewdefaultactions.h"
#include"qgssymbolselectordialog.h"
#include"qgslayertree.h"
#include"qgslayertreeutils.h"
#include"qgsstyle.h"
#include"qgis/qgsmapthemes.h"
#include"qgsmessagebar.h"
#include"qgis/qgslayerstylingwidget.h"
#include"qgis/qgsmaptoolselect.h"
#include"qgsuserinputwidget.h"
#include"qgstransaction.h"
#include"qgstransactiongroup.h"
#include"qgsapplication.h"
#include "qgstaskmanager.h"
#include"qgis/qgsguivectorlayertools.h"

#include"qgscoordinatereferencesystem.h"
#include"qgsproviderregistry.h"
#include"qgsdatumtransformdialog.h"
#include"qgsvectorlayerutils.h"
#include"qgis/qgsclipboard.h"
#include"qgis/qgsfixattributedialog.h"
#include"qgsmemoryproviderutils.h"
#include"qgsdatasourcemanagerdialog.h"
#include"qgsbrowserguimodel.h"
#include"qgsbrowserdockwidget.h"
//状态栏
#include"qgsstatusbar.h"
#include"qprogressbar.h"
#include"qgis/qgsstatusbarcoordinateswidget.h"
#include"qgis/qgsstatusbarscalewidget.h"
#include"qelapsedtimer.h"
#include"qtimer.h"
#include"qmetaobject.h"

//复制、粘贴操作
#include"qgsvectorfilewritertask.h"
#include"qgsmessageviewer.h"
#include"qgsfieldformatter.h"
#include"qgsgui.h"
#include"qgsfieldformatterregistry.h"
#include"qgseditorwidgetregistry.h"
#include"qgsdataitem.h"
#include"qgssublayersdialog.h"
#include"qinputdialog.h"
//pan、zoomIN...
#include"qgsmaptoolzoom.h"
#include"qgsmaptoolpan.h"
#include"qgis/qgsmaptoolidentifyaction.h"
#include"qgsfeaturestore.h"
#include"qgsmaptool.h"
#include"qgis/qgsmeasuretool.h"
#include"qgsadvanceddigitizingdockwidget.h"
//#include"qgsgeometryvalidator.h"

#include"qgis/qgsattributetabledialog.h"
//project & layer
#include"qgscustomprojectopenhandler.h"
#include"qgsziputils.h"
#include"qgsproject.h"
#include"qgsprojectviewsettings.h"
#include"qgsfileutils.h"
#include"qgsnewvectorlayerdialog.h"
#include"qgsnewmemorylayerdialog.h"
//#include"qgsmessagebaritem.h"

//python
#include"subCode/DPythonUtils.h"
#include"subCode/DInterface.h"
#include"pyInterface/DPyInterface.h"

// database
#include"databaseInterface/DDatabaseInterface.h"
#include"qgsnative.h"

//edit tools
#include"qgis/qgsmaptooladdfeature.h"
#include"qgis/qgsmaptoolsplitfeatures.h"
#include "qgis/qgsmaptoolmovefeature.h"
#include"subCode/DMapToolAddFeature.h"
// add data
#include"subCode/DAddDataset.h"
#include"subCode/DCustomWindow.h"
#include"DMapToolTemporyDrawer.h"
#include"DTemporyDrawerManager.h"
#include"DClassifyManager.h"
#include"DMapToolDrawSample.h"

// Layertreeview
#include"qgis/qgsapplayertreeviewmenuprovider.h"
#include"qgsprojectionselectiondialog.h"
#include"qgsrasterlayersaveasdialog.h"
#include"qgsrasterprojector.h"
#include"qgsrasterfilewritertask.h"
#include"qgsrasternuller.h"

#include "DWelComePage.h"
#include "DGeeObjectManager.h"
#include "DToolBoxManager.h"
#include "DToolBox.h"

DGisApp *DGisApp::sInstance = nullptr;

static QStringList splitSubLayerDef(const QString &subLayerDef)
{
	return subLayerDef.split(QgsDataProvider::sublayerSeparator());
}

static void setupVectorLayer(const QString &vectorLayerPath,
	const QStringList &sublayers,
	QgsVectorLayer *&layer,
	const QString &providerKey,
	QgsVectorLayer::LayerOptions options)
{
	//set friendly name for datasources with only one layer
	QgsSettings settings;
	QStringList elements = splitSubLayerDef(sublayers.at(0));
	QString rawLayerName = elements.size() >= 2 ? elements.at(1) : QString();
	QString subLayerNameFormatted = rawLayerName;
	if (settings.value(QStringLiteral("qgis/formatLayerName"), false).toBool())
	{
		subLayerNameFormatted = QgsMapLayer::formatLayerName(subLayerNameFormatted);
	}

	if (elements.size() >= 4 && layer->name().compare(rawLayerName, Qt::CaseInsensitive) != 0 && layer->name().compare(subLayerNameFormatted, Qt::CaseInsensitive) != 0)
	{
		layer->setName(QStringLiteral("%1 %2").arg(layer->name(), rawLayerName));
	}

	// Systematically add a layername= option to OGR datasets in case
	// the current single layer dataset becomes layer a multi-layer one.
	// Except for a few select extensions, known to be always single layer dataset.
	QFileInfo fi(vectorLayerPath);
	QString ext = fi.suffix().toLower();
	if (providerKey == QLatin1String("ogr") &&
		ext != QLatin1String("shp") &&
		ext != QLatin1String("mif") &&
		ext != QLatin1String("tab") &&
		ext != QLatin1String("csv") &&
		ext != QLatin1String("geojson") &&
		!vectorLayerPath.contains(QStringLiteral("layerid=")) &&
		!vectorLayerPath.contains(QStringLiteral("layername=")))
	{
		auto uriParts = QgsProviderRegistry::instance()->decodeUri(
			layer->providerType(), layer->dataProvider()->dataSourceUri());
		uriParts.insert(QStringLiteral("layerName"), rawLayerName);
		QString composedURI = QgsProviderRegistry::instance()->encodeUri(
			layer->providerType(), uriParts);

		auto newLayer = qgis::make_unique<QgsVectorLayer>(composedURI, layer->name(), QStringLiteral("ogr"), options);
		if (newLayer && newLayer->isValid())
		{
			delete layer;
			layer = newLayer.release();
		}
	}
}

DGisApp::DGisApp(DEnums::User* usr, QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	sInstance = this;
	// 设置自定义标题栏
	this->setWindowFlags(Qt::CustomizeWindowHint);
	this->setMouseTracking(true);
	mTitleBar = new DTitleBar(this);
	QMenuBar *menuBar = this->menuBar();
	mTitleBar->addMenuBar(menuBar);
	//mTitleBar->setFixedHeight(50);
	this->setMenuWidget(mTitleBar);
	
	//初始化mainShowWidget的布局
	QWidget *mainShowWidget = ui.mWidgetMainShow;
	QGridLayout *centralLayout = new QGridLayout(mainShowWidget);
	mainShowWidget->setLayout(centralLayout);
	centralLayout->setContentsMargins(0, 0, 0, 0);
	// 初始化mSetting
	mSettings = new QSettings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat, this);

	//初始化MapCanvas
	mMapCanvas = new QgsMapCanvas(mainShowWidget);
	mMapCanvas->setObjectName(QStringLiteral("theMapCanvas"));
	mMapCanvas->setCanvasColor(QColor(230, 230, 230));
	// set project linked to main canvas
	mMapCanvas->setProject(QgsProject::instance());
	//set the focus to the map canvas
	mMapCanvas->setFocus();


	//创建刷新阻拦对象，在该对象运行周期内(本函数)，静止地图刷新
	QgsCanvasRefreshBlocker refreshBlocker;

	// User Input Dock Widget
	mUserInputDockWidget = new QgsUserInputWidget(mMapCanvas);
	mUserInputDockWidget->setObjectName(QStringLiteral("UserInputDockWidget"));
	mUserInputDockWidget->setAnchorWidget(mMapCanvas);
	mUserInputDockWidget->setAnchorWidgetPoint(QgsFloatingWidget::TopRight);
	mUserInputDockWidget->setAnchorPoint(QgsFloatingWidget::TopRight);
	
	//初始化layertree
	mLayerTreeView = new QgsLayerTreeView(this);
	mLayerTreeView->setObjectName(QStringLiteral("theLayerTreeView"));

	mWelcomPage = new DWelComePage(usr,this);
	//mWelcomePage->setStyleSheet("background-color:green");

	//mwssagefobar初始化
	mInfoBar = new QgsMessageBar();
	mInfoBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	centralLayout->addWidget(mInfoBar, 0, 0, 1, 1);
	mInfoBar->raise();

	//connect(mMapCanvas, &QgsMapCanvas::layersChanged, this, &DGisApp::showMapCanvas); //dxl
	initLayerTreeView();

	// 开始创建界面风格停靠窗口
	mMapStylingDock = new QgsDockWidget(this);
	mMapStylingDock->setWindowTitle(tr("Layer Styling"));
	mMapStylingDock->setObjectName(QStringLiteral("LayerStyling"));
	QShortcut *showStylingDock = new QShortcut(QKeySequence(tr("Ctrl+3")), this);
	connect(showStylingDock, &QShortcut::activated, mMapStylingDock, &QgsDockWidget::toggleUserVisible);
	showStylingDock->setObjectName(QStringLiteral("ShowLayerStylingPanel"));
	showStylingDock->setWhatsThis(tr("Show Style Panel"));

	mMapStyleWidget = new QgsLayerStylingWidget(mMapCanvas, mInfoBar, mMapLayerPanelFactories);
	mMapStylingDock->setWidget(mMapStyleWidget);
	connect(mMapStyleWidget, &QgsLayerStylingWidget::styleChanged, this, &DGisApp::updateLabelToolButtons);
	connect(mMapStylingDock, &QDockWidget::visibilityChanged, mActionStyleDock, &QAction::setChecked);

	addDockWidget(Qt::RightDockWidgetArea, mMapStylingDock);
	mMapStylingDock->hide();

	// 初始化临时图形管理窗口
	mTemporyDrawerDock = new QgsDockWidget(this);
	mTemporyDrawerDock->setWindowTitle(tr("Tempory drawer manager"));
	mTemporyDrawerDock->setObjectName(QStringLiteral("Tempory_drawer_manager"));

	mTemporyDrawerManager = new DTemporyDrawerManager(mMapCanvas);
	mTemporyDrawerDock->setWidget(mTemporyDrawerManager);
	mTemporyDrawerDock->setAllowedAreas(Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, mTemporyDrawerDock);
	mTemporyDrawerDock->setVisible(false);

	// 初始化分类样本选择窗口
	mClassifyManagerDock = new QgsDockWidget(this);
	mClassifyManagerDock->setWindowTitle(tr("Classify manager"));
	mClassifyManagerDock->setObjectName(QStringLiteral("Classify_manager"));

	mClassifyManager = new DClassifyManager();
	mClassifyManagerDock->setWidget(mClassifyManager);
	mClassifyManagerDock->setAllowedAreas(Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, mClassifyManagerDock);
	mClassifyManagerDock->setVisible(false);
	//endProfile();

	//mBrowserModel = new QgsBrowserGuiModel(this);
	//mBrowserWidget = new QgsBrowserDockWidget(tr("Browser"), mBrowserModel, this);
	//mBrowserWidget->setObjectName(QStringLiteral("Browser"));
	//mBrowserWidget->setMessageBar(mInfoBar);
	//addDockWidget(Qt::RightDockWidgetArea, mBrowserWidget);
	//mBrowserWidget->hide();

	//build python interface
	//mPyInterfaceDock = new QDockWidget(this);
	//mPyInterfaceDock->setWindowTitle(tr("Python coder"));
	// 初始化python编辑界面
	mPyInterface = new DPyInterface();
	mPyInterface->setMainGisApp(this);

	//mPyInterfaceDock->setWidget(mPyInterface); //dxl 后续有需要可以更改
	//addDockWidget(Qt::BottomDockWidgetArea, mPyInterfaceDock);
	//mPyInterfaceDock->hide();

	// now build vector and raster file filters
	//mVectorFileFilter = QgsProviderRegistry::instance()->fileVectorFilters();
	//mRasterFileFilter = QgsProviderRegistry::instance()->fileRasterFilters();

	mVectorLayerTools = new QgsGuiVectorLayerTools();

	// 创建大多数地图工具、并建立连接
	createCanvasTools();
	// 创建状态栏、包含渲染进程、坐标、比例尺、日志记录
	createStatusBar();

	//---------------------------------------------------------4界面的添加、切换相关------------------------------------------------------------------------------------

	//创建主container，包含欢迎界面、编程界面、地图界面、数据库界面
	mCentralContainer = new QStackedWidget(mCentralContainer);
	mDbInterface = new DDatabaseInterface();
	mCentralContainer->addWidget(mWelcomPage);		// 0
	mCentralContainer->addWidget(mMapCanvas);		// 1	后续待补充绘图界面，与mMapCanvas同一index，使用tabWidget控件 TODO
	mCentralContainer->addWidget(mPyInterface);		// 2
	mCentralContainer->addWidget(mDbInterface);		// 3
	centralLayout->addWidget(mCentralContainer);

	mCentralContainer->setCurrentIndex(1);		// 默认打开的是地图界面
	connect(ui.mBtnToWelcom, &QPushButton::clicked, [=] {changeInterfaceTo(DGisApp::Interface::ToWelcome); });
	connect(ui.mBtnToMap, &QPushButton::clicked, [=] {changeInterfaceTo(DGisApp::Interface::ToMap); });
	connect(ui.mBtnToEditor, &QPushButton::clicked, [=] {changeInterfaceTo(DGisApp::Interface::ToCodeEditor); });
	connect(ui.mBtnToDbManager, &QPushButton::clicked, [=] {changeInterfaceTo(DGisApp::Interface::ToDatabase); });

	// mGeeObjectManager
	mGeeObjectManagerDock = new QDockWidget(this);
	mGeeObjectManagerDock->setWindowTitle(tr("GeeObj Manager"));
	mGeeObjectManagerDock->setObjectName(QStringLiteral("GeeObj_Manager"));

	mGeeObjectManager = DGeeObjectManager::Instance();
	mGeeObjectManagerDock->setWidget(mGeeObjectManager);
	mGeeObjectManagerDock->setAllowedAreas(Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, mGeeObjectManagerDock);
	mGeeObjectManagerDock->setVisible(false);

	// toolBox
	mDToolBoxDock = new QDockWidget(this);
	mDToolBoxDock->setWindowTitle(tr("ToolBox"));
	mDToolBoxDock->setObjectName(QStringLiteral("ToolBox"));

	mToolBoxManager = new DToolBoxManager();
	mDToolBox = new DToolBox();
	mDToolBoxDock->setWidget(mDToolBox);
	mDToolBoxDock->setAllowedAreas(Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, mDToolBoxDock);
	mDToolBoxDock->setVisible(false);

	connect(mDToolBox, &DToolBox::toolClicked, mToolBoxManager, &DToolBoxManager::openToolBox);


	// 初始化程序与python的交互接口
	mDInterface = new DInterface(this); // create the interface
	loadPythonSupport();
	bindSignalsSlots();
	testPython();
}

DGisApp::~DGisApp()
{
	// 1、首先先析构地图工具
	delete mMapTools.mZoomIn;
	delete mMapTools.mZoomOut;
	delete mMapTools.mPan;
	delete mMapTools.mIdentify;
	delete mMapTools.mMeasureDist;
	delete mMapTools.mMeasureArea;
	delete mMapTools.mAddFeature;
	delete mMapTools.mMoveFeature;
	delete mMapTools.mMoveFeatureCopy;
	delete mMapTools.mSplitFeatures;
	delete mMapTools.mSelectFeatures;
	delete mMapTools.mSelectPolygon;
	delete mMapTools.mSelectFreehand;

	// 2A、在析构各个自定义添加的widget
	delete mMapStyleWidget;
	//delete mDataSourceManagerDialog;
	delete mUserInputDockWidget;
	delete mInternalClipboard;
	delete mInfoBar;
	//delete mBrowserModel;
	delete mVectorLayerTools;
	delete editIuput;
	delete editOut;
	const QList<QgsMapCanvas *> canvases = mapCanvases();
	for (QgsMapCanvas *canvas : canvases)
	{
		delete canvas;
	}
	delete mToolBoxManager;
	delete mDToolBox;

	// 3、清楚其余指针
	delete mPythonRunner;
	delete mPyInterface;
	delete mTitleBar;
	//delete mDbInterface;

	// 4、清除且退出程序
	QgsGui::instance()->nativePlatformInterface()->cleanup();
	QgsApplication::exitQgis();
	//以下静止添加内容
}


//初始化layertree
void DGisApp::initLayerTreeView()
{
	mLayerTreeDock = new QgsDockWidget(tr("Layers"), this);
	mLayerTreeDock->setObjectName(QStringLiteral("Layers"));
	mLayerTreeDock->setAllowedAreas(Qt::RightDockWidgetArea);//Qt::LeftDockWidgetArea | 

	QShortcut *showLayersTreeDock = new QShortcut(QKeySequence(tr("Ctrl+1")), this);
	connect(showLayersTreeDock, &QShortcut::activated, mLayerTreeDock, &QgsDockWidget::toggleUserVisible);
	showLayersTreeDock->setObjectName(QStringLiteral("ShowLayersPanel"));
	showLayersTreeDock->setWhatsThis(tr("Show Layers Panel"));

	QgsLayerTreeModel *model = new QgsLayerTreeModel(QgsProject::instance()->layerTreeRoot(), this);

	model->setFlag(QgsLayerTreeModel::AllowNodeReorder);
	model->setFlag(QgsLayerTreeModel::AllowNodeRename);
	model->setFlag(QgsLayerTreeModel::AllowNodeChangeVisibility);
	model->setFlag(QgsLayerTreeModel::ShowLegendAsTree);
	model->setFlag(QgsLayerTreeModel::UseEmbeddedWidgets);
	model->setFlag(QgsLayerTreeModel::UseTextFormatting);
	model->setAutoCollapseLegendNodes(10);

	mLayerTreeView->setModel(model);
	mLayerTreeView->setMessageBar(mInfoBar);
	mLayerTreeView->setMenuProvider(new QgsAppLayerTreeViewMenuProvider(mLayerTreeView, mMapCanvas));

	//setupLayerTreeViewFromSettings();
	QFont fontLayer, fontGroup;
	fontLayer.setBold(true);
	fontGroup.setBold(false);
	model->setLayerTreeNodeFont(QgsLayerTreeNode::NodeLayer, fontLayer);
	model->setLayerTreeNodeFont(QgsLayerTreeNode::NodeGroup, fontGroup);

	connect(mLayerTreeView, &QAbstractItemView::doubleClicked, this, &DGisApp::layerTreeViewDoubleClicked);
	connect(mLayerTreeView, &QgsLayerTreeView::currentLayerChanged, this, &DGisApp::onActiveLayerChanged);
	connect(mLayerTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DGisApp::updateNewLayerInsertionPoint);
	connect(QgsProject::instance()->layerTreeRegistryBridge(), &QgsLayerTreeRegistryBridge::addedLayersToLayerTree,
		this, &DGisApp::autoSelectAddedLayer);

	// add group action
	QAction *actionAddGroup = new QAction(tr("Add Group"), this);
	actionAddGroup->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionAddGroup.svg")));
	actionAddGroup->setToolTip(tr("Add Group"));
	connect(actionAddGroup, &QAction::triggered, mLayerTreeView->defaultActions(), &QgsLayerTreeViewDefaultActions::addGroup);

	// visibility groups tool button
	QToolButton *btnVisibilityPresets = new QToolButton;
	btnVisibilityPresets->setAutoRaise(true);
	btnVisibilityPresets->setToolTip(tr("Manage Map Themes"));
	btnVisibilityPresets->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionShowAllLayers.svg")));
	btnVisibilityPresets->setPopupMode(QToolButton::InstantPopup);
	btnVisibilityPresets->setMenu(QgsMapThemes::instance()->menu());	//缺乏头文件	TODO

	// filter legend action
	mActionFilterLegend = new QAction(tr("Filter Legend by Map Content"), this);
	mActionFilterLegend->setCheckable(true);
	mActionFilterLegend->setToolTip(tr("Filter Legend by Map Content"));
	mActionFilterLegend->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionFilter2.svg")));
	connect(mActionFilterLegend, &QAction::toggled, this, &DGisApp::updateFilterLegend);

	mLegendExpressionFilterButton = new QgsLegendFilterButton(this);
	mLegendExpressionFilterButton->setToolTip(tr("Filter legend by expression"));
	connect(mLegendExpressionFilterButton, &QAbstractButton::toggled, this, &DGisApp::toggleFilterLegendByExpression);

	mActionStyleDock = new QAction(tr("Layer Styling"), this);
	mActionStyleDock->setCheckable(true);
	mActionStyleDock->setToolTip(tr("Open the Layer Styling panel"));
	mActionStyleDock->setShortcut(QStringLiteral("F7"));
	mActionStyleDock->setIcon(QgsApplication::getThemeIcon(QStringLiteral("propertyicons/symbology.svg")));
	connect(mActionStyleDock, &QAction::toggled, this, &DGisApp::mapStyleDock);

	// expand / collapse tool buttons
	QAction *actionExpandAll = new QAction(tr("Expand All"), this);
	actionExpandAll->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionExpandTree.svg")));
	actionExpandAll->setToolTip(tr("Expand All"));
	connect(actionExpandAll, &QAction::triggered, mLayerTreeView, &QgsLayerTreeView::expandAllNodes);
	QAction *actionCollapseAll = new QAction(tr("Collapse All"), this);
	actionCollapseAll->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mActionCollapseTree.svg")));
	actionCollapseAll->setToolTip(tr("Collapse All"));
	connect(actionCollapseAll, &QAction::triggered, mLayerTreeView, &QgsLayerTreeView::collapseAllNodes);

	QToolBar *toolbar = new QToolBar();
	toolbar->setIconSize(iconSize(true));
	toolbar->setIconSize(iconSize(true));
	toolbar->addAction(mActionStyleDock);
	toolbar->addAction(actionAddGroup);
	toolbar->addWidget(btnVisibilityPresets);
	toolbar->addAction(mActionFilterLegend);
	toolbar->addWidget(mLegendExpressionFilterButton);
	toolbar->addAction(actionExpandAll);
	toolbar->addAction(actionCollapseAll);
	toolbar->addAction(ui.mActionRemoveLayer);

	// mLayerTreeDock >> w >> vboxLayout >> toolbar + mLayerTreeView
	QVBoxLayout *vboxLayout = new QVBoxLayout;
	vboxLayout->setContentsMargins(0, 0, 0, 0);
	vboxLayout->setSpacing(0);
	vboxLayout->addWidget(toolbar);
	vboxLayout->addWidget(mLayerTreeView);

	QWidget *w = new QWidget;
	w->setStyleSheet("*{color: rgb(234, 234, 234); background-color: rgb(51, 51, 55);} QTreeView::item{color:rgb(234, 234, 234)} QToolTip{color: rgb(50, 50, 50); background-color: rgb(245, 245, 245);}");
	w->setLayout(vboxLayout);
	mLayerTreeDock->setWidget(w);
	//mLayerTreeDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::RightDockWidgetArea, mLayerTreeDock);

	mLayerTreeCanvasBridge = new QgsLayerTreeMapCanvasBridge(QgsProject::instance()->layerTreeRoot(), mMapCanvas, this);

	mMapLayerOrder = new QgsCustomLayerOrderWidget(mLayerTreeCanvasBridge, this);
	mMapLayerOrder->setStyleSheet("*{color: rgb(234, 234, 234); background-color: rgb(51, 51, 55);}");
	mMapLayerOrder->setObjectName(QStringLiteral("theMapLayerOrder"));

	mLayerOrderDock = new QgsDockWidget(tr("Layer Order"), this);
	mLayerOrderDock->setObjectName(QStringLiteral("LayerOrder"));
	mLayerOrderDock->setAllowedAreas(Qt::RightDockWidgetArea);//Qt::LeftDockWidgetArea | 

	QShortcut *showLayerOrderDock = new QShortcut(QKeySequence(tr("Ctrl+9")), this);
	connect(showLayerOrderDock, &QShortcut::activated, mLayerOrderDock, &QgsDockWidget::toggleUserVisible);
	showLayerOrderDock->setObjectName(QStringLiteral("ShowLayerOrderPanel"));
	showLayerOrderDock->setWhatsThis(tr("Show Layer Order Panel"));

	mLayerOrderDock->setWidget(mMapLayerOrder);
	addDockWidget(Qt::RightDockWidgetArea, mLayerOrderDock);
	mLayerOrderDock->hide();


	// Advanced Digitizing dock
	//startProfile(tr("Advanced digitize panel"));
	mAdvancedDigitizingDockWidget = new QgsAdvancedDigitizingDockWidget(mMapCanvas, this);
	mAdvancedDigitizingDockWidget->setWindowTitle(tr("Advanced Digitizing"));
	mAdvancedDigitizingDockWidget->setObjectName(QStringLiteral("AdvancedDigitizingTools"));

	QShortcut *showAdvancedDigitizingDock = new QShortcut(QKeySequence(tr("Ctrl+4")), this);
	connect(showAdvancedDigitizingDock, &QShortcut::activated, mAdvancedDigitizingDockWidget, &QgsDockWidget::toggleUserVisible);
	showAdvancedDigitizingDock->setObjectName(QStringLiteral("ShowAdvancedDigitizingPanel"));
	showAdvancedDigitizingDock->setWhatsThis(tr("Show Advanced Digitizing Panel"));
	mAdvancedDigitizingDockWidget->hide();

	//初始化粘贴板
	mInternalClipboard = new QgsClipboard; // create clipboard
	//connect(mInternalClipboard, &QgsClipboard::changed, this, &DGisApp::clipboardChanged);	//可以不用

	//connect(mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &DGisApp::updateFilterLegend);
	//connect(mMapCanvas, &QgsMapCanvas::renderErrorOccurred, badLayerIndicatorProvider, &QgsLayerTreeViewBadLayerIndicatorProvider::reportLayerError);
}

QSettings * DGisApp::getSetting()
{
	return mSettings;
}


void DGisApp::createStatusBar()
{
	//remove borders from children under Windows
	statusBar()->setStyleSheet(QStringLiteral("QStatusBar::item {border: none;}"));

	// Drop the font size in the status bar by a couple of points
	QFont statusBarFont = font();
	int fontSize = statusBarFont.pointSize();
	fontSize = std::max(fontSize - 1, 8); 
	statusBar()->setFont(statusBarFont);

	mStatusBar = new QgsStatusBar();
	mStatusBar->setParentStatusBar(QMainWindow::statusBar());
	mStatusBar->setFont(statusBarFont);

	statusBar()->addPermanentWidget(mStatusBar, 10);
	statusBar()->setStyleSheet("color: rgb(241, 241, 241);background-color: rgb(45, 45, 48);");
	// Add a panel to the status bar for the scale, coords and progress
	// And also rendering suppression checkbox
	mProgressBar = new QProgressBar(mStatusBar);
	mProgressBar->setStyleSheet("QProgressBar{ border-color:blue; background:rgb(54, 54, 54); border-radius:5px; text-align:center; color: rgb(229, 229, 229); }QProgressBar::chunk{ background-color:rgb(58, 154, 255); border-radius:4px; }");
	mProgressBar->setObjectName(QStringLiteral("mProgressBar"));
	mProgressBar->setMaximumWidth(150);
	mProgressBar->setMinimumWidth(100);
	mProgressBar->setMaximumHeight(18);
	mProgressBar->setMinimumHeight(16);
	mProgressBar->hide();
	mStatusBar->addPermanentWidget(mProgressBar, 1);

	connect(mMapCanvas, &QgsMapCanvas::renderStarting, this, &DGisApp::canvasRefreshStarted);
	connect(mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, this, &DGisApp::canvasRefreshFinished);
	//coords status bar widget
	mCoordsEdit = new QgsStatusBarCoordinatesWidget(mStatusBar);
	mCoordsEdit->setMouseCoordinatesPrecision(2);
	mCoordsEdit->setObjectName(QStringLiteral("mCoordsEdit"));
	mCoordsEdit->setStyleSheet("border-radius:3px; color: rgb(234, 234, 234); background-color: rgb(51, 51, 55);");
	mCoordsEdit->setMapCanvas(mMapCanvas);
	mCoordsEdit->setFont(statusBarFont);
	mStatusBar->addPermanentWidget(mCoordsEdit, 0);

	mScaleWidget = new QgsStatusBarScaleWidget(mMapCanvas, mStatusBar);
	mScaleWidget->setStyleSheet("*,QComboBox,QComboBox::item{border-radius:3px; color: rgb(234, 234, 234); background-color: rgb(51, 51, 55);}");
	mScaleWidget->setObjectName(QStringLiteral("mScaleWidget"));
	mScaleWidget->setFont(statusBarFont);
	mStatusBar->addPermanentWidget(mScaleWidget, 0);

	mMessageLog = new QToolButton(mStatusBar);
	mMessageLog->setAutoRaise(true);
	mMessageLog->setIcon(QgsApplication::getThemeIcon(QStringLiteral("/mMessageLogRead.svg")));
	mMessageLog->setToolTip(tr("Messages"));
	mMessageLog->setObjectName(QStringLiteral("mMessageLogViewerButton"));
	mMessageLog->setMaximumHeight(mScaleWidget->height());
	mMessageLog->setCheckable(true);
	mStatusBar->addPermanentWidget(mMessageLog, 0);
}

// slot to update the progress bar in the status bar
void DGisApp::showProgress(int progress, int totalSteps)
{
	if (progress == totalSteps)
	{
		mProgressBar->reset();
		mProgressBar->hide();
	}
	else
	{
		//only call show if not already hidden to reduce flicker
		if (!mProgressBar->isVisible())
		{
			mProgressBar->show();
		}
		mProgressBar->setMaximum(totalSteps);
		mProgressBar->setValue(progress);
	}
}

void DGisApp::canvasRefreshStarted()
{
	mLastRenderTime.restart();
	// if previous render took less than 0.5 seconds, delay the appearance of the
	// render in progress status bar by 0.5 seconds - this avoids the status bar
	// rapidly appearing and then disappearing for very fast renders
	if (mLastRenderTimeSeconds > 0 && mLastRenderTimeSeconds < 0.5)
	{
		mRenderProgressBarTimer.setSingleShot(true);
		mRenderProgressBarTimer.setInterval(500);
		disconnect(mRenderProgressBarTimerConnection);
		mRenderProgressBarTimerConnection = connect(&mRenderProgressBarTimer, &QTimer::timeout, this, [=]() {
			showProgress(-1, 0);
		});
		mRenderProgressBarTimer.start();
	}
	else
	{
		showProgress(-1, 0); // trick to make progress bar show busy indicator
	}
}

void DGisApp::canvasRefreshFinished()
{
	mRenderProgressBarTimer.stop();
	mLastRenderTimeSeconds = mLastRenderTime.elapsed() / 1000.0;
	showProgress(0, 0); // stop the busy indicator
}

// 欢迎界面、地图界面、编程界面、数据管理界面的切换
void DGisApp::changeInterfaceTo(DGisApp::Interface option)
{
	switch (option)
	{
	case DGisApp::ToWelcome:
	{
		mCentralContainer->setCurrentIndex(0);
		mLayerTreeDock->hide();
		mLayerOrderDock->hide();
	}
		break;
	case DGisApp::ToMap:
		mCentralContainer->setCurrentIndex(1);
		mLayerTreeDock->show();
		break;
	case DGisApp::ToCodeEditor:
		mCentralContainer->setCurrentIndex(2);
		mLayerTreeDock->hide();
		mLayerOrderDock->hide();
		break;
	case DGisApp::ToDatabase:
		mCentralContainer->setCurrentIndex(3);
		mLayerTreeDock->hide();
		mLayerOrderDock->hide();
		break;
	default:
		break;
	}
}


void DGisApp::onActiveLayerChanged(QgsMapLayer *layer)
{
	const QList<QgsMapCanvas *> canvases = mapCanvases();
	for (QgsMapCanvas *canvas : canvases)
		canvas->setCurrentLayer(layer);

	emit activeLayerChanged(layer);
}


void DGisApp::copySelectionToClipboard(QgsMapLayer *layerContainingSelection)
{
	QgsVectorLayer *selectionVectorLayer = qobject_cast<QgsVectorLayer *>(layerContainingSelection ? layerContainingSelection : activeLayer());
	if (!selectionVectorLayer)
		return;

	// Test for feature support in this layer
	clipboard()->replaceWithCopyOf(selectionVectorLayer);
}

//void DGisApp::setupLayerTreeViewFromSettings()
//{
//	QgsSettings s;
//
//	QgsLayerTreeModel *model = mLayerTreeView->layerTreeModel();
//	QFont fontLayer, fontGroup;
//	fontLayer.setBold(true);
//	fontGroup.setBold(false);
//	model->setLayerTreeNodeFont(QgsLayerTreeNode::NodeLayer, fontLayer);
//	model->setLayerTreeNodeFont(QgsLayerTreeNode::NodeGroup, fontGroup);
//}

QList<QgsMapCanvas*> DGisApp::mapCanvases()
{
	// filter out browser canvases -- they are children of app, but a different
	// kind of beast, and here we only want the main canvas or dock canvases
	auto canvases = findChildren<QgsMapCanvas *>();
	canvases.erase(std::remove_if(canvases.begin(), canvases.end(),
		[](QgsMapCanvas *canvas) {
		return !canvas || canvas->property("browser_canvas").toBool();
	}),
		canvases.end());
	return canvases;
}


void DGisApp::freezeCanvases(bool frozen)
{
	const auto canvases = mapCanvases();
	for (QgsMapCanvas *canvas : canvases)
	{
		canvas->freeze(frozen);
	}
}

void DGisApp::refreshMapCanvas(bool redrawAllLayers)
{
	const auto canvases = mapCanvases();
	for (QgsMapCanvas *canvas : canvases)
	{
		//stop any current rendering
		canvas->stopRendering();
		if (redrawAllLayers)
			canvas->refreshAllLayers();
		else
			canvas->refresh();
	}
}


QSize DGisApp::iconSize(bool dockedToolbar) const
{
	return QgsGuiUtils::iconSize(dockedToolbar);
}


void DGisApp::slotAddVectorLayer(const QString &fileName, const QString &encode, const QString &providerKey)
{
	QStringList temp = fileName.split(QDir::separator());
	QString basename = temp.at(temp.size() - 1);
	addVectorLayerPrivate(fileName, basename, "ogr");
	//QgsVectorLayer* vecLayer = new QgsVectorLayer(fileName, basename, "ogr");// , false);
	//if (!vecLayer->isValid())
	//{
	//	QMessageBox::critical(this, "error", "layer is invalid");
	//	return;
	//}
	//vecLayer->setProviderEncoding(encode);
	//QgsProject::instance()->addMapLayer(vecLayer);
	//mMapCanvas->setExtent(vecLayer->extent());
}

void DGisApp::slotAddRasterLayer(const QString &fileName)
{
	QStringList temp = fileName.split(QDir::separator());
	QString basename = temp.at(temp.size() - 1);
	addRasterLayerPrivate(fileName, basename, "gdal", true, true);
	//QgsRasterLayer* rasterLayer = new QgsRasterLayer(fileName, basename, "gdal");
	//if (!rasterLayer->isValid())
	//{
	//	QMessageBox::critical(this, "error", "layer is invalid");
	//	return;
	//}

	//QgsProject::instance()->addMapLayer(rasterLayer);
	//mMapCanvas->setExtent(rasterLayer->extent());
}


void DGisApp::layerTreeViewDoubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index)
		QgsSettings settings;
	switch (settings.value(QStringLiteral("qgis/legendDoubleClickAction"), 0).toInt())
	{
	case 0:
	{
		//show properties
		if (mLayerTreeView)
		{
			// if it's a legend node, open symbol editor directly
			if (QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>(mLayerTreeView->currentLegendNode()))
			{
				const QgsSymbol *originalSymbol = node->symbol();
				if (!originalSymbol)
					return;

				std::unique_ptr<QgsSymbol> symbol(originalSymbol->clone());
				QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(node->layerNode()->layer());
				QgsSymbolSelectorDialog dlg(symbol.get(), QgsStyle::defaultStyle(), vlayer, this);
				QgsSymbolWidgetContext context;
				context.setMapCanvas(mMapCanvas);
				//context.setMessageBar(mInfoBar);
				dlg.setContext(context);
				if (dlg.exec())
				{
					node->setSymbol(symbol.release());
				}

				return;
			}
		}
		//DGisApp::instance()->layerProperties();	//dxl
		break;
	}
	case 1:
	{
		//QgsSettings settings;
		//QgsAttributeTableFilterModel::FilterMode initialMode = settings.enumValue(QStringLiteral("qgis/attributeTableBehavior"), QgsAttributeTableFilterModel::ShowAll);
		//DGisApp::instance()->attributeTable(initialMode);
		break;
	}
	case 2:
		//mapStyleDock(true);
		break;
	default:
		break;
	}
}


void DGisApp::bindSignalsSlots()
{
	// 链接按钮点击信号
	connect(ui.mActionAddOgrLayer, &QAction::triggered, [=] {
		openAddDatasetManager(0);
	});
	connect(ui.mActionAddRasterLayer, &QAction::triggered, [=] {
		openAddDatasetManager(1);
	});
	connect(ui.mActionAddWmsLayer, &QAction::triggered, [=] {
		openAddDatasetManager(2);
	});
	connect(ui.mActionNewVectorLayer, &QAction::triggered, this, &DGisApp::newVectorLayer);
	connect(ui.mActionNewMemoryLayer, &QAction::triggered, this, &DGisApp::newMemoryLayer);

	// connect legend signals
	//connect(this, &DGisApp::activeLayerChanged,this, &DGisApp::activateDeactivateLayerRelatedActions);
	//project open
	connect(ui.mActionNewProject, &QAction::triggered, this, [=] { fileNew(); });
	connect(ui.mActionOpenProject, &QAction::triggered, this, &DGisApp::fileOpen);
	connect(ui.mActionSaveProject, &QAction::triggered, this, &DGisApp::fileSave);

	connect(ui.mActionShowAllLayers, &QAction::triggered, this, &DGisApp::showAllLayers);
	connect(ui.mActionHideAllLayers, &QAction::triggered, this, &DGisApp::hideAllLayers);
	connect(ui.mActionShowSelectedLayers, &QAction::triggered, this, &DGisApp::showSelectedLayers);
	connect(ui.mActionHideSelectedLayers, &QAction::triggered, this, &DGisApp::hideSelectedLayers);
	connect(ui.mActionToggleSelectedLayers, &QAction::triggered, this, &DGisApp::toggleSelectedLayers);
	connect(ui.mActionToggleSelectedLayersIndependently, &QAction::triggered, this, &DGisApp::toggleSelectedLayersIndependently);
	connect(ui.mActionHideDeselectedLayers, &QAction::triggered, this, &DGisApp::hideDeselectedLayers);

	connect(ui.mActionRemoveLayer, &QAction::triggered, this, &DGisApp::removeLayer);
	connect(this, &DGisApp::activeLayerChanged, this, &DGisApp::setMapStyleDockLayer);
	//select functions
	connect(ui.mActionDeselectAll, &QAction::triggered, this, &DGisApp::deselectAll);
	connect(ui.mActionDeselectActiveLayer, &QAction::triggered, this, &DGisApp::deselectActiveLayer);
	connect(ui.mActionInvertSelection, &QAction::triggered, this, &DGisApp::invertSelection);
	connect(ui.mActionSelectAll, &QAction::triggered, this, &DGisApp::selectAll);
	connect(ui.mActionReselect, &QAction::triggered, this, [=] {
		QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(mMapCanvas->currentLayer());
		if (!vlayer)
		{
			messageBar()->pushMessage(
				tr("No active vector layer"),
				tr("To reselect features, choose a vector layer in the legend."),
				Qgis::Info,
				messageTimeout());
			return;
		}
		vlayer->reselect();
	});

	// copy paste cut
	connect(ui.mActionCutFeatures, &QAction::triggered, this, [=] { cutSelectionToClipboard(); });
	connect(ui.mActionCopyFeatures, &QAction::triggered, this, [=] { copySelectionToClipboard(); });
	connect(ui.mActionPasteFeatures, &QAction::triggered, this, [=] { pasteFromClipboard(); });
	connect(ui.mActionPasteAsNewVector, &QAction::triggered, this, &DGisApp::pasteAsNewVector);
	connect(ui.mActionPasteAsNewMemoryVector, &QAction::triggered, this, [=] { pasteAsNewMemoryVector(); });
	connect(ui.mActionDeleteSelected, &QAction::triggered, this, [=] { deleteSelected(nullptr, nullptr, true); });

	connect(ui.mActionOpenTable, &QAction::triggered, this, [=] {
		QgsSettings settings;
		QgsAttributeTableFilterModel::FilterMode initialMode = settings.enumValue(QStringLiteral("qgis/attributeTableBehavior"), QgsAttributeTableFilterModel::ShowAll);
		attributeTable(initialMode);
	});
	//clicked showPythondialog to show or hide python dialog
	//connect(ui.mActionShowPythonDialog, &QAction::triggered, [=] {mPyInterfaceDock->setVisible(!mPyInterface->isVisible()); });
	//connect(ui.mActionAddWmsLayer, &QAction::triggered, this, [=] { dataSourceManager(QStringLiteral("wms")); });

	// 进行加载数据窗口的信号绑定
	connect(ui.mActionDataSourceManager, &QAction::triggered, [=] {openAddDatasetManager(0); });

	// qgsproject
	connect(QgsProject::instance(), &QgsProject::crsChanged, this, &DGisApp::projectCrsChanged);
	connect(QgsProject::instance(), &QgsProject::layersAdded,
		this, &DGisApp::layersWereAdded);

	connect(ui.mActionDraw, &QAction::triggered, this, [=] { refreshMapCanvas(); });
}

// 打开数据加载管理界面，进行数据的加载
void DGisApp::openAddDatasetManager(int currentIndex)
{
	// 如果界面已经打开，但只是不在顶层，则将其置于顶层
	if (mToolContainer && mToolContainer->isVisible())
	{
		mToolContainer->raise();
		return;
	}
	// 保证每次打开的窗体都重新初始化
	if (mToolContainer)
		delete mToolContainer;
	mToolContainer = new DCustomWindow();
	DAddDataset *w = new DAddDataset(nullptr);
	w->setCurrentIndex(currentIndex);

	connect(w, &DAddDataset::addVectorLayer, this, &DGisApp::slotAddVectorLayer);
	connect(w, &DAddDataset::addRasterLayer, this, &DGisApp::slotAddRasterLayer);
	connect(w, &DAddDataset::testCurrentLink, this, &DGisApp::testWmsLink);
	connect(w, &DAddDataset::addWmsLayer, this, &DGisApp::slotAddWmsLayer);

	mToolContainer->addMainWidget(w);
	mToolContainer->setWindowTitle("Add Dataset");
	QIcon icon(":/images/Mine/toolbox.png");
	mToolContainer->setWindowIcon(icon);
	mToolContainer->raise();
	mToolContainer->show();
}


// 打开当前图层的属性表，如果当前图层不存在则退出
void DGisApp::attributeTable(QgsAttributeTableFilterModel::FilterMode filter)
{
	QgsVectorLayer *myLayer = qobject_cast<QgsVectorLayer *>(activeLayer());
	if (!myLayer)
	{
		return;
	}

	QgsAttributeTableDialog *mDialog = new QgsAttributeTableDialog(myLayer, filter);
	mDialog->show();
	// the dialog will be deleted by itself on close
}


// 设置激活图层，默认新加入的图层为激活图层
bool DGisApp::setActiveLayer(QgsMapLayer *layer)
{
	if (!layer)
		return false;

	if (!mLayerTreeView->layerTreeModel()->rootGroup()->findLayer(layer->id()))
		return false;

	mLayerTreeView->setCurrentLayer(layer);
	return true;
}


// 判断一个链接是否合法
void DGisApp::testWmsLink(const QString &URL, const QString &layerName, bool *res)
{
	//addRasterLayerPrivate(URL, layerName, "wms",true, true);
	QgsRasterLayer *layer = new QgsRasterLayer(URL, layerName, "wms");
	bool ok = layer->isValid();
	if (ok)
	{
		*res = true;
		mPlanToLoadRasterLayer = layer;
	}
	else
	{
		*res = false;
		delete layer;
	}

}


// 下列函数禁止非DAdddataset实例调用
void DGisApp::slotAddWmsLayer()
{
	if (!mPlanToLoadRasterLayer || !mPlanToLoadRasterLayer->isValid()) return;
	QgsProject::instance()->addMapLayer(mPlanToLoadRasterLayer);
	mMapCanvas->setExtent(mPlanToLoadRasterLayer->extent());
}


// 当某些工具开始使用时，打开或者关闭某些控件
void DGisApp::activateDeactivateLayerRelatedActions(QgsMapLayer *layer)
{
	//TODO
}


// 更新图层结点
void DGisApp::updateNewLayerInsertionPoint()
{
	QgsLayerTreeRegistryBridge::InsertionPoint insertionPoint = layerTreeInsertionPoint();
	QgsProject::instance()->layerTreeRegistryBridge()->setLayerInsertionPoint(insertionPoint);
}


//图层树插入结点响应槽
QgsLayerTreeRegistryBridge::InsertionPoint DGisApp::layerTreeInsertionPoint() const
{
	// defaults
	QgsLayerTreeGroup *insertGroup = mLayerTreeView->layerTreeModel()->rootGroup();
	QModelIndex current = mLayerTreeView->currentIndex();

	int index = 0;

	if (current.isValid())
	{
		index = current.row();

		QgsLayerTreeNode *currentNode = mLayerTreeView->currentNode();
		if (currentNode)
		{
			// if the insertion point is actually a group, insert new layers into the group
			if (QgsLayerTree::isGroup(currentNode))
			{
				// if the group is embedded go to the first non-embedded group, at worst the top level item
				QgsLayerTreeGroup *insertGroup = QgsLayerTreeUtils::firstGroupWithoutCustomProperty(QgsLayerTree::toGroup(currentNode), QStringLiteral("embedded"));

				return QgsLayerTreeRegistryBridge::InsertionPoint(insertGroup, 0);
			}

			// otherwise just set the insertion point in front of the current node
			QgsLayerTreeNode *parentNode = currentNode->parent();
			if (QgsLayerTree::isGroup(parentNode))
			{
				// if the group is embedded go to the first non-embedded group, at worst the top level item
				QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup(parentNode);
				insertGroup = QgsLayerTreeUtils::firstGroupWithoutCustomProperty(parentGroup, QStringLiteral("embedded"));
				if (parentGroup != insertGroup)
					index = 0;
			}
		}
	}
	return QgsLayerTreeRegistryBridge::InsertionPoint(insertGroup, index);
}


//当添加图层时，自动选中图层
void DGisApp::autoSelectAddedLayer(QList<QgsMapLayer *> layers)
{
	if (!layers.isEmpty())
	{
		QgsLayerTreeLayer *nodeLayer = QgsProject::instance()->layerTreeRoot()->findLayer(layers[0]->id());

		if (!nodeLayer)
			return;

		QModelIndex index = mLayerTreeView->layerTreeModel()->node2index(nodeLayer);
		mLayerTreeView->setCurrentIndex(index);
	}
}


// 更新树控件的legend
void DGisApp::updateFilterLegend()
{
	bool hasExpressions = mLegendExpressionFilterButton->isChecked() && QgsLayerTreeUtils::hasLegendFilterExpression(*mLayerTreeView->layerTreeModel()->rootGroup());
	if (mActionFilterLegend->isChecked() || hasExpressions)
	{
		layerTreeView()->layerTreeModel()->setLegendFilter(&mMapCanvas->mapSettings(),
			/* useExtent */ mActionFilterLegend->isChecked(),
			/* polygon */ QgsGeometry(),
			hasExpressions);
	}
	else
	{
		layerTreeView()->layerTreeModel()->setLegendFilterByMap(nullptr);
	}
}


// 图层返回树控件
QgsLayerTreeView * DGisApp::layerTreeView()
{
	Q_ASSERT(mLayerTreeView);
	return mLayerTreeView;
}


void DGisApp::toggleFilterLegendByExpression(bool checked)
{
	QgsLayerTreeNode *node = mLayerTreeView->currentNode();
	if (!node)
		return;

	if (QgsLayerTree::isLayer(node))
	{
		QString e = mLegendExpressionFilterButton->expressionText();
		QgsLayerTreeUtils::setLegendFilterByExpression(*QgsLayerTree::toLayer(node), e, checked);
	}

	updateFilterLegend();
}


void DGisApp::hideAllLayers()
{
	QgsDebugMsgLevel(QStringLiteral("hiding all layers!"), 3);

	const auto constChildren = mLayerTreeView->layerTreeModel()->rootGroup()->children();
	for (QgsLayerTreeNode *node : constChildren)
	{
		node->setItemVisibilityCheckedRecursive(false);
	}
}


void DGisApp::showAllLayers()
{
	QgsDebugMsgLevel(QStringLiteral("Showing all layers!"), 3);
	mLayerTreeView->layerTreeModel()->rootGroup()->setItemVisibilityCheckedRecursive(true);
}


void DGisApp::hideSelectedLayers()
{
	QgsDebugMsgLevel(QStringLiteral("hiding selected layers!"), 3);

	const auto constSelectedNodes = mLayerTreeView->selectedNodes();
	for (QgsLayerTreeNode *node : constSelectedNodes)
	{
		node->setItemVisibilityChecked(false);
	}
}


void DGisApp::toggleSelectedLayers()
{
	QgsDebugMsgLevel(QStringLiteral("toggling selected layers!"), 3);

	const auto constSelectedNodes = mLayerTreeView->selectedNodes();
	if (!constSelectedNodes.isEmpty())
	{
		bool isFirstNodeChecked = constSelectedNodes[0]->itemVisibilityChecked();
		for (QgsLayerTreeNode *node : constSelectedNodes)
		{
			node->setItemVisibilityChecked(!isFirstNodeChecked);
		}
	}
}


void DGisApp::toggleSelectedLayersIndependently()
{
	QgsDebugMsgLevel(QStringLiteral("toggling selected layers independently!"), 3);

	const auto constSelectedNodes = mLayerTreeView->selectedNodes();
	if (!constSelectedNodes.isEmpty())
	{
		for (QgsLayerTreeNode *node : constSelectedNodes)
		{
			node->setItemVisibilityChecked(!node->itemVisibilityChecked());
		}
	}
}


void DGisApp::hideDeselectedLayers()
{
	QList<QgsLayerTreeLayer *> selectedLayerNodes = mLayerTreeView->selectedLayerNodes();

	const auto constFindLayers = mLayerTreeView->layerTreeModel()->rootGroup()->findLayers();
	for (QgsLayerTreeLayer *nodeLayer : constFindLayers)
	{
		if (selectedLayerNodes.contains(nodeLayer))
			continue;
		nodeLayer->setItemVisibilityChecked(false);
	}
}


void DGisApp::showSelectedLayers()
{
	QgsDebugMsgLevel(QStringLiteral("show selected layers!"), 3);

	const auto constSelectedNodes = mLayerTreeView->selectedNodes();
	for (QgsLayerTreeNode *node : constSelectedNodes)
	{
		QgsLayerTreeNode *nodeIter = node;
		while (nodeIter)
		{
			nodeIter->setItemVisibilityChecked(true);
			nodeIter = nodeIter->parent();
		}
	}
}


void DGisApp::mapStyleDock(bool enabled)
{
	mMapStylingDock->setUserVisible(enabled);
	setMapStyleDockLayer(activeLayer());
}


void DGisApp::updateLabelToolButtons()
{
	bool enableMove = false, enableRotate = false, enablePin = false, enableShowHide = false, enableChange = false;

	QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
	for (QMap<QString, QgsMapLayer *>::iterator it = layers.begin(); it != layers.end(); ++it)
	{
		QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(it.value());
		if (vlayer && (vlayer->diagramsEnabled() || vlayer->labelsEnabled()))
		{
			enablePin = true;
			enableShowHide = true;
			enableMove = true;
			enableRotate = true;
			enableChange = true;

			break;
		}
	}
	//TODO
	//mActionPinLabels->setEnabled(enablePin);
	//mActionShowHideLabels->setEnabled(enableShowHide);
	//mActionMoveLabel->setEnabled(enableMove);
	//mActionRotateLabel->setEnabled(enableRotate);
	//mActionChangeLabelProperties->setEnabled(enableChange);
}


void DGisApp::setMapStyleDockLayer(QgsMapLayer *layer)
{
	if (!layer)
	{
		return;
	}

	mMapStyleWidget->setEnabled(true);
	// We don't set the layer if the dock isn't open mainly to save
	// the extra work if it's not needed
	if (mMapStylingDock->isVisible())
	{
		mMapStyleWidget->setLayer(layer);
	}
}


QgsMapLayer *DGisApp::activeLayer()
{
	return mLayerTreeView ? mLayerTreeView->currentLayer() : nullptr;
}


void DGisApp::addUserInputWidget(QWidget * widget)
{
	mUserInputDockWidget->addUserInputWidget(widget);
}



//---------------------------------------------选择工具start------------------------------------------
void DGisApp::selectFeatures()
{
	mMapCanvas->setMapTool(mMapTools.mSelectFeatures);
}

void DGisApp::selectByPolygon()
{
	mMapCanvas->setMapTool(mMapTools.mSelectPolygon);
}

void DGisApp::selectByFreehand()
{
	mMapCanvas->setMapTool(mMapTools.mSelectFreehand);
}

//-------------------------------------------选择工具end----------------------------------------------



void DGisApp::createCanvasTools()
{
	//地图平移、缩放等工具
	mMapTools.mZoomIn = new QgsMapToolZoom(mMapCanvas, false /* zoomIn */);
	mMapTools.mZoomIn->setAction(ui.mActionZoomIn);
	connect(ui.mActionZoomIn, &QAction::triggered, this, &DGisApp::zoomIn);

	mMapTools.mZoomOut = new QgsMapToolZoom(mMapCanvas, true /* zoomOut */);
	mMapTools.mZoomOut->setAction(ui.mActionZoomOut);
	connect(ui.mActionZoomOut, &QAction::triggered, this, &DGisApp::zoomOut);

	mMapTools.mPan = new QgsMapToolPan(mMapCanvas);
	//connect(static_cast<QgsMapToolPan *>(mMapTools.mPan), &QgsMapToolPan::panDistanceBearingChanged, this, &DGisApp::showPanMessage);
	mMapTools.mPan->setAction(ui.mActionPan);
	connect(ui.mActionPan, &QAction::triggered, this, &DGisApp::pan);

	connect(ui.mActionPanToSelected, &QAction::triggered, this, &DGisApp::panToSelected);
	connect(ui.mActionZoomToSelected, &QAction::triggered, this, &DGisApp::zoomToSelected);
	connect(ui.mActionZoomToLayer, &QAction::triggered, this, &DGisApp::zoomToLayerExtent);
	connect(ui.mActionZoomFullExtent, &QAction::triggered, this, &DGisApp::zoomFull);
	connect(ui.mActionZoomToLayers, &QAction::triggered, ui.mActionZoomToLayer, &QAction::triggered);
	connect(ui.mActionZoomLast, &QAction::triggered, this, &DGisApp::zoomToPrevious);
	connect(ui.mActionZoomNext, &QAction::triggered, this, &DGisApp::zoomToNext);
	//simple select
	mMapTools.mSelectFeatures = new QgsMapToolSelect(mMapCanvas);
	mMapTools.mSelectFeatures->setAction(ui.mActionSelectFeatures);
	mMapTools.mSelectFeatures->setSelectionMode(QgsMapToolSelectionHandler::SelectSimple);
	connect(ui.mActionSelectFeatures, &QAction::triggered, this, &DGisApp::selectFeatures);
	//select by polygon
	mMapTools.mSelectPolygon = new QgsMapToolSelect(mMapCanvas);
	mMapTools.mSelectPolygon->setAction(ui.mActionSelectPolygon);
	mMapTools.mSelectPolygon->setSelectionMode(QgsMapToolSelectionHandler::SelectPolygon);
	connect(ui.mActionSelectPolygon, &QAction::triggered, this, &DGisApp::selectByPolygon);
	//select by freehand
	mMapTools.mSelectFreehand = new QgsMapToolSelect(mMapCanvas);
	mMapTools.mSelectFreehand->setAction(ui.mActionSelectFreehand);
	mMapTools.mSelectFreehand->setSelectionMode(QgsMapToolSelectionHandler::SelectFreehand);
	connect(ui.mActionSelectFreehand, &QAction::triggered, this, &DGisApp::selectByFreehand);

	//tool identify
	mMapTools.mIdentify = new QgsMapToolIdentifyAction(mMapCanvas);
	mMapTools.mIdentify->setAction(ui.mActionIdentify);
	connect(mMapTools.mIdentify, &QgsMapToolIdentifyAction::copyToClipboard,
		this, &DGisApp::copyFeatures);
	connect(ui.mActionIdentify, &QAction::triggered, this, &DGisApp::identify);

	//tool measure
	mMapTools.mMeasureDist = new QgsMeasureTool(mMapCanvas, false /* area */);
	mMapTools.mMeasureDist->setAction(ui.mActionMeasure);
	mMapTools.mMeasureArea = new QgsMeasureTool(mMapCanvas, true /* area */);
	mMapTools.mMeasureArea->setAction(ui.mActionMeasureArea);
	connect(ui.mActionMeasure, &QAction::triggered, this, &DGisApp::measure);
	connect(ui.mActionMeasureArea, &QAction::triggered, this, &DGisApp::measureArea);

	mNonEditMapTool = mMapTools.mPan;
	//tool edit
	
	connect(ui.mActionToggleEditing, &QAction::toggled, [&] {toggleEditing(); });
	mMapTools.mAddFeature = new QgsMapToolAddFeature(mMapCanvas, QgsMapToolCapture::CapturePolygon);
	mMapTools.mAddFeature->setAction(ui.mActionAddFeature);
	//connect(ui.mActionAddFeature, &QAction::triggered, this, &DGisApp::addFeature);
	connect(ui.mActionAddFeature, &QAction::toggled, this, &DGisApp::enableDigitizeWithCurveAction);

	mMapTools.mSplitFeatures = new QgsMapToolSplitFeatures(mMapCanvas);
	mMapTools.mSplitFeatures->setAction(ui.mActionSplitFeatures);
	connect(ui.mActionSplitFeatures, &QAction::toggled, this, &DGisApp::enableDigitizeWithCurveAction);
	connect(ui.mActionSplitFeatures, &QAction::triggered, this, &DGisApp::splitFeatures);
	connect(ui.mActionSaveLayerEdits, &QAction::triggered, this, &DGisApp::saveActiveLayerEdits);

	mMapTools.mMoveFeature = new QgsMapToolMoveFeature(mMapCanvas, QgsMapToolMoveFeature::Move);
	mMapTools.mMoveFeature->setAction(ui.mActionMoveFeature);
	mMapTools.mMoveFeatureCopy = new QgsMapToolMoveFeature(mMapCanvas, QgsMapToolMoveFeature::CopyMove);
	mMapTools.mMoveFeatureCopy->setAction(ui.mActionMoveFeatureCopy);
	connect(ui.mActionMoveFeature, &QAction::triggered, this, &DGisApp::moveFeature);
	connect(ui.mActionMoveFeatureCopy, &QAction::triggered, this, &DGisApp::moveFeatureCopy);

	// testf
	// 创建“绘制多边形”地图工具
	mpToolAddPolygon = new DMapToolAddFeature(mMapCanvas);
	// “绘制多边形”按钮
	connect(ui.mActionAddFeature, &QAction::triggered, [&] {mMapCanvas->setMapTool(mpToolAddPolygon); });

	// 绘制临时多边形
	mMapTools.mTemporyDrawer = new DMapToolTemporyDrawer(mMapCanvas);
	connect(ui.mActionMultiEditAttributes, &QAction::triggered, [&] {mMapCanvas->setMapTool(mMapTools.mTemporyDrawer); });
	connect(mTemporyDrawerManager, &DTemporyDrawerManager::drawStyleChanged, mMapTools.mTemporyDrawer, &DMapToolTemporyDrawer::changeDrawStyle); 
	connect(mMapTools.mTemporyDrawer, &DMapToolTemporyDrawer::succeedDrawAGeometry, mTemporyDrawerManager, &DTemporyDrawerManager::insertAItemToTree);
	connect(mTemporyDrawerManager, &DTemporyDrawerManager::itemsVisibleChanged, mMapTools.mTemporyDrawer, &DMapToolTemporyDrawer::changeItemVisible);
	connect(mTemporyDrawerManager, &DTemporyDrawerManager::toDeleteAGeometry, mMapTools.mTemporyDrawer, &DMapToolTemporyDrawer::deleteGeometry);
	// succeedDrawAGeometry
	connect(mMapTools.mTemporyDrawer, &DMapToolTemporyDrawer::succeedDrawAGeometry, [=](QString &name) {
		// name != varName
		QString varName;
		QString jsonGeom = mMapTools.mTemporyDrawer->getJsonOfGeomByName(name, &varName);
		//int drawType = mMapTools.mTemporyDrawer->getCurrentDrawType();

		bool res = this->mPythonRunner->runString(jsonGeom, QString("Fail to Transform samples to pyObject"), true);
		if (!res)
		{
			QString errRes = this->mPythonRunner->getLastRes();
			QMessageBox::critical(this, "Transfrom samples", errRes, QMessageBox::Ok);
		}
		else
			mGeeObjectManager->addOneRecord(varName, DGeeObjectManager::GeeObjType::Geometry, 1);
	});

	// 分类
	mMapTools.mDrawSample = new DMapToolDrawSample(mMapCanvas);
	connect(ui.mActionVertexTool, &QAction::triggered, [&] {mMapCanvas->setMapTool(mMapTools.mDrawSample); });
	//addASampleItemToTree
	connect(mMapTools.mDrawSample, &DMapToolDrawSample::succeedDrawAGeometry, mClassifyManager, &DClassifyManager::addASampleItemToTree);
	connect(mClassifyManager, &DClassifyManager::CreateNewClass, mMapTools.mDrawSample, &DMapToolDrawSample::addANewClass);
	// to visible
	connect(mClassifyManager, &DClassifyManager::currentDrawClassChanged, mMapTools.mDrawSample, &DMapToolDrawSample::changeCurrentDrawClass);
	connect(mClassifyManager, &DClassifyManager::itemVisibleChanged, mMapTools.mDrawSample, &DMapToolDrawSample::changeItemVisible);
	connect(mClassifyManager, &DClassifyManager::toChangeAllItemVisible, mMapTools.mDrawSample, &DMapToolDrawSample::changeAllItemVisible);
	// to delete
	connect(mClassifyManager, &DClassifyManager::toRemoveAGeom, mMapTools.mDrawSample, &DMapToolDrawSample::removeAGeom);
	connect(mClassifyManager, &DClassifyManager::toRemoveAClass, mMapTools.mDrawSample, &DMapToolDrawSample::removeAClass);
	// change draw style
	connect(mClassifyManager, &DClassifyManager::toChangeDrawStyle, mMapTools.mDrawSample, &DMapToolDrawSample::changeDrawStyle);
	connect(mClassifyManager, &DClassifyManager::toExportSamples, mMapTools.mDrawSample, &DMapToolDrawSample::exportSamples);
	// to shp
	connect(mMapTools.mDrawSample, &DMapToolDrawSample::toSaveAsFile, [=](QgsMapLayer * layer) {
		saveAsFile(layer);
	});
	// to run a command
	//connect(mMapTools.mDrawSample, &DMapToolDrawSample::toRunACommand, [=](QString &command) {
	//	bool res = this->mPythonRunner->runString(command, QString("Fail to Transform samples to pyObject"),true);
	//	if (!res)
	//	{
	//		QString errRes = this->mPythonRunner->getLastRes();
	//		QMessageBox::critical(this, "Transfrom samples", errRes, QMessageBox::Ok);
	//	}
	//});
}


void DGisApp::copyFeatures(QgsFeatureStore &featureStore)
{
	clipboard()->replaceWithCopyOf(featureStore);
}


QgsVectorLayerTools *DGisApp::vectorLayerTools() { return mVectorLayerTools; }


QgsMapCanvas *DGisApp::mapCanvas()
{
	Q_ASSERT(mMapCanvas);
	return mMapCanvas;
}


QgsMessageBar *DGisApp::messageBar()
{
	// Q_ASSERT( mInfoBar );
	return mInfoBar;
}


QgsClipboard *DGisApp::clipboard()
{
	return mInternalClipboard;
}


QgsAttributeEditorContext DGisApp::createAttributeEditorContext()
{
	QgsAttributeEditorContext context;
	context.setVectorLayerTools(vectorLayerTools());
	context.setMapCanvas(mapCanvas());
	context.setCadDockWidget(cadDockWidget());
	context.setMainMessageBar(messageBar());
	return context;
}


void DGisApp::removeLayer()
{
	if (!mLayerTreeView)
	{
		return;
	}

	// look for layers recursively so we catch also those that are within selected groups
	const QList<QgsMapLayer *> selectedLayers = mLayerTreeView->selectedLayersRecursive();

	QStringList nonRemovableLayerNames;
	for (QgsMapLayer *layer : selectedLayers)
	{
		if (!layer->flags().testFlag(QgsMapLayer::Removable))
			nonRemovableLayerNames << layer->name();
	}
	if (!nonRemovableLayerNames.isEmpty())
	{
		QMessageBox::warning(this, tr("Required Layers"),
			tr("The following layers are marked as required by the project:\n\n%1\n\nPlease deselect them (or unmark as required) and retry.").arg(nonRemovableLayerNames.join(QLatin1Char('\n'))));
		return;
	}

	for (QgsMapLayer *layer : selectedLayers)
	{
		QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(layer);
		if (vlayer && vlayer->isEditable() && !toggleEditing(vlayer, true))
			return;
	}

	QStringList activeTaskDescriptions;
	for (QgsMapLayer *layer : selectedLayers)
	{
		QList<QgsTask *> tasks = QgsApplication::taskManager()->tasksDependentOnLayer(layer);
		if (!tasks.isEmpty())
		{
			const auto constTasks = tasks;
			for (QgsTask *task : constTasks)
			{
				activeTaskDescriptions << tr("*%1").arg(task->description());
			}
		}
	}

	if (!activeTaskDescriptions.isEmpty())
	{
		QMessageBox::warning(this, tr("Active Tasks"),
			tr("The following tasks are currently running which depend on this layer:\n\n%1\n\nPlease cancel these tasks and retry.").arg(activeTaskDescriptions.join(QLatin1Char('\n'))));
		return;
	}

	QList<QgsLayerTreeNode *> selectedNodes = mLayerTreeView->selectedNodes(true);

	//validate selection
	if (selectedNodes.isEmpty())
	{
		messageBar()->pushMessage(tr("No legend entries selected"),
			tr("Select the layers and groups you want to remove in the legend."),
			Qgis::Info, 4);//messageTimeout()
		return;
	}

	bool promptConfirmation = QgsSettings().value(QStringLiteral("qgis/askToDeleteLayers"), true).toBool();

	// Don't show prompt to remove a empty group.
	if (selectedNodes.count() == 1 && selectedNodes.at(0)->nodeType() == QgsLayerTreeNode::NodeGroup && selectedNodes.at(0)->children().count() == 0)
	{
		promptConfirmation = false;
	}

	bool shiftHeld = QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier);
	//display a warning
	if (!shiftHeld && promptConfirmation && QMessageBox::warning(this, tr("Remove layers and groups"), tr("Remove %n legend entries?", "number of legend items to remove", selectedNodes.count()), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
	{
		return;
	}

	const auto constSelectedNodes = selectedNodes;
	for (QgsLayerTreeNode *node : constSelectedNodes)
	{
		QgsLayerTreeGroup *parentGroup = qobject_cast<QgsLayerTreeGroup *>(node->parent());
		if (parentGroup)
			parentGroup->removeChildNode(node);
	}

	//showStatusMessage(tr("%n legend entries removed.", "number of removed legend entries", selectedNodes.count()));

	refreshMapCanvas();
}


bool DGisApp::toggleEditing(QgsMapLayer *layer, bool allowCancel)
{
	QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(layer);
	if (!vlayer)
	{
		return false;
	}

	bool res = true;

	QString connString = QgsTransaction::connectionString(vlayer->source());
	QString key = vlayer->providerType();

	QMap<QPair<QString, QString>, QgsTransactionGroup *> transactionGroups = QgsProject::instance()->transactionGroups();
	QMap<QPair<QString, QString>, QgsTransactionGroup *>::iterator tIt = transactionGroups.find(qMakePair(key, connString));
	QgsTransactionGroup *tg = (tIt != transactionGroups.end() ? tIt.value() : nullptr);

	bool isModified = false;

	// Assume changes if: a) the layer reports modifications or b) its transaction group was modified
	QString modifiedLayerNames;
	bool hasSeveralModifiedLayers = false;
	if (tg && tg->layers().contains(vlayer) && tg->modified())
	{
		isModified = true;
		std::vector<QString> vectModifiedLayerNames;
		if (vlayer->isModified())
		{
			vectModifiedLayerNames.push_back(vlayer->name());
		}
		for (QgsVectorLayer *iterLayer : tg->layers())
		{
			if (iterLayer != vlayer && iterLayer->isModified())
			{
				vectModifiedLayerNames.push_back(iterLayer->name());
			}
		}
		if (vectModifiedLayerNames.size() == 1)
		{
			modifiedLayerNames = vectModifiedLayerNames[0];
		}
		else if (vectModifiedLayerNames.size() == 2)
		{
			modifiedLayerNames = tr("%1 and %2").arg(vectModifiedLayerNames[0]).arg(vectModifiedLayerNames[1]);
		}
		else if (vectModifiedLayerNames.size() > 2)
		{
			modifiedLayerNames = tr("%1, %2, …").arg(vectModifiedLayerNames[0]).arg(vectModifiedLayerNames[1]);
		}
		hasSeveralModifiedLayers = vectModifiedLayerNames.size() > 1;
	}
	else if (vlayer->isModified())
	{
		isModified = true;
		modifiedLayerNames = vlayer->name();
	}

	if (!vlayer->isEditable() && !vlayer->readOnly())
	{
		if (!(vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::EditingCapabilities))
		{
			ui.mActionToggleEditing->setChecked(false);
			ui.mActionToggleEditing->setEnabled(false);
			messageBar()->pushMessage(tr("Start editing failed"),
				tr("Provider cannot be opened for editing"),
				Qgis::Info, 4);//messageTimeout()
			return false;
		}

		vlayer->startEditing();

		QgsSettings settings;
		QString markerType = settings.value(QStringLiteral("qgis/digitizing/marker_style"), "Cross").toString();
		bool markSelectedOnly = settings.value(QStringLiteral("qgis/digitizing/marker_only_for_selected"), true).toBool();

		// redraw only if markers will be drawn
		if ((!markSelectedOnly || vlayer->selectedFeatureCount() > 0) &&
			(markerType == QLatin1String("Cross") || markerType == QLatin1String("SemiTransparentCircle")))
		{
			vlayer->triggerRepaint();
		}
	}
	else if (isModified)
	{
		QMessageBox::StandardButtons buttons = QMessageBox::Save | QMessageBox::Discard;
		if (allowCancel)
			buttons |= QMessageBox::Cancel;

		switch (QMessageBox::question(nullptr,
			tr("Stop Editing"),
			hasSeveralModifiedLayers ? tr("Do you want to save the changes to layers %1?").arg(modifiedLayerNames) : tr("Do you want to save the changes to layer %1?").arg(modifiedLayerNames),
			buttons))
		{
		case QMessageBox::Cancel:
			res = false;
			break;

		case QMessageBox::Save:
			QApplication::setOverrideCursor(Qt::WaitCursor);
			vlayer->endEditCommand();
			if (!vlayer->commitChanges())
			{
				commitError(vlayer);
				// Leave the in-memory editing state alone,
				// to give the user a chance to enter different values
				// and try the commit again later
				res = false;
			}

			vlayer->triggerRepaint();

			QApplication::restoreOverrideCursor();
			break;

		case QMessageBox::Discard:
		{
			QApplication::setOverrideCursor(Qt::WaitCursor);

			QgsCanvasRefreshBlocker refreshBlocker;
			if (!vlayer->rollBack())
			{
				messageBar()->pushMessage(tr("Error"),
					tr("Problems during roll back"),
					Qgis::Critical);
				res = false;
			}

			vlayer->triggerRepaint();

			QApplication::restoreOverrideCursor();
			break;
		}

		default:
			break;
		}
	}
	else //layer not modified
	{
		QgsCanvasRefreshBlocker refreshBlocker;
		vlayer->rollBack();
		res = true;
		vlayer->triggerRepaint();
	}

	if (!res && layer == activeLayer())
	{
		// while also called when layer sends editingStarted/editingStopped signals,
		// this ensures correct restoring of gui state if toggling was canceled
		// or layer commit/rollback functions failed
		activateDeactivateLayerRelatedActions(layer);
	}

	return res;
}



bool DGisApp::askUserForDatumTransform(const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsMapLayer *layer)
{
	Q_ASSERT(qApp->thread() == QThread::currentThread());

	QString title;
	if (layer)
	{
		// try to make a user-friendly (short!) identifier for the layer
		QString layerIdentifier;
		if (!layer->name().isEmpty())
		{
			layerIdentifier = layer->name();
		}
		else
		{
			const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri(layer->providerType(), layer->source());
			if (parts.contains(QStringLiteral("path")))
			{
				const QFileInfo fi(parts.value(QStringLiteral("path")).toString());
				layerIdentifier = fi.fileName();
			}
			else if (layer->dataProvider())
			{
				const QgsDataSourceUri uri(layer->source());
				layerIdentifier = uri.table();
			}
		}
		if (!layerIdentifier.isEmpty())
			title = tr("Select Transformation for %1").arg(layerIdentifier);
	}

	return QgsDatumTransformDialog::run(sourceCrs, destinationCrs, this, mMapCanvas, title);
}

void DGisApp::saveEdits(QgsMapLayer *layer, bool leaveEditable, bool triggerRepaint)
{
	QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(layer);
	if (!vlayer || !vlayer->isEditable() || !vlayer->isModified())
		return;

	//if (vlayer == activeLayer())
	//	mSaveRollbackInProgress = true;

	//if (!vlayer->commitChanges(!leaveEditable))
	//{
	//	mSaveRollbackInProgress = false;
	//	commitError(vlayer);
	//}

	if (triggerRepaint)
	{
		vlayer->triggerRepaint();
	}
}

void DGisApp::saveEdits()
{
	const auto constSelectedLayers = mLayerTreeView->selectedLayers();
	for (QgsMapLayer *layer : constSelectedLayers)
	{
		saveEdits(layer, true, false);
	}
	refreshMapCanvas();
	activateDeactivateLayerRelatedActions(activeLayer());
}

void DGisApp::addTabifiedDockWidget(Qt::DockWidgetArea area, QDockWidget *dockWidget, const QStringList &tabifyWith, bool raiseTab)
{
	QList<QDockWidget *> dockWidgetsInArea;
	const auto dockWidgets = findChildren<QDockWidget *>();
	for (QDockWidget *w : dockWidgets)
	{
		if (w->isVisible() && dockWidgetArea(w) == area)
		{
			dockWidgetsInArea << w;
		}
	}

	addDockWidget(area, dockWidget); // First add the dock widget, then attempt to tabify
	if (dockWidgetsInArea.length() > 0)
	{
		// Get the base dock widget that we'll use to tabify our new dockWidget
		QDockWidget *tabifyWithDockWidget = nullptr;
		if (!tabifyWith.isEmpty())
		{
			// Iterate the list of object names looking for a
			// dock widget to tabify the new one on top of it
			bool objectNameFound = false;
			for (int i = 0; i < tabifyWith.size(); i++)
			{
				for (QDockWidget *cw : dockWidgetsInArea)
				{
					if (cw->objectName() == tabifyWith.at(i))
					{
						tabifyWithDockWidget = cw;
						objectNameFound = true; // Also exit the outer for loop
						break;
					}
				}
				if (objectNameFound)
				{
					break;
				}
			}
		}
		if (!tabifyWithDockWidget)
		{
			tabifyWithDockWidget = dockWidgetsInArea.at(0); // Last resort
		}

		QTabBar *existentTabBar = nullptr;
		int currentIndex = -1;
		if (!raiseTab && dockWidgetsInArea.length() > 1)
		{
			// Chances are we've already got a tabBar, if so, get
			// currentIndex to restore status after inserting our new tab
			const QList<QTabBar *> tabBars = findChildren<QTabBar *>(QString(), Qt::FindDirectChildrenOnly);
			bool tabBarFound = false;
			for (QTabBar *tabBar : tabBars)
			{
				for (int i = 0; i < tabBar->count(); i++)
				{
					if (tabBar->tabText(i) == tabifyWithDockWidget->windowTitle())
					{
						existentTabBar = tabBar;
						currentIndex = tabBar->currentIndex();
						tabBarFound = true;
						break;
					}
				}
				if (tabBarFound)
				{
					break;
				}
			}
		}

		// Now we can put the new dockWidget on top of tabifyWith
		tabifyDockWidget(tabifyWithDockWidget, dockWidget);

		// Should we restore dock widgets status?
		if (!raiseTab)
		{
			if (existentTabBar)
			{
				existentTabBar->setCurrentIndex(currentIndex);
			}
			else
			{
				tabifyWithDockWidget->raise(); // Single base dock widget, we can just raise it
			}
		}
	}
}

void DGisApp::deleteSelected(QgsMapLayer *layer, QWidget *parent, bool checkFeaturesVisible)
{
	if (!layer)
	{
		layer = mLayerTreeView->currentLayer();
	}

	if (!parent)
	{
		parent = this;
	}

	if (!layer)
	{
		messageBar()->pushMessage(tr("No Layer Selected"),
			tr("To delete features, you must select a vector layer in the legend"),
			Qgis::Info, messageTimeout());
		return;
	}

	QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(layer);
	if (!vlayer)
	{
		messageBar()->pushMessage(tr("No Vector Layer Selected"),
			tr("Deleting features only works on vector layers"),
			Qgis::Info, messageTimeout());
		return;
	}

	if (!(vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteFeatures))
	{
		messageBar()->pushMessage(tr("Provider does not support deletion"),
			tr("Data provider does not support deleting features"),
			Qgis::Info, messageTimeout());
		return;
	}

	if (!vlayer->isEditable())
	{
		messageBar()->pushMessage(tr("Layer not editable"),
			tr("The current layer is not editable. Choose 'Start editing' in the digitizing toolbar."),
			Qgis::Info, messageTimeout());
		return;
	}

	//validate selection
	const int numberOfSelectedFeatures = vlayer->selectedFeatureCount();
	if (numberOfSelectedFeatures == 0)
	{
		messageBar()->pushMessage(tr("No Features Selected"),
			tr("The current layer has no selected features"),
			Qgis::Info, messageTimeout());
		return;
	}
	//display a warning
	if (checkFeaturesVisible)
	{
		QgsFeature feat;
		QgsFeatureIterator it = vlayer->getSelectedFeatures(QgsFeatureRequest().setNoAttributes());
		bool allFeaturesInView = true;
		QgsRectangle viewRect = mMapCanvas->mapSettings().mapToLayerCoordinates(vlayer, mMapCanvas->extent());

		while (it.nextFeature(feat))
		{
			if (allFeaturesInView && !viewRect.intersects(feat.geometry().boundingBox()))
			{
				allFeaturesInView = false;
				break;
			}
		}

		if (!allFeaturesInView)
		{
			// for extra safety to make sure we are not removing geometries by accident
			int res = QMessageBox::warning(mMapCanvas, tr("Delete %n feature(s) from layer \"%1\"", nullptr, numberOfSelectedFeatures).arg(vlayer->name()),
				tr("Some of the selected features are outside of the current map view. Would you still like to continue?"),
				QMessageBox::Yes | QMessageBox::No);
			if (res != QMessageBox::Yes)
				return;
		}
	}

	QgsVectorLayerUtils::QgsDuplicateFeatureContext infoContext;
	if (QgsVectorLayerUtils::impactsCascadeFeatures(vlayer, vlayer->selectedFeatureIds(), QgsProject::instance(), infoContext, QgsVectorLayerUtils::IgnoreAuxiliaryLayers))
	{
		QString childrenInfo;
		int childrenCount = 0;
		const auto infoContextLayers = infoContext.layers();
		for (QgsVectorLayer *chl : infoContextLayers)
		{
			childrenCount += infoContext.duplicatedFeatures(chl).size();
			childrenInfo += (tr("%1 feature(s) on layer \"%2\", ").arg(infoContext.duplicatedFeatures(chl).size()).arg(chl->name()));
		}

		// for extra safety to make sure we know that the delete can have impact on children and joins
		int res = QMessageBox::question(mMapCanvas, tr("Delete at least %1 feature(s) on other layer(s)").arg(childrenCount),
			tr("Delete %1 feature(s) on layer \"%2\", %3 as well\nand all of its other descendants.\nDelete these features?").arg(numberOfSelectedFeatures).arg(vlayer->name()).arg(childrenInfo),
			QMessageBox::Yes | QMessageBox::No);
		if (res != QMessageBox::Yes)
			return;
	}

	vlayer->beginEditCommand(tr("Features deleted"));
	int deletedCount = 0;
	QgsVectorLayer::DeleteContext context(true, QgsProject::instance());
	if (!vlayer->deleteSelectedFeatures(&deletedCount, &context))
	{
		messageBar()->pushMessage(tr("Problem deleting features"),
			tr("A problem occurred during deletion from layer \"%1\". %n feature(s) not deleted.", nullptr, numberOfSelectedFeatures - deletedCount).arg(vlayer->name()),
			Qgis::Warning);
	}
	else
	{
		const QList<QgsVectorLayer *> contextLayers = context.handledLayers(false);
		// if it affects more than one non-auxiliary layer, print feedback for all descendants
		if (contextLayers.size() > 1)
		{
			deletedCount = 0;
			QString feedbackMessage;
			for (QgsVectorLayer *contextLayer : contextLayers)
			{
				feedbackMessage += tr("%1 on layer %2. ").arg(context.handledFeatures(contextLayer).size()).arg(contextLayer->name());
				deletedCount += context.handledFeatures(contextLayer).size();
			}
			messageBar()->pushMessage(tr("%1 features deleted: %2").arg(deletedCount).arg(feedbackMessage), Qgis::Success);
		}

		//showStatusMessage(tr("%n feature(s) deleted.", "number of features deleted", deletedCount));
	}

	vlayer->endEditCommand();
}

//deleable dxl
int DGisApp::messageTimeout()
{
	QgsSettings settings;
	return settings.value(QStringLiteral("qgis/messageTimeout"), 5).toInt();
}

QgsAdvancedDigitizingDockWidget *DGisApp::cadDockWidget() { return mAdvancedDigitizingDockWidget; }



//-----------------------------------------地图平移、缩放等工具 start----------------------------------------------------

void DGisApp::zoomIn()
{
	QgsDebugMsgLevel(QStringLiteral("Setting map tool to zoomIn"), 2);

	mMapCanvas->setMapTool(mMapTools.mZoomIn);
}

void DGisApp::zoomOut()
{
	mMapCanvas->setMapTool(mMapTools.mZoomOut);
}

void DGisApp::pan()
{
	mMapCanvas->setMapTool(mMapTools.mPan);
}

void DGisApp::zoomToSelected()
{
	mMapCanvas->zoomToSelected();
}

void DGisApp::zoomToLayerExtent()
{
	mLayerTreeView->defaultActions()->zoomToLayer(mMapCanvas);
}

void DGisApp::panToSelected()
{
	mMapCanvas->panToSelected();
}

void DGisApp::zoomFull()
{
	mMapCanvas->zoomToFullExtent();
}

void DGisApp::zoomToPrevious()
{
	mMapCanvas->zoomToPreviousExtent();
}

void DGisApp::zoomToNext()
{
	mMapCanvas->zoomToNextExtent();
}

void DGisApp::identify()
{
	mMapCanvas->setMapTool(mMapTools.mIdentify);
}

void DGisApp::measure()
{
	mMapCanvas->setMapTool(mMapTools.mMeasureDist);
}

void DGisApp::measureArea()
{
	mMapCanvas->setMapTool(mMapTools.mMeasureArea);
}
//-----------------------------------------地图平移、缩放等工具 end----------------------------------------------------



void DGisApp::cutSelectionToClipboard(QgsMapLayer *layerContainingSelection)
{
	// Test for feature support in this layer
	QgsVectorLayer *selectionVectorLayer = qobject_cast<QgsVectorLayer *>(layerContainingSelection ? layerContainingSelection : activeLayer());
	if (!selectionVectorLayer)
		return;

	if (!selectionVectorLayer->isEditable())
	{
		messageBar()->pushMessage(tr("Layer not editable"),
			tr("The current layer is not editable. Choose 'Start editing' in the digitizing toolbar."),
			Qgis::Info, messageTimeout());
		return;
	}

	clipboard()->replaceWithCopyOf(selectionVectorLayer);

	selectionVectorLayer->beginEditCommand(tr("Features cut"));
	selectionVectorLayer->deleteSelectedFeatures();
	selectionVectorLayer->endEditCommand();
}


void DGisApp::pasteFromClipboard(QgsMapLayer *destinationLayer)
{
	QgsVectorLayer *pasteVectorLayer = qobject_cast<QgsVectorLayer *>(destinationLayer ? destinationLayer : activeLayer());
	if (!pasteVectorLayer)
		return;

	if (!pasteVectorLayer->isEditable())
	{
		messageBar()->pushMessage(tr("Layer not editable"),
			tr("The current layer is not editable. Choose 'Start editing' in the digitizing toolbar."),
			Qgis::Info, messageTimeout());
		return;
	}

	pasteVectorLayer->beginEditCommand(tr("Features pasted"));
	QgsFeatureList features = clipboard()->transformedCopyOf(pasteVectorLayer->crs(), pasteVectorLayer->fields());
	int nTotalFeatures = features.count();
	QgsExpressionContext context = pasteVectorLayer->createExpressionContext();

	QgsFeatureList compatibleFeatures(QgsVectorLayerUtils::makeFeaturesCompatible(features, pasteVectorLayer));
	QgsVectorLayerUtils::QgsFeaturesDataList newFeaturesDataList;
	newFeaturesDataList.reserve(compatibleFeatures.size());

	// Count collapsed geometries
	int invalidGeometriesCount = 0;

	for (const auto &feature : qgis::as_const(compatibleFeatures))
	{

		QgsGeometry geom = feature.geometry();

		if (!(geom.isEmpty() || geom.isNull()))
		{
			// avoid intersection if enabled in digitize settings
			QList<QgsVectorLayer *> avoidIntersectionsLayers;
			switch (QgsProject::instance()->avoidIntersectionsMode())
			{
			case QgsProject::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer:
				avoidIntersectionsLayers.append(pasteVectorLayer);
				break;
			case QgsProject::AvoidIntersectionsMode::AvoidIntersectionsLayers:
				avoidIntersectionsLayers = QgsProject::instance()->avoidIntersectionsLayers();
				break;
			case QgsProject::AvoidIntersectionsMode::AllowIntersections:
				break;
			}
			if (avoidIntersectionsLayers.size() > 0)
			{
				geom.avoidIntersections(avoidIntersectionsLayers);
			}

			// count collapsed geometries
			if (geom.isEmpty() || geom.isNull())
				invalidGeometriesCount++;
		}

		QgsAttributeMap attrMap;
		for (int i = 0; i < feature.attributes().count(); i++)
		{
			attrMap[i] = feature.attribute(i);
		}
		newFeaturesDataList << QgsVectorLayerUtils::QgsFeatureData(geom, attrMap);
	}

	// now create new feature using pasted feature as a template. This automatically handles default
	// values and field constraints
	QgsFeatureList newFeatures{ QgsVectorLayerUtils::createFeatures(pasteVectorLayer, newFeaturesDataList, &context) };

	// check constraints
	bool hasStrongConstraints = false;

	for (const QgsField &field : pasteVectorLayer->fields())
	{
		if ((field.constraints().constraints() & QgsFieldConstraints::ConstraintUnique && field.constraints().constraintStrength(QgsFieldConstraints::ConstraintUnique) & QgsFieldConstraints::ConstraintStrengthHard) || (field.constraints().constraints() & QgsFieldConstraints::ConstraintNotNull && field.constraints().constraintStrength(QgsFieldConstraints::ConstraintNotNull) & QgsFieldConstraints::ConstraintStrengthHard) || (field.constraints().constraints() & QgsFieldConstraints::ConstraintExpression && !field.constraints().constraintExpression().isEmpty() && field.constraints().constraintStrength(QgsFieldConstraints::ConstraintExpression) & QgsFieldConstraints::ConstraintStrengthHard))
		{
			hasStrongConstraints = true;
			break;
		}
	}

	if (hasStrongConstraints)
	{
		QgsFeatureList validFeatures = newFeatures;
		QgsFeatureList invalidFeatures;
		QMutableListIterator<QgsFeature> it(validFeatures);
		while (it.hasNext())
		{
			QgsFeature &f = it.next();
			for (int idx = 0; idx < pasteVectorLayer->fields().count(); ++idx)
			{
				QStringList errors;
				if (!QgsVectorLayerUtils::validateAttribute(pasteVectorLayer, f, idx, errors, QgsFieldConstraints::ConstraintStrengthHard, QgsFieldConstraints::ConstraintOriginNotSet))
				{
					invalidFeatures << f;
					it.remove();
					break;
				}
			}
		}

		if (!invalidFeatures.isEmpty())
		{
			newFeatures.clear();

			QgsAttributeEditorContext context(createAttributeEditorContext());
			context.setAllowCustomUi(false);
			context.setFormMode(QgsAttributeEditorContext::StandaloneDialog);

			QgsFixAttributeDialog *dialog = new QgsFixAttributeDialog(pasteVectorLayer, invalidFeatures, this, context);

			connect(dialog, &QgsFixAttributeDialog::finished, this, [=](int feedback) {
				QgsFeatureList features = newFeatures;
				switch (feedback)
				{
				case QgsFixAttributeDialog::PasteValid:
					//paste valid and fixed, vanish unfixed
					features << validFeatures << dialog->fixedFeatures();
					break;
				case QgsFixAttributeDialog::PasteAll:
					//paste all, even unfixed
					features << validFeatures << dialog->fixedFeatures() << dialog->unfixedFeatures();
					break;
				}
				pasteFeatures(pasteVectorLayer, invalidGeometriesCount, nTotalFeatures, features);
				dialog->deleteLater();
			});
			dialog->show();
			return;
		}
	}

	pasteFeatures(pasteVectorLayer, invalidGeometriesCount, nTotalFeatures, newFeatures);
}


void DGisApp::pasteFeatures(QgsVectorLayer *pasteVectorLayer, int invalidGeometriesCount, int nTotalFeatures, QgsFeatureList &features)
{
	int nCopiedFeatures = features.count();
	if (pasteVectorLayer->addFeatures(features))
	{
		QgsFeatureIds newIds;
		newIds.reserve(features.size());
		for (const QgsFeature &f : qgis::as_const(features))
		{
			newIds << f.id();
		}

		pasteVectorLayer->selectByIds(newIds);
	}
	else
	{
		nCopiedFeatures = 0;
	}
	pasteVectorLayer->endEditCommand();
	pasteVectorLayer->updateExtents();

	Qgis::MessageLevel level = (nCopiedFeatures == 0 || invalidGeometriesCount > 0) ? Qgis::Warning : Qgis::Info;
	QString message;
	if (nCopiedFeatures == 0)
	{
		message = tr("No features pasted.");
	}
	else if (nCopiedFeatures == nTotalFeatures)
	{
		message = tr("%1 features were pasted.").arg(nCopiedFeatures);
	}
	else
	{
		message = tr("%1 of %2 features could be pasted.").arg(nCopiedFeatures).arg(nTotalFeatures);
	}

	// warn the user if the pasted features have invalid geometries
	if (invalidGeometriesCount > 0)
		message += invalidGeometriesCount == 1 ? tr(" Geometry collapsed due to intersection avoidance.") : tr("%1 geometries collapsed due to intersection avoidance.").arg(invalidGeometriesCount);

	messageBar()->pushMessage(tr("Paste features"),
		message,
		level, messageTimeout());

	pasteVectorLayer->triggerRepaint();
}


QgsVectorLayer *DGisApp::pasteAsNewMemoryVector(const QString &layerName)
{
	QString layerNameCopy = layerName;

	if (layerNameCopy.isEmpty())
	{
		bool ok;
		QString defaultName = tr("Pasted");
		layerNameCopy = QInputDialog::getText(this, tr("Paste as Scratch Layer"),
			tr("Layer name"), QLineEdit::Normal,
			defaultName, &ok);
		if (!ok)
			return nullptr;

		if (layerNameCopy.isEmpty())
		{
			layerNameCopy = defaultName;
		}
	}

	std::unique_ptr<QgsVectorLayer> layer = pasteToNewMemoryVector();
	if (!layer)
		return nullptr;

	layer->setName(layerNameCopy);

	QgsCanvasRefreshBlocker refreshBlocker;

	QgsVectorLayer *result = layer.get();
	QgsProject::instance()->addMapLayer(layer.release());

	return result;
}


void DGisApp::deselectAll()
{
	// Turn off rendering to improve speed.
	QgsCanvasRefreshBlocker refreshBlocker;

	QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
	for (QMap<QString, QgsMapLayer *>::iterator it = layers.begin(); it != layers.end(); ++it)
	{
		QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>(it.value());
		if (!vl)
			continue;

		vl->removeSelection();
	}
}


void DGisApp::deselectActiveLayer()
{
	QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(mMapCanvas->currentLayer());

	if (!vlayer)
	{
		messageBar()->pushMessage(
			tr("No active vector layer"),
			tr("To deselect all features, choose a vector layer in the legend"),
			Qgis::Info,
			messageTimeout());
		return;
	}

	vlayer->removeSelection();
}


void DGisApp::selectAll()
{
	QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(mMapCanvas->currentLayer());
	if (!vlayer)
	{
		messageBar()->pushMessage(
			tr("No active vector layer"),
			tr("To select all, choose a vector layer in the legend."),
			Qgis::Info,
			messageTimeout());
		return;
	}

	vlayer->selectAll();
}


void DGisApp::invertSelection()
{
	QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(mMapCanvas->currentLayer());
	if (!vlayer)
	{
		messageBar()->pushMessage(
			tr("No active vector layer"),
			tr("To invert selection, choose a vector layer in the legend"),
			Qgis::Info,
			messageTimeout());
		return;
	}

	vlayer->invertSelection();
}


void DGisApp::pasteAsNewVector()
{

	std::unique_ptr<QgsVectorLayer> layer = pasteToNewMemoryVector();
	if (!layer)
		return;

	saveAsVectorFileGeneral(layer.get(), false);
}


std::unique_ptr<QgsVectorLayer> DGisApp::pasteToNewMemoryVector()
{
	const QgsFields fields = clipboard()->fields();

	// Decide geometry type from features, switch to multi type if at least one multi is found
	QMap<QgsWkbTypes::Type, int> typeCounts;
	const QgsFeatureList features = clipboard()->copyOf(fields);
	for (const QgsFeature &feature : features)
	{
		if (!feature.hasGeometry())
			continue;

		const QgsWkbTypes::Type type = feature.geometry().wkbType();

		if (type == QgsWkbTypes::Unknown || type == QgsWkbTypes::NoGeometry)
			continue;

		if (QgsWkbTypes::isSingleType(type))
		{
			if (typeCounts.contains(QgsWkbTypes::multiType(type)))
			{
				typeCounts[QgsWkbTypes::multiType(type)] = typeCounts[QgsWkbTypes::multiType(type)] + 1;
			}
			else
			{
				typeCounts[type] = typeCounts[type] + 1;
			}
		}
		else if (QgsWkbTypes::isMultiType(type))
		{
			if (typeCounts.contains(QgsWkbTypes::singleType(type)))
			{
				// switch to multi type
				typeCounts[type] = typeCounts[QgsWkbTypes::singleType(type)];
				typeCounts.remove(QgsWkbTypes::singleType(type));
			}
			typeCounts[type] = typeCounts[type] + 1;
		}
	}

	const QgsWkbTypes::Type wkbType = !typeCounts.isEmpty() ? typeCounts.keys().value(0) : QgsWkbTypes::NoGeometry;

	if (features.isEmpty())
	{
		// should not happen
		messageBar()->pushMessage(tr("Paste features"),
			tr("No features in clipboard."),
			Qgis::Warning, messageTimeout());
		return nullptr;
	}
	else if (typeCounts.size() > 1)
	{
		QString typeName = wkbType != QgsWkbTypes::NoGeometry ? QgsWkbTypes::displayString(wkbType) : QStringLiteral("none");
		messageBar()->pushMessage(tr("Paste features"),
			tr("Multiple geometry types found, features with geometry different from %1 will be created without geometry.").arg(typeName),
			Qgis::Info, messageTimeout());
	}

	std::unique_ptr<QgsVectorLayer> layer(QgsMemoryProviderUtils::createMemoryLayer(QStringLiteral("pasted_features"), QgsFields(), wkbType, clipboard()->crs()));

	if (!layer->isValid() || !layer->dataProvider())
	{
		messageBar()->pushMessage(tr("Paste features"),
			tr("Cannot create new layer."),
			Qgis::Warning, messageTimeout());
		return nullptr;
	}

	layer->startEditing();
	for (const QgsField &f : clipboard()->fields())
	{
		QgsDebugMsgLevel(QStringLiteral("field %1 (%2)").arg(f.name(), QVariant::typeToName(f.type())), 2);
		if (!layer->addAttribute(f))
		{
			messageBar()->pushMessage(tr("Paste features"),
				tr("Cannot create field %1 (%2,%3)").arg(f.name(), f.typeName(), QVariant::typeToName(f.type())),
				Qgis::Warning, messageTimeout());
			return nullptr;
		}
	}

	// Convert to multi if necessary
	QgsFeatureList convertedFeatures;
	convertedFeatures.reserve(features.length());
	for (QgsFeature feature : features)
	{
		if (!feature.hasGeometry())
		{
			convertedFeatures.append(feature);
			continue;
		}

		const QgsWkbTypes::Type type = feature.geometry().wkbType();
		if (type == QgsWkbTypes::Unknown || type == QgsWkbTypes::NoGeometry)
		{
			convertedFeatures.append(feature);
			continue;
		}

		if (QgsWkbTypes::singleType(wkbType) != QgsWkbTypes::singleType(type))
		{
			feature.clearGeometry();
		}

		if (QgsWkbTypes::isMultiType(wkbType) && QgsWkbTypes::isSingleType(type))
		{
			QgsGeometry g = feature.geometry();
			g.convertToMultiType();
			feature.setGeometry(g);
		}
		convertedFeatures.append(feature);
	}
	if (!layer->addFeatures(convertedFeatures) || !layer->commitChanges())
	{
		QgsDebugMsg(QStringLiteral("Cannot add features or commit changes"));
		return nullptr;
	}

	QgsDebugMsgLevel(QStringLiteral("%1 features pasted to temporary scratch layer").arg(layer->featureCount()), 2);
	return layer;
}


QString DGisApp::saveAsVectorFileGeneral(QgsVectorLayer *vlayer, bool symbologyOption, bool onlySelected, bool defaultToAddToMap)
{
	if (!vlayer)
	{
		vlayer = qobject_cast<QgsVectorLayer *>(activeLayer()); // FIXME: output of multiple layers at once?
	}

	if (!vlayer)
		return QString();

	const QString layerId = vlayer->id();

	auto onSuccess = [this, layerId](const QString &newFilename,
		bool addToCanvas,
		const QString &layerName,
		const QString &encoding,
		const QString &vectorFileName) {
		if (addToCanvas)
		{
			QString uri(newFilename);
			if (!layerName.isEmpty())
				uri += "|layername=" + layerName;
			this->addVectorLayers(QStringList(uri), encoding, QStringLiteral("file"));
		}

		// We need to re-retrieve the map layer here, in case it's been deleted during the lifetime of the task
		if (QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(QgsProject::instance()->mapLayer(layerId)))
			this->emit layerSavedAs(vlayer, vectorFileName);

		this->messageBar()->pushMessage(tr("Layer Exported"),
			tr("Successfully saved vector layer to <a href=\"%1\">%2</a>").arg(QUrl::fromLocalFile(newFilename).toString(), QDir::toNativeSeparators(newFilename)),
			Qgis::Success, messageTimeout());
	};

	auto onFailure = [](int error, const QString &errorMessage) {
		if (error != QgsVectorFileWriter::Canceled)
		{
			QgsMessageViewer *m = new QgsMessageViewer(nullptr);
			m->setWindowTitle(tr("Save Error"));
			m->setMessageAsPlainText(tr("Export to vector file failed.\nError: %1").arg(errorMessage));
			m->exec();
		}
	};

	return saveAsVectorFileGeneral(vlayer, symbologyOption, onlySelected, defaultToAddToMap, onSuccess, onFailure);
}


QString DGisApp::saveAsVectorFileGeneral(QgsVectorLayer *vlayer, bool symbologyOption, bool onlySelected, bool defaultToAddToMap, const std::function<void(const QString &, bool, const QString &, const QString &, const QString &)> &onSuccess, const std::function<void(int, const QString &)> &onFailure, int options, const QString &dialogTitle)
{
	QgsCoordinateReferenceSystem destCRS;

	if (!symbologyOption)
	{
		options &= ~QgsVectorLayerSaveAsDialog::Symbology;
	}

	QgsVectorLayerSaveAsDialog *dialog = new QgsVectorLayerSaveAsDialog(vlayer, options, this);
	if (!dialogTitle.isEmpty())
		dialog->setWindowTitle(dialogTitle);

	dialog->setMapCanvas(mMapCanvas);
	dialog->setIncludeZ(QgsWkbTypes::hasZ(vlayer->wkbType()));
	dialog->setOnlySelected(onlySelected);
	dialog->setAddToCanvas(defaultToAddToMap);

	QString vectorFilename;
	if (dialog->exec() == QDialog::Accepted)
	{
		QString encoding = dialog->encoding();
		vectorFilename = dialog->filename();
		QString format = dialog->format();
		QStringList datasourceOptions = dialog->datasourceOptions();
		bool autoGeometryType = dialog->automaticGeometryType();
		QgsWkbTypes::Type forcedGeometryType = dialog->geometryType();

		QgsCoordinateTransform ct;
		destCRS = dialog->crsObject();

		if (destCRS.isValid())
		{
			QgsDatumTransformDialog::run(vlayer->crs(), destCRS, this, mMapCanvas);
			ct = QgsCoordinateTransform(vlayer->crs(), destCRS, QgsProject::instance());
		}

		QgsRectangle filterExtent = dialog->filterExtent();
		QgisAppFieldValueConverter converter(vlayer, dialog->attributesAsDisplayedValues());
		QgisAppFieldValueConverter *converterPtr = nullptr;
		// No need to use the converter if there is nothing to convert
		if (!dialog->attributesAsDisplayedValues().isEmpty())
			converterPtr = &converter;

		QgsVectorFileWriter::SaveVectorOptions options;
		options.driverName = format;
		options.layerName = dialog->layername();
		options.actionOnExistingFile = dialog->creationActionOnExistingFile();
		options.fileEncoding = encoding;
		options.ct = ct;
		options.onlySelectedFeatures = dialog->onlySelected();
		options.datasourceOptions = datasourceOptions;
		options.layerOptions = dialog->layerOptions();
		options.skipAttributeCreation = dialog->selectedAttributes().isEmpty();
		options.symbologyExport = static_cast<QgsVectorFileWriter::SymbologyExport>(dialog->symbologyExport());
		options.symbologyScale = dialog->scale();
		if (dialog->hasFilterExtent())
			options.filterExtent = filterExtent;
		options.overrideGeometryType = autoGeometryType ? QgsWkbTypes::Unknown : forcedGeometryType;
		options.forceMulti = dialog->forceMulti();
		options.includeZ = dialog->includeZ();
		options.attributes = dialog->selectedAttributes();
		options.fieldValueConverter = converterPtr;

		bool addToCanvas = dialog->addToCanvas();
		QgsVectorFileWriterTask *writerTask = new QgsVectorFileWriterTask(vlayer, vectorFilename, options);

		// when writer is successful:
		connect(writerTask, &QgsVectorFileWriterTask::completed, this, [onSuccess, addToCanvas, encoding, vectorFilename](const QString &newFilename, const QString &newLayer) {
			onSuccess(newFilename, addToCanvas, newLayer, encoding, vectorFilename);
		});

		// when an error occurs:
		connect(writerTask, &QgsVectorFileWriterTask::errorOccurred, this, [onFailure](int error, const QString &errorMessage) {
			onFailure(error, errorMessage);
		});

		QgsApplication::taskManager()->addTask(writerTask);
	}

	delete dialog;
	return vectorFilename;
}


bool DGisApp::addVectorLayers(const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType)
{
	return addVectorLayersPrivate(layerQStringList, enc, dataSourceType);
}


bool DGisApp::addVectorLayersPrivate(const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType, const bool guiWarning)
{
	QgsCanvasRefreshBlocker refreshBlocker;

	QList<QgsMapLayer *> layersToAdd;
	QList<QgsMapLayer *> addedLayers;
	QgsSettings settings;
	bool userAskedToAddLayers = false;

	for (QString src : layerQStringList)
	{
		src = src.trimmed();
		QString baseName;
		if (dataSourceType == QLatin1String("file"))
		{
			QString srcWithoutLayername(src);
			int posPipe = srcWithoutLayername.indexOf('|');
			if (posPipe >= 0)
				srcWithoutLayername.resize(posPipe);
			QFileInfo fi(srcWithoutLayername);
			baseName = fi.completeBaseName();

			// if needed prompt for zipitem layers
			QString vsiPrefix = QgsZipItem::vsiPrefix(src);
			if (!src.startsWith(QLatin1String("/vsi"), Qt::CaseInsensitive) &&
				(vsiPrefix == QLatin1String("/vsizip/") || vsiPrefix == QLatin1String("/vsitar/")))
			{
				if (askUserForZipItemLayers(src))
					continue;
			}
		}
		else if (dataSourceType == QLatin1String("database"))
		{
			// Try to extract the database name and use it as base name
			// sublayers names (if any) will be appended to the layer name
			auto parts(QgsProviderRegistry::instance()->decodeUri(QStringLiteral("ogr"), src));
			if (parts.value(QStringLiteral("databaseName")).isValid())
				baseName = parts.value(QStringLiteral("databaseName")).toString();
			else
				baseName = src;
		}
		else //directory //protocol
		{
			QFileInfo fi(src);
			baseName = fi.completeBaseName();
		}
		if (settings.value(QStringLiteral("qgis/formatLayerName"), false).toBool())
		{
			baseName = QgsMapLayer::formatLayerName(baseName);
		}

		QgsDebugMsgLevel("completeBaseName: " + baseName, 2);
		const bool isVsiCurl{ src.startsWith(QLatin1String("/vsicurl"), Qt::CaseInsensitive) };
		const auto scheme{ QUrl(src).scheme() };
		const bool isRemoteUrl{ scheme.startsWith(QLatin1String("http")) || scheme == QLatin1String("ftp") };

		// create the layer
		QgsVectorLayer::LayerOptions options{ QgsProject::instance()->transformContext() };
		options.loadDefaultStyle = false;
		if (isVsiCurl || isRemoteUrl)
		{
			messageBar()->pushInfo(tr("Remote layer"), tr("loading %1, please wait …").arg(src));
			QApplication::setOverrideCursor(Qt::WaitCursor);
			qApp->processEvents();
		}
		QgsVectorLayer *layer = new QgsVectorLayer(src, baseName, QStringLiteral("ogr"), options);
		Q_CHECK_PTR(layer);
		if (isVsiCurl || isRemoteUrl)
		{
			QApplication::restoreOverrideCursor();
		}
		if (!layer)
		{
			// Let render() do its own cursor management
			//      QApplication::restoreOverrideCursor();

			// XXX insert meaningful whine to the user here
			return false;
		}

		if (layer->isValid())
		{
			userAskedToAddLayers = true;
			if (!enc.isEmpty())
				layer->setProviderEncoding(enc);

			QStringList sublayers = layer->dataProvider()->subLayers();
			QgsDebugMsgLevel(QStringLiteral("got valid layer with %1 sublayers").arg(sublayers.count()), 2);

			// If the newly created layer has more than 1 layer of data available, we show the
			// sublayers selection dialog so the user can select the sublayers to actually load.
			if (sublayers.count() > 1)
			{
				addedLayers.append(askUserForOGRSublayers(layer, sublayers));
				// layer is no longer valid and has been nullified

				for (QgsMapLayer *l : addedLayers)
					askUserForDatumTransform(l->crs(), QgsProject::instance()->crs(), l);
			}
			else if (!sublayers.isEmpty()) // there is 1 layer of data available
			{
				setupVectorLayer(src, sublayers, layer,
					QStringLiteral("ogr"), options);

				layersToAdd << layer;
			}
			else
			{
				if (guiWarning)
				{
					QString msg = tr("%1 doesn't have any layers.").arg(src);
					messageBar()->pushMessage(tr("Invalid Data Source"), msg, Qgis::Critical, messageTimeout());
				}
				delete layer;
			}
		}
		else
		{
			// since the layer is bad, stomp on it
			delete layer;
			QString msg = tr("%1 is not a valid or recognized data source.").arg(src);
			// If the failed layer was a vsicurl type, give the user a chance to try the normal download.
			if (isVsiCurl &&
				QMessageBox::question(this, tr("Invalid Data Source"),
					tr("Download with \"Protocol\" source type has failed, do you want to try the \"File\" source type?")) == QMessageBox::Yes)
			{
				return addVectorLayersPrivate(QStringList() << src.replace(QLatin1String("/vsicurl/"), " "), enc, dataSourceType, guiWarning);
			}
			else if (guiWarning)
			{
				messageBar()->pushMessage(tr("Invalid Data Source"), msg, Qgis::Critical, messageTimeout());
			}
		}
	}

	// make sure at least one layer was successfully added
	if (layersToAdd.isEmpty())
	{
		// we also return true if we asked the user for sublayers, but they choose none. In this case nothing
		// went wrong, so we shouldn't return false and cause GUI warnings to appear
		return userAskedToAddLayers || !addedLayers.isEmpty();
	}

	// Register this layer with the layers registry
	QgsProject::instance()->addMapLayers(layersToAdd);
	for (QgsMapLayer *l : qgis::as_const(layersToAdd))
	{
		bool ok;
		l->loadDefaultStyle(ok);
		l->loadDefaultMetadata(ok);
		askUserForDatumTransform(l->crs(), QgsProject::instance()->crs(), l);
	}
	activateDeactivateLayerRelatedActions(activeLayer());

	return true;
}


QgisAppFieldValueConverter::QgisAppFieldValueConverter(QgsVectorLayer *vl, const QgsAttributeList &attributesAsDisplayedValues)
	: mLayer(vl), mAttributesAsDisplayedValues(attributesAsDisplayedValues)
{
}

QgsField QgisAppFieldValueConverter::fieldDefinition(const QgsField &field)
{
	if (!mLayer)
		return field;

	int idx = mLayer->fields().indexFromName(field.name());
	if (mAttributesAsDisplayedValues.contains(idx))
	{
		return QgsField(field.name(), QVariant::String);
	}
	return field;
}

QVariant QgisAppFieldValueConverter::convert(int idx, const QVariant &value)
{
	if (!mLayer || !mAttributesAsDisplayedValues.contains(idx))
	{
		return value;
	}
	const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest(mLayer, mLayer->fields().field(idx).name());
	QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter(setup.type());
	return fieldFormatter->representValue(mLayer, idx, setup.config(), QVariant(), value);
}

QgisAppFieldValueConverter *QgisAppFieldValueConverter::clone() const
{
	return new QgisAppFieldValueConverter(*this);
}

// present a dialog to choose zipitem layers
bool DGisApp::askUserForZipItemLayers(const QString &path)
{
	bool ok = false;
	QVector<QgsDataItem *> childItems;
	QgsZipItem *zipItem = nullptr;
	QgsSettings settings;
	QgsSublayersDialog::PromptMode promptLayers = settings.enumValue(QStringLiteral("qgis/promptForSublayers"), QgsSublayersDialog::PromptAlways);

	QgsDebugMsgLevel("askUserForZipItemLayers( " + path + ')', 2);

	// if scanZipBrowser == no: skip to the next file
	if (settings.value(QStringLiteral("qgis/scanZipInBrowser2"), "basic").toString() == QLatin1String("no"))
	{
		return false;
	}

	zipItem = new QgsZipItem(nullptr, path, path);
	if (!zipItem)
		return false;

	zipItem->populate(true);
	QgsDebugMsgLevel(QStringLiteral("Path= %1 got zipitem with %2 children").arg(path).arg(zipItem->rowCount()), 2);

	// if 1 or 0 child found, exit so a normal item is created by gdal or ogr provider
	if (zipItem->rowCount() <= 1)
	{
		delete zipItem;
		return false;
	}

	switch (promptLayers)
	{
		// load all layers without prompting
	case QgsSublayersDialog::PromptLoadAll:
		childItems = zipItem->children();
		break;
		// return because we should not prompt at all
	case QgsSublayersDialog::PromptNever:
		delete zipItem;
		return false;
		// initialize a selection dialog and display it.
	case QgsSublayersDialog::PromptAlways:
	case QgsSublayersDialog::PromptIfNeeded:
		QgsSublayersDialog chooseSublayersDialog(QgsSublayersDialog::Vsifile, QStringLiteral("vsi"), this, Qt::WindowFlags(), path);
		QgsSublayersDialog::LayerDefinitionList layers;

		for (int i = 0; i < zipItem->children().size(); i++)
		{
			QgsDataItem *item = zipItem->children().at(i);
			QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>(item);
			if (!layerItem)
				continue;

			QgsDebugMsgLevel(QStringLiteral("item path=%1 provider=%2").arg(item->path(), layerItem->providerKey()), 2);

			QgsSublayersDialog::LayerDefinition def;
			def.layerId = i;
			def.layerName = item->name();
			if (layerItem->providerKey() == QLatin1String("gdal"))
			{
				def.type = tr("Raster");
			}
			else if (layerItem->providerKey() == QLatin1String("ogr"))
			{
				def.type = tr("Vector");
			}
			layers << def;
		}

		chooseSublayersDialog.populateLayerTable(layers);

		if (chooseSublayersDialog.exec())
		{
			const auto constSelection = chooseSublayersDialog.selection();
			for (const QgsSublayersDialog::LayerDefinition &def : constSelection)
			{
				childItems << zipItem->children().at(def.layerId);
			}
		}

		break;
	}

	if (childItems.isEmpty())
	{
		// return true so dialog doesn't popup again (#6225) - hopefully this doesn't create other trouble
		ok = true;
	}

	// add childItems
	const auto constChildItems = childItems;
	for (QgsDataItem *item : constChildItems)
	{
		QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>(item);
		if (!layerItem)
			continue;

		QgsDebugMsgLevel(QStringLiteral("item path=%1 provider=%2").arg(item->path(), layerItem->providerKey()), 2);
		if (layerItem->providerKey() == QLatin1String("gdal"))
		{
			//if (addRasterLayer(item->path(), QFileInfo(item->name()).completeBaseName()))
				ok = true;
		}
		else if (layerItem->providerKey() == QLatin1String("ogr"))
		{
			if (addVectorLayers(QStringList(item->path()), QString(), QStringLiteral("file")))
				ok = true;
		}
	}

	delete zipItem;
	return ok;
}

// This method is the method that does the real job. If the layer given in
// parameter is nullptr, then the method tries to act on the activeLayer.
QList<QgsMapLayer *> DGisApp::askUserForOGRSublayers(QgsVectorLayer *&parentLayer, const QStringList &sublayers)
{
	QList<QgsMapLayer *> result;

	QgsSublayersDialog::LayerDefinitionList list;
	QMap<QString, int> mapLayerNameToCount;
	bool uniqueNames = true;
	int lastLayerId = -1;
	const auto constSublayers = sublayers;
	for (const QString &sublayer : constSublayers)
	{
		// OGR provider returns items in this format:
		// <layer_index>:<name>:<feature_count>:<geom_type>

		QStringList elements = splitSubLayerDef(sublayer);
		if (elements.count() >= 4)
		{
			QgsSublayersDialog::LayerDefinition def;
			def.layerId = elements[0].toInt();
			def.layerName = elements[1];
			def.count = elements[2].toInt();
			def.type = elements[3];
			// ignore geometry column name at elements[4]
			if (elements.count() >= 6)
				def.description = elements[5];
			if (lastLayerId != def.layerId)
			{
				int count = ++mapLayerNameToCount[def.layerName];
				if (count > 1 || def.layerName.isEmpty())
					uniqueNames = false;
				lastLayerId = def.layerId;
			}
			list << def;
		}
		else
		{
			QgsDebugMsg("Unexpected output from OGR provider's subLayers()! " + sublayer);
		}
	}

	// Check if the current layer uri contains the

	// We initialize a selection dialog and display it.
	QgsSublayersDialog chooseSublayersDialog(QgsSublayersDialog::Ogr, QStringLiteral("ogr"), this, Qt::WindowFlags(), parentLayer->dataProvider()->dataSourceUri());
	chooseSublayersDialog.setShowAddToGroupCheckbox(true);
	chooseSublayersDialog.populateLayerTable(list);

	if (!chooseSublayersDialog.exec())
		return result;

	const bool addToGroup = chooseSublayersDialog.addToGroupCheckbox();

	QString name = parentLayer->name();

	auto uriParts = QgsProviderRegistry::instance()->decodeUri(
		parentLayer->providerType(), parentLayer->dataProvider()->dataSourceUri());
	QString uri(uriParts.value(QStringLiteral("path")).toString());
	QStringList openOptions(uriParts.value(QStringLiteral("openOptions")).toStringList());

	// The uri must contain the actual uri of the vectorLayer from which we are
	// going to load the sublayers.
	QString fileName = QFileInfo(uri).baseName();
	const auto constSelection = chooseSublayersDialog.selection();
	const QString providerType = parentLayer->providerType();

	// We delete the parent layer now, to be sure in the GeoPackage case that
	// when several sublayers are selected, they will use the same GDAL dataset
	// This is critical to make project transactions work, as in
	// https://github.com/qgis/QGIS/issues/39431#issuecomment-713460189
	delete parentLayer;
	parentLayer = nullptr;

	for (const QgsSublayersDialog::LayerDefinition &def : constSelection)
	{
		QVariantMap newUriParts;
		newUriParts.insert(QStringLiteral("path"), uri);
		QString layerGeometryType = def.type;
		if (uniqueNames)
		{
			newUriParts.insert(QStringLiteral("layerName"), def.layerName);
		}
		else
		{
			// Only use layerId if there are ambiguities with names
			newUriParts.insert(QStringLiteral("layerId"), QString::number(def.layerId));
		}

		if (!layerGeometryType.isEmpty())
		{
			newUriParts.insert(QStringLiteral("geometryType"), layerGeometryType);
		}
		if (!openOptions.isEmpty())
		{
			newUriParts.insert(QStringLiteral("openOptions"), openOptions);
		}

		QString composedURI = QgsProviderRegistry::instance()->encodeUri(
			providerType, newUriParts);
		QgsDebugMsgLevel("Creating new vector layer using " + composedURI, 2);

		// if user has opted to add sublayers to a group, then we don't need to include the
		// filename in the layer's name, because the group is already titled with the filename.
		// But otherwise, we DO include the file name so that users can differentiate the source
		// when multiple layers are loaded from a GPX file or similar (refs https://github.com/qgis/QGIS/issues/37551)
		const QString name = addToGroup ? def.layerName : fileName + " " + def.layerName;

		QgsVectorLayer::LayerOptions options{ QgsProject::instance()->transformContext() };
		options.loadDefaultStyle = false;
		QgsVectorLayer *layer = new QgsVectorLayer(composedURI, name, QStringLiteral("ogr"), options);
		if (layer && layer->isValid())
		{
			result << layer;
		}
		else
		{
			QString msg = tr("%1 is not a valid or recognized data source").arg(composedURI);
			messageBar()->pushMessage(tr("Invalid Data Source"), msg, Qgis::Critical, messageTimeout());
			delete layer;
		}
	}

	if (!result.isEmpty())
	{
		QgsSettings settings;
		bool newLayersVisible = settings.value(QStringLiteral("/qgis/new_layers_visible"), true).toBool();
		QgsLayerTreeGroup *group = nullptr;
		if (addToGroup)
			group = QgsProject::instance()->layerTreeRoot()->insertGroup(0, name);

		QgsProject::instance()->addMapLayers(result, !addToGroup);
		for (QgsMapLayer *l : qgis::as_const(result))
		{
			bool ok;
			l->loadDefaultStyle(ok);
			l->loadDefaultMetadata(ok);
			if (addToGroup)
				group->addLayer(l);
		}

		// Respect if user don't want the new group of layers visible.
		if (addToGroup && !newLayersVisible)
			group->setItemVisibilityCheckedRecursive(newLayersVisible);
	}
	return result;
}

//----------------------------------------file new / save as START----------------------------------------------

bool DGisApp::fileNew()
{
	return fileNew(true); // prompts whether to save project
} // fileNew()

bool DGisApp::fileNewBlank()
{
	return fileNew(true, true);
}

void DGisApp::fileClose()
{
	if (fileNewBlank())
		mCentralContainer->setCurrentIndex(1);
}

bool DGisApp::checkTasksDependOnProject()
{
	QSet<QString> activeTaskDescriptions;
	QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
	QMap<QString, QgsMapLayer *>::const_iterator layerIt = layers.constBegin();

	for (; layerIt != layers.constEnd(); ++layerIt)
	{
		QList<QgsTask *> tasks = QgsApplication::taskManager()->tasksDependentOnLayer(layerIt.value());
		if (!tasks.isEmpty())
		{
			const auto constTasks = tasks;
			for (QgsTask *task : constTasks)
			{
				activeTaskDescriptions.insert(tr(" * %1").arg(task->description()));
			}
		}
	}

	if (!activeTaskDescriptions.isEmpty())
	{
		QMessageBox::warning(this, tr("Active Tasks"),
			tr("The following tasks are currently running which depend on layers in this project:\n\n%1\n\nPlease cancel these tasks and retry.").arg(qgis::setToList(activeTaskDescriptions).join(QLatin1Char('\n'))));
		return true;
	}
	return false;
}

//as file new but accepts flags to indicate whether we should prompt to save
bool DGisApp::fileNew(bool promptToSaveFlag, bool forceBlank)
{
	if (checkTasksDependOnProject())
		return false;

	if (promptToSaveFlag)
	{
		if (!checkUnsavedLayerEdits() || !checkMemoryLayers() || !saveDirty())
		{
			return false; //cancel pressed
		}
	}

	QgsSettings settings;

	MAYBE_UNUSED QgsProjectDirtyBlocker dirtyBlocker(QgsProject::instance());
	QgsCanvasRefreshBlocker refreshBlocker;
	closeProject();

	QgsProject *prj = QgsProject::instance();
	prj->layerTreeRegistryBridge()->setNewLayersVisible(settings.value(QStringLiteral("qgis/new_layers_visible"), true).toBool());

	//set the canvas to the default project background color
	//apply Project settings To Canvas
	mMapCanvas->setCanvasColor(QgsProject::instance()->backgroundColor());
	mMapCanvas->setSelectionColor(QgsProject::instance()->selectionColor());

	prj->setDirty(false);

	//setTitleBarText_(*this);

	// emit signal so listeners know we have a new project
	emit newProject();

	mMapCanvas->clearExtentHistory();
	mMapCanvas->setRotation(0.0);
	//mScaleWidget->updateScales();

	// set project CRS
	const QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem(settings.value(QStringLiteral("/projections/defaultProjectCrs"), geoEpsgCrsAuthId(), QgsSettings::App).toString());
	// write the projections _proj string_ to project settings
	const bool planimetric = settings.value(QStringLiteral("measure/planimetric"), true, QgsSettings::Core).toBool();
	prj->setCrs(srs, !planimetric); // If the default ellipsoid is not planimetric, set it from the default crs
	if (planimetric)
		prj->setEllipsoid(geoNone());

	/* New Empty Project Created
		(before attempting to load custom project templates/filepaths) */

		// load default template
		/* NOTE: don't open default template on launch until after initialization,
				 in case a project was defined via command line */

				 // don't open template if last auto-opening of a project failed
	if (!forceBlank)
	{
		forceBlank = !settings.value(QStringLiteral("qgis/projOpenedOKAtLaunch"), QVariant(true)).toBool();
	}

	if (!forceBlank && settings.value(QStringLiteral("qgis/newProjectDefault"), QVariant(false)).toBool())
	{
		fileNewFromDefaultTemplate();
	}

	// set the initial map tool
	mMapCanvas->setMapTool(mMapTools.mPan);
	mNonEditMapTool = mMapTools.mPan; // signals are not yet setup to catch this

	prj->setDirty(false);
	return true;
}

bool DGisApp::fileNewFromTemplate(const QString &fileName)
{
	if (checkTasksDependOnProject())
		return false;

	if (!checkUnsavedLayerEdits() || !checkMemoryLayers() || !saveDirty())
	{
		return false; //cancel pressed
	}

	MAYBE_UNUSED QgsProjectDirtyBlocker dirtyBlocker(QgsProject::instance());
	QgsDebugMsgLevel(QStringLiteral("loading project template: %1").arg(fileName), 2);
	if (addProject(fileName))
	{
		// set null filename so we don't override the template
		QgsProject::instance()->setFileName(QString());
		return true;
	}
	return false;
}

void DGisApp::fileNewFromDefaultTemplate()
{
	QString projectTemplate = QgsApplication::qgisSettingsDirPath() + QStringLiteral("project_default.qgs");
	QString msgTxt;
	if (!projectTemplate.isEmpty() && QFile::exists(projectTemplate))
	{
		if (fileNewFromTemplate(projectTemplate))
		{
			return;
		}
		msgTxt = tr("Default failed to open: %1");
	}
	else
	{
		msgTxt = tr("Default not found: %1");
	}
	messageBar()->pushMessage(tr("Open Template Project"),
		msgTxt.arg(projectTemplate),
		Qgis::Warning);
}

bool DGisApp::saveDirty()
{
	QString whyDirty;
	bool hasUnsavedEdits = false;
	// extra check to see if there are any vector layers with unsaved provider edits
	// to ensure user has opportunity to save any editing
	if (QgsProject::instance()->count() > 0)
	{
		QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
		for (QMap<QString, QgsMapLayer *>::iterator it = layers.begin(); it != layers.end(); ++it)
		{
			QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>(it.value());
			// note that we skip the unsaved edits check for memory layers -- it's misleading, because their contents aren't actually
			// saved if this is part of a project close operation. Instead we let these get picked up by checkMemoryLayers().
			if (!vl || vl->providerType() == QLatin1String("memory"))
			{
				continue;
			}

			hasUnsavedEdits = (vl->isEditable() && vl->isModified());
			if (hasUnsavedEdits)
			{
				break;
			}
		}

		if (hasUnsavedEdits)
		{
			// notify the project that there was a change
			QgsProject::instance()->setDirty(true);
			whyDirty = QStringLiteral("<p style='color:darkred;'>");
			whyDirty += tr("Project has layer(s) in edit mode with unsaved edits, which will NOT be saved!");
			whyDirty += QLatin1String("</p>");
		}
	}

	QMessageBox::StandardButton answer(QMessageBox::Discard);
	QgsCanvasRefreshBlocker refreshBlocker;

	QgsSettings settings;
	bool askThem = settings.value(QStringLiteral("qgis/askToSaveProjectChanges"), true).toBool();

	if (askThem && QgsProject::instance()->isDirty())
	{
		// flag project as dirty since dirty state of canvas is reset if "dirty"
		// is based on a zoom or pan
		QgsProject::instance()->setDirty(true);		// notify the project that there was a change

		// prompt user to save
		answer = QMessageBox::question(this, tr("Save Project"),
			tr("Do you want to save the current project? %1")
			.arg(whyDirty),
			QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard,
			hasUnsavedEdits ? QMessageBox::Cancel : QMessageBox::Save);
		if (QMessageBox::Save == answer)
		{
			if (!fileSave())
				answer = QMessageBox::Cancel;
		}
	}

	if (answer == QMessageBox::Cancel)
		return false;

	// for memory layers, we discard all unsaved changes manually. Users have already been warned about
	// these by an earlier call to checkMemoryLayers(), and we don't want duplicate "unsaved changes" prompts
	// and anyway, saving the changes to a memory layer here won't actually save ANYTHING!
	// we do this at the very end here, because if the user opted to cancel above then ALL unsaved
	// changes in memory layers should still exist for them.
	const QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
	for (auto it = layers.begin(); it != layers.end(); ++it)
	{
		if (QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>(it.value()))
		{
			if (vl->providerType() == QLatin1String("memory") && vl->isEditable() && vl->isModified())
			{
				vl->rollBack();
			}
		}
	}

	return true;
}

bool DGisApp::checkUnsavedLayerEdits()
{
	// check to see if there are any vector layers with unsaved provider edits
	// to ensure user has opportunity to save any editing
	if (QgsProject::instance()->count() > 0)
	{
		const QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
		for (auto it = layers.begin(); it != layers.end(); ++it)
		{
			if (QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>(it.value()))
			{
				// note that we skip the unsaved edits check for memory layers -- it's misleading, because their contents aren't actually
				// saved if this is part of a project close operation. Instead we let these get picked up by checkMemoryLayers()
				if (!vl->dataProvider() || vl->providerType() == QLatin1String("memory"))
					continue;

				const bool hasUnsavedEdits = (vl->isEditable() && vl->isModified());
				if (!hasUnsavedEdits)
					continue;

				if (!toggleEditing(vl, true))
					return false;
			}
		}
	}

	return true;
}

bool DGisApp::checkMemoryLayers()
{
	if (!QgsSettings().value(QStringLiteral("askToSaveMemoryLayers"), true, QgsSettings::App).toBool())
		return true;

	// check to see if there are any temporary layers present (with features)
	bool hasTemporaryLayers = false;
	bool hasMemoryLayers = false;

	const QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
	for (auto it = layers.begin(); it != layers.end(); ++it)
	{
		if (it.value() && it.value()->providerType() == QLatin1String("memory"))
		{
			QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>(it.value());
			if (vl && vl->featureCount() != 0 && !vl->customProperty(QStringLiteral("skipMemoryLayersCheck")).toInt())
			{
				hasMemoryLayers = true;
				break;
			}
		}
		else if (it.value() && it.value()->isTemporary())
		{
			hasTemporaryLayers = true;
		}
	}

	bool close = true;
	if (hasTemporaryLayers)
		close &= QMessageBox::warning(this,
			tr("Close Project"),
			tr("This project includes one or more temporary layers. These layers are not permanently saved and their contents will be lost. Are you sure you want to proceed?"),
			QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Yes;
	else if (hasMemoryLayers)
		// use the more specific warning for memory layers
		close &= QMessageBox::warning(this,
			tr("Close Project"),
			tr("This project includes one or more temporary scratch layers. These layers are not saved to disk and their contents will be permanently lost. Are you sure you want to proceed?"),
			QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Yes;

	return close;
}

void DGisApp::closeProject()
{
	QgsCanvasRefreshBlocker refreshBlocker;

	mLegendExpressionFilterButton->setExpressionText(QString());
	mLegendExpressionFilterButton->setChecked(false);
	mActionFilterLegend->setChecked(false);

	// ensure layout widgets are fully deleted
	QgsApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

	// clear out any stuff from project
	mMapCanvas->setLayers(QList<QgsMapLayer *>());
	mMapCanvas->clearCache();

	// Avoid unnecessary layer changed handling for each layer removed - instead,
	// defer the handling until we've removed all layers
	QgsProject::instance()->clear();

	onActiveLayerChanged(activeLayer());
}

bool DGisApp::addProject(const QString &projectFile)
{
	QgsCanvasRefreshBlocker refreshBlocker;

	bool returnCode = false;
	std::unique_ptr<QgsProjectDirtyBlocker> dirtyBlocker = qgis::make_unique<QgsProjectDirtyBlocker>(QgsProject::instance());
	QObject connectionScope; // manually control scope of layersChanged lambda connection - we need the connection automatically destroyed when this function finishes
	bool badLayersHandled = false;
	//connect(mAppBadLayersHandler, &QgsHandleBadLayersHandler::layersChanged, &connectionScope, [&badLayersHandled] { badLayersHandled = true; });

	// close the previous opened project if any
	closeProject();

	QFileInfo pfi(projectFile);
	//mStatusBar->showMessage(tr("Loading project: %1").arg(pfi.fileName()));
	qApp->processEvents();

	QApplication::setOverrideCursor(Qt::WaitCursor);

	bool autoSetupOnFirstLayer = mLayerTreeCanvasBridge->autoSetupOnFirstLayer();
	mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer(false);

	// give custom handlers a chance first
	bool usedCustomHandler = false;
	bool customHandlerWantsThumbnail = false;
	QIcon customHandlerIcon;
	for (QgsCustomProjectOpenHandler *handler : qgis::as_const(mCustomProjectOpenHandlers))
	{
		if (handler && handler->handleProjectOpen(projectFile))
		{
			usedCustomHandler = true;
			customHandlerWantsThumbnail = handler->createDocumentThumbnailAfterOpen();
			customHandlerIcon = handler->icon();
			break;
		}
	}

	if (!usedCustomHandler && !QgsProject::instance()->read(projectFile) && !QgsZipUtils::isZipFile(projectFile))
	{
		QString backupFile = projectFile + "~";
		QString loadBackupPrompt;
		QMessageBox::StandardButtons buttons;
		if (QFile(backupFile).exists())
		{
			loadBackupPrompt = "\n\n" + tr("Do you want to open the backup file\n%1\ninstead?").arg(backupFile);
			buttons |= QMessageBox::Yes;
			buttons |= QMessageBox::No;
		}
		else
		{
			buttons |= QMessageBox::Ok;
		}
		QApplication::restoreOverrideCursor();
		//mStatusBar->clearMessage();

		int r = QMessageBox::critical(this,
			tr("Unable to open project"),
			QgsProject::instance()->error() + loadBackupPrompt,
			buttons);

		if (QMessageBox::Yes == r && addProject(backupFile))
		{
			// We loaded data from the backup file, but we pretend to work on the original project file.
			QgsProject::instance()->setFileName(projectFile);
			QgsProject::instance()->setDirty(true);
			mProjectLastModified = QgsProject::instance()->lastModified();
			returnCode = true;
		}
		else
		{
			returnCode = false;
		}
	}
	else
	{

		mProjectLastModified = QgsProject::instance()->lastModified();

		//setTitleBarText_(*this);
		//mOverviewCanvas->setBackgroundColor(QgsProject::instance()->backgroundColor());

		//applyProjectSettingsToCanvas(mMapCanvas);

		//load project scales
		bool projectScales = QgsProject::instance()->viewSettings()->useProjectScales();
		if (projectScales)
		{
			//mScaleWidget->updateScales();
		}

		mMapCanvas->updateScale();
		QgsDebugMsgLevel(QStringLiteral("Scale restored..."), 3);

		mActionFilterLegend->setChecked(QgsProject::instance()->readBoolEntry(QStringLiteral("Legend"), QStringLiteral("filterByMap")));

		// Select the first layer
		if (mLayerTreeView->layerTreeModel()->rootGroup()->findLayers().count() > 0)
		{
			mLayerTreeView->setCurrentLayer(mLayerTreeView->layerTreeModel()->rootGroup()->findLayers().at(0)->layer());
		}

		QgsSettings settings;
		//TODO
//#ifdef WITH_BINDINGS
//		// does the project have any macros?
//		if (mPythonUtils && mPythonUtils->isEnabled())
//		{
//			if (!QgsProject::instance()->readEntry(QStringLiteral("Macros"), QStringLiteral("/pythonCode"), QString()).isEmpty())
//			{
//				auto lambda = []() { QgisApp::instance()->enableProjectMacros(); };
//				QgsGui::pythonMacroAllowed(lambda, mInfoBar);
//			}
//		}
//#endif

		// Check for missing layer widget dependencies
		const auto constVLayers{ QgsProject::instance()->layers<QgsVectorLayer *>() };
		for (QgsVectorLayer *vl : constVLayers)
		{
			if (vl->isValid())
			{
				//resolveVectorLayerDependencies(vl);
			}
		}

		emit projectRead(); // let plug-ins know that we've read in a new
		// project so that they can check any project
		// specific plug-in state

		// add this to the list of recently used project files
		// if a custom handler was used, then we generate a thumbnail
		//if (!usedCustomHandler || !customHandlerWantsThumbnail)
			//saveRecentProjectPath(false);
		//else 
		if (!QgsProject::instance()->originalPath().isEmpty())
		{
			// we have to delay the thumbnail creation until after the canvas has refreshed for the first time
			QMetaObject::Connection *connection = new QMetaObject::Connection();
			*connection = connect(mMapCanvas, &QgsMapCanvas::mapCanvasRefreshed, [=]() {
				QObject::disconnect(*connection);
				delete connection;
				//saveRecentProjectPath(true, customHandlerIcon);
			});
		}

		QApplication::restoreOverrideCursor();

		if (autoSetupOnFirstLayer)
			mLayerTreeCanvasBridge->setAutoSetupOnFirstLayer(true);

		//mStatusBar->showMessage(tr("Project loaded"), 3000);
		returnCode = true;
	}

	if (badLayersHandled)
	{
		dirtyBlocker.reset(); // allow project dirtying again
		QgsProject::instance()->setDirty(true);
	}

	return returnCode;
} // QgisApp::addProject(QString projectFile)

bool DGisApp::fileSave()
{
	// if we don't have a file name, then obviously we need to get one; note
	// that the project file name is reset to null in fileNew()

	if (QgsProject::instance()->fileName().isNull())
	{
		// Retrieve last used project dir from persistent settings
		QgsSettings settings;
		QString lastUsedDir = settings.value(QStringLiteral("UI/lastProjectDir"), QDir::homePath()).toString();

		const QString qgsExt = tr("QGIS files") + " (*.qgs)";
		const QString zipExt = tr("QGZ files") + " (*.qgz)";

		QString exts;
		QgsProject::FileFormat defaultProjectFileFormat = settings.enumValue(QStringLiteral("/qgis/defaultProjectFileFormat"), QgsProject::FileFormat::Qgz);
		if (defaultProjectFileFormat == QgsProject::FileFormat::Qgs)
		{
			exts = qgsExt + QStringLiteral(";;") + zipExt;
		}
		else
		{
			exts = zipExt + QStringLiteral(";;") + qgsExt;
		}
		QString filter;
		QString path = QFileDialog::getSaveFileName(
			this,
			tr("Choose a QGIS project file"),
			lastUsedDir + '/' + QgsProject::instance()->title(),
			exts, &filter);
		if (path.isEmpty())
			return false;

		QFileInfo fullPath;
		fullPath.setFile(path);

		// make sure we have the .qgs extension in the file name
		if (filter == zipExt)
		{
			if (fullPath.suffix().compare(QLatin1String("qgz"), Qt::CaseInsensitive) != 0)
				fullPath.setFile(fullPath.filePath() + ".qgz");
		}
		else
		{
			if (fullPath.suffix().compare(QLatin1String("qgs"), Qt::CaseInsensitive) != 0)
				fullPath.setFile(fullPath.filePath() + ".qgs");
		}

		QgsProject::instance()->setFileName(fullPath.filePath());
	}
	else
	{
		bool usingProjectStorage = QgsProject::instance()->projectStorage();
		bool fileExists = usingProjectStorage ? true : QFileInfo::exists(QgsProject::instance()->fileName());

		if (fileExists && !mProjectLastModified.isNull() && mProjectLastModified != QgsProject::instance()->lastModified())
		{
			if (QMessageBox::warning(this,
				tr("Open a Project"),
				tr("The loaded project file on disk was meanwhile changed. Do you want to overwrite the changes?\n"
					"\nLast modification date on load was: %1"
					"\nCurrent last modification date is: %2")
				.arg(mProjectLastModified.toString(Qt::DefaultLocaleLongDate),
					QgsProject::instance()->lastModified().toString(Qt::DefaultLocaleLongDate)),
				QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
				return false;
		}

		if (fileExists && !usingProjectStorage && !QFileInfo(QgsProject::instance()->fileName()).isWritable())
		{
			messageBar()->pushMessage(tr("Insufficient permissions"),
				tr("The project file is not writable."),
				Qgis::Warning);
			return false;
		}
	}

	// Store current map view settings into the project
	QgsProject::instance()->viewSettings()->setDefaultViewExtent(QgsReferencedRectangle(mapCanvas()->extent(), QgsProject::instance()->crs()));

	if (QgsProject::instance()->write())
	{
		//setTitleBarText_(*this); // update title bar
		//mStatusBar->showMessage(tr("Saved project to: %1").arg(QDir::toNativeSeparators(QgsProject::instance()->fileName())), 5000);

		//saveRecentProjectPath();

		mProjectLastModified = QgsProject::instance()->lastModified();
	}
	else
	{
		QMessageBox::critical(this,
			tr("Unable to save project %1").arg(QDir::toNativeSeparators(QgsProject::instance()->fileName())),
			QgsProject::instance()->error());
		return false;
	}

	// run the saved project macro
	//if (mPythonMacrosEnabled)
	//{
	//	QgsPythonRunner::run(QStringLiteral("qgis.utils.saveProjectMacro();"));
	//}

	return true;
} // QgisApp::fileSave

void DGisApp::fileOpen()
{
	if (checkTasksDependOnProject())
		return;

	// possibly save any pending work before opening a new project
	if (checkUnsavedLayerEdits() && checkMemoryLayers() && saveDirty())
	{
		// Retrieve last used project dir from persistent settings
		QgsSettings settings;
		QString lastUsedDir = settings.value(QStringLiteral("UI/lastProjectDir"), QDir::homePath()).toString();

		QStringList fileFilters;
		QStringList extensions;
		fileFilters << tr("QGIS files") + QStringLiteral(" (*.qgs *.qgz *.QGS *.QGZ)");
		extensions << QStringLiteral("qgs") << QStringLiteral("qgz");
		for (QgsCustomProjectOpenHandler *handler : qgis::as_const(mCustomProjectOpenHandlers))
		{
			if (handler)
			{
				const QStringList filters = handler->filters();
				fileFilters.append(filters);
				for (const QString &filter : filters)
					extensions.append(QgsFileUtils::extensionsFromFilter(filter));
			}
		}

		// generate master "all projects" extension list
		QString allEntry = tr("All Project Files") + QStringLiteral(" (");
		for (const QString &extension : extensions)
			allEntry += QStringLiteral("*.%1 *.%2 ").arg(extension.toLower(), extension.toUpper());
		allEntry.chop(1); // remove trailing ' '
		allEntry += ')';
		fileFilters.insert(0, allEntry);

		QString fullPath = QFileDialog::getOpenFileName(this,
			tr("Open Project"),
			lastUsedDir,
			fileFilters.join(QLatin1String(";;")));
		if (fullPath.isNull())
		{
			return;
		}

		QFileInfo myFI(fullPath);
		QString myPath = myFI.path();
		// Persist last used project dir
		settings.setValue(QStringLiteral("UI/lastProjectDir"), myPath);

		// open the selected project
		addProject(fullPath);
	}
}
//----------------------------------------file new / save as END------------------------------------------------

//-------------------------------------------layer new / edit.. START-----------------------------------------------
void DGisApp::newVectorLayer()
{
	QString enc;
	QString error;
	QString fileName = QgsNewVectorLayerDialog::execAndCreateLayer(error, this, QString(), &enc, QgsProject::instance()->defaultCrsForNewLayers());

	if (!fileName.isEmpty())
	{
		//then add the layer to the view
		QStringList fileNames;
		fileNames.append(fileName);
		//todo: the last parameter will change accordingly to layer type
		addVectorLayers(fileNames, enc, QStringLiteral("file"));
	}
	//else if (!error.isEmpty())
	//{
	//	QLabel *msgLabel = new QLabel(tr("Layer creation failed: %1").arg(error), messageBar());
	//	msgLabel->setWordWrap(true);
	//	connect(msgLabel, &QLabel::linkActivated, mLogDock, &QWidget::show);
	//	QgsMessageBarItem *item = new QgsMessageBarItem(msgLabel, Qgis::Critical);
	//	messageBar()->pushItem(item);
	//}
}

void DGisApp::newMemoryLayer()
{
	QgsVectorLayer *newLayer = QgsNewMemoryLayerDialog::runAndCreateLayer(this, QgsProject::instance()->defaultCrsForNewLayers());

	if (newLayer)
	{
		//then add the layer to the view
		QList<QgsMapLayer *> layers;
		layers << newLayer;

		QgsProject::instance()->addMapLayers(layers);
		newLayer->startEditing();
	}
}

void DGisApp::commitError(QgsVectorLayer *vlayer)
{
	const QStringList commitErrors = vlayer->commitErrors();
	if (!vlayer->allowCommit() && commitErrors.empty())
	{
		return;
	}

	QgsMessageViewer *mv = new QgsMessageViewer();
	mv->setWindowTitle(tr("Commit Errors"));
	mv->setMessageAsPlainText(tr("Could not commit changes to layer %1").arg(vlayer->name()) + "\n\n" + tr("Errors: %1\n").arg(commitErrors.join(QLatin1String("\n  "))));

	QToolButton *showMore = new QToolButton();
	// store pointer to vlayer in data of QAction
	QAction *act = new QAction(showMore);
	act->setData(QVariant(QMetaType::QObjectStar, &vlayer));
	act->setText(tr("Show more"));
	showMore->setStyleSheet(QStringLiteral("background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline;"));
	showMore->setCursor(Qt::PointingHandCursor);
	showMore->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	showMore->addAction(act);
	showMore->setDefaultAction(act);
	connect(showMore, &QToolButton::triggered, mv, &QDialog::exec);
	connect(showMore, &QToolButton::triggered, showMore, &QObject::deleteLater);

	// no timeout set, since notice needs attention and is only shown first time layer is labeled
	QgsMessageBarItem *errorMsg = new QgsMessageBarItem(
		tr("Commit errors"),
		tr("Could not commit changes to layer %1").arg(vlayer->name()),
		showMore,
		Qgis::Warning,
		0,
		messageBar());
	messageBar()->pushItem(errorMsg);
}

//---------------------------------------------python-----------------------------------------------
void DGisApp::loadPythonSupport()
{
	QString pythonlibName(QStringLiteral("qgispython"));
#if defined(Q_OS_UNIX)
	pythonlibName.prepend(QgsApplication::libraryPath());
#endif
#ifdef __MINGW32__
	pythonlibName.prepend("lib");
#endif
	QString version = QStringLiteral("%1.%2.%3").arg(Qgis::versionInt() / 10000).arg(Qgis::versionInt() / 100 % 100).arg(Qgis::versionInt() % 100);
	QgsDebugMsgLevel(QStringLiteral("load library %1 (%2)").arg(pythonlibName, version), 2);
	QLibrary pythonlib(pythonlibName, version);
	// It's necessary to set these two load hints, otherwise Python library won't work correctly
	// see http://lists.kde.org/?l=pykde&m=117190116820758&w=2
	pythonlib.setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::ExportExternalSymbolsHint);
	if (!pythonlib.load())
	{
		pythonlib.setFileName(pythonlibName);
		if (!pythonlib.load())
		{
			qDebug() << QString("Couldn't load Python support library: %1").arg(pythonlib.errorString());
			return;
		}
	}

	//  typedef QgsPythonUtils*( *inst )();
	//    //inst是一个函数指针，函数列表为()，返回值为QgsPythonUtils*
	//    //pythonlib_inst()等价于python库中的instance()，其返回值为QgsPythonUtils*
	//    inst pythonlib_inst = reinterpret_cast< inst >( cast_to_fptr( pythonlib.resolve( "instance" ) ) );
	//    if ( !pythonlib_inst )
	//    {
	//      //using stderr on purpose because we want end users to see this [TS]
	//       qDebug() << tr( "Couldn't resolve python support library's instance() symbol." );
	//      return;
	//    }

	mPythonRunner = DPythonUtils::Instance();
	mPythonRunner->initPython(mDInterface, true);
}

void DGisApp::testPython()
{
	//testDockW = new QgsDockWidget(this);
	//QVBoxLayout *boxLayout = new QVBoxLayout;
	//boxLayout->setContentsMargins(0, 0, 0, 0);
	//boxLayout->setSpacing(0);

	//editIuput = new QTextEdit(testDockW);
	//editOut = new QTextEdit(testDockW);
	//QPushButton *btn = new QPushButton(testDockW);

	//editOut->setStyleSheet("color:red");

	//boxLayout->addWidget(editIuput);
	//boxLayout->addWidget(editOut);
	//boxLayout->addWidget(btn);

	//QWidget *w = new QWidget;
	//w->setLayout(boxLayout);
	//testDockW->setWidget(w);
	//addDockWidget(Qt::RightDockWidgetArea, testDockW);

	//connect(btn, &QPushButton::pressed, [&] {
	//	mPythonRunner->runString(editIuput->toPlainText());
	//	editOut->setText(mPythonRunner->getLastRes());
	//});

	//QString providerKey = "wms";
	//QgsDataSourceUri uri;
	//uri.setParam("type", "xyz");
	//QString url = "https://demo.mapserver.org/cgi-bin/wms";
	//uri.setParam("url", url);
	//QgsRasterLayer *layer = new QgsRasterLayer(uri.encodedUri(), QStringLiteral("map"), providerKey);
	//bool flag = layer->isValid();
	//addRasterLayer(layer);

	//test2
	// 创建临时面 (Polygon) 图层，使用坐标系 EPSG:4326 (WGS 84)
	//mpStratchLayer = new QgsVectorLayer("polygon?crs=epsg:4326", u8"tempL", "memory");
	//mMapCanvas->setLayers(QList<QgsMapLayer*>() << mpStratchLayer);

	//// 将临时图层添加到画布，并将画布缩放到 WGS 84 坐标系的边界范围
	//mMapCanvas->setCurrentLayer(mpStratchLayer);
	//mMapCanvas->setExtent(QgsCoordinateReferenceSystem("EPSG:4326").bounds());
	//mpStratchLayer->startEditing();
}

void DGisApp::runScript(const QString &filePath)
{
	QgsSettings settings;
	bool showScriptWarning = settings.value(QStringLiteral("UI/showScriptWarning"), true).toBool();

	QMessageBox msgbox;
	if (showScriptWarning)
	{
		msgbox.setWindowTitle(tr("Security warning"));
		msgbox.setText(tr("Executing a script from an untrusted source can harm your computer. Only continue if you trust the source of the script. Continue?"));
		msgbox.setIcon(QMessageBox::Icon::Warning);
		msgbox.addButton(QMessageBox::Yes);
		msgbox.addButton(QMessageBox::No);
		msgbox.setDefaultButton(QMessageBox::No);
		QCheckBox *cb = new QCheckBox(tr("Don't show this again."));
		msgbox.setCheckBox(cb);
		msgbox.exec();
		settings.setValue(QStringLiteral("UI/showScriptWarning"), !msgbox.checkBox()->isChecked());
	}

	if (!showScriptWarning || msgbox.result() == QMessageBox::Yes)
	{
		this->mPythonRunner->runString(QString("qgis.utils.run_script_from_file(\"%1\")").arg(filePath),
			tr("Failed to run Python script:"), false);
	}
	Q_UNUSED(filePath)
}

//--------------------------------------------------------edit functions START----------------------------------------------------------------
void DGisApp::toggleEditing()
{
	QgsVectorLayer *currentLayer = qobject_cast<QgsVectorLayer *>(activeLayer());
	if (currentLayer)
	{
		toggleEditing(currentLayer, true);
	}
	else
	{
		// active although there's no layer active!?
		ui.mActionToggleEditing->setChecked(false);
	}
}


void DGisApp::splitFeatures()
{
	mMapCanvas->setMapTool(mMapTools.mSplitFeatures);
}

void DGisApp::addFeature()
{
	mMapCanvas->setMapTool(mMapTools.mAddFeature);
}

void DGisApp::moveFeature()
{
	mMapCanvas->setMapTool(mMapTools.mMoveFeature);
}

void DGisApp::moveFeatureCopy()
{
	mMapCanvas->setMapTool(mMapTools.mMoveFeatureCopy);
}

void DGisApp::enableDigitizeWithCurveAction(bool enable)
{
	QgsSettings settings;

	QObject *sender = QObject::sender();

	enable &= (sender == ui.mActionAddFeature && mMapTools.mAddFeature->mode() != QgsMapToolCapture::CapturePoint) ||
		(mMapCanvas->mapTool() == mMapTools.mAddFeature && mMapTools.mAddFeature->mode() != QgsMapToolCapture::CapturePoint) ||
		sender == ui.mActionSplitFeatures ||
		mMapCanvas->mapTool() == mMapTools.mSplitFeatures;

	bool isChecked = settings.value(QStringLiteral("UI/digitizeWithCurve")).toInt() && enable;
	ui.mActionDigitizeWithCurve->setChecked(isChecked);
	ui.mActionDigitizeWithCurve->setEnabled(enable);

	mMapTools.mAddFeature->setCircularDigitizingEnabled(isChecked);
	static_cast<QgsMapToolCapture *>(mMapTools.mSplitFeatures)->setCircularDigitizingEnabled(isChecked);
}

void DGisApp::saveActiveLayerEdits()
{
	saveEdits(activeLayer(), true, true);
}

//void DGisApp::dataSourceManager(const QString &pageName)
//{
//	if (!mDataSourceManagerDialog)
//	{
//		//mDataSourceManagerDialog = new QgsDataSourceManagerDialog(mBrowserModel, this, mapCanvas());
//		//connect(this, &DGisApp::connectionsChanged, mDataSourceManagerDialog, &QgsDataSourceManagerDialog::refresh);
//		//connect(mDataSourceManagerDialog, &QgsDataSourceManagerDialog::connectionsChanged, this, &DGisApp::connectionsChanged);
//		//connect(mDataSourceManagerDialog, SIGNAL(addRasterLayer(QString const &, QString const &, QString const &)),
//		//	this, SLOT(addRasterLayer(QString const &, QString const &, QString const &)));
//		//connect(mDataSourceManagerDialog, SIGNAL(addVectorLayer(QString const &, QString const &, QString const &)),
//		//	this, SLOT(addVectorLayer(QString const &, QString const &, QString const &)));
//		//connect(mDataSourceManagerDialog, SIGNAL(addVectorLayers(QStringList const &, QString const &, QString const &)),
//		//	this, SLOT(addVectorLayers(QStringList const &, QString const &, QString const &)));
//		//connect(mDataSourceManagerDialog, &QgsDataSourceManagerDialog::addMeshLayer, this, &DGisApp::addMeshLayer);
//		//connect(mDataSourceManagerDialog, &QgsDataSourceManagerDialog::addVectorTileLayer, this, &DGisApp::addVectorTileLayer);
//		//connect(mDataSourceManagerDialog, &QgsDataSourceManagerDialog::showStatusMessage, this, &DGisApp::showStatusMessage);
//		//connect(mDataSourceManagerDialog, &QgsDataSourceManagerDialog::addDatabaseLayers, this, &DGisApp::addDatabaseLayers);
//		//connect(mDataSourceManagerDialog, &QgsDataSourceManagerDialog::replaceSelectedVectorLayer, this, &DGisApp::replaceSelectedVectorLayer);
//		//connect(mDataSourceManagerDialog, static_cast<void (QgsDataSourceManagerDialog::*)()>(&QgsDataSourceManagerDialog::addRasterLayer), this, static_cast<void (DGisApp::*)()>(&DGisApp::addRasterLayer));
//		//connect(mDataSourceManagerDialog, &QgsDataSourceManagerDialog::handleDropUriList, this, &DGisApp::handleDropUriList);
//		//connect(this, &DGisApp::newProject, mDataSourceManagerDialog, &QgsDataSourceManagerDialog::updateProjectHome);
//		//connect(mDataSourceManagerDialog, &QgsDataSourceManagerDialog::openFile, this, &DGisApp::openFile);
//	}
//	else
//	{
//		mDataSourceManagerDialog->reset();
//	}
//	// Try to open the dialog on a particular page
//	if (!pageName.isEmpty())
//	{
//		mDataSourceManagerDialog->openPage(pageName);
//	}
//	if (QgsSettings().value(QStringLiteral("/qgis/dataSourceManagerNonModal"), true).toBool())
//	{
//		mDataSourceManagerDialog->show();
//	}
//	else
//	{
//		mDataSourceManagerDialog->exec();
//	}
//}

//---------------------------------------------Python functions Start---------------------------------------------------
DPythonUtils *DGisApp::getPythonRunner()
{
	return mPythonRunner;
}

bool DGisApp::runString(QString &content, QString &msgOnError, bool single)
{
	bool res = false;
	if (!content.isEmpty()&&mPythonRunner!=nullptr)
		res = mPythonRunner->runString(content);
	return res;
}

void DGisApp::registerCustomDropHandler(QgsCustomDropHandler *handler)
{
  if (!mCustomDropHandlers.contains(handler))
    mCustomDropHandlers << handler;

  const auto canvases = mapCanvases();
  for (QgsMapCanvas *canvas : canvases)
  {
    canvas->setCustomDropHandlers(mCustomDropHandlers);
  }
}

//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
QgsRasterLayer *DGisApp::addRasterLayer(
	QString const &rasterFile, QString const &baseName, bool guiWarning)
{
	return addRasterLayerPrivate(rasterFile, baseName, QString(), guiWarning, true);
}

QgsRasterLayer *DGisApp::addRasterLayer(
	QString const &uri, QString const &baseName, QString const &providerKey)
{
	return addRasterLayerPrivate(uri, baseName, providerKey, true, true);
}

bool DGisApp::addRasterLayer(QgsRasterLayer *rasterLayer)
{
	Q_CHECK_PTR(rasterLayer);

	if (!rasterLayer)
	{
		// XXX insert meaningful whine to the user here; although be
		// XXX mindful that a null layer may mean exhausted memory resources
		return false;
	}

	if (!rasterLayer->isValid())
	{
		delete rasterLayer;
		return false;
	}

	// register this layer with the central layers registry
	QList<QgsMapLayer *> myList;
	myList << rasterLayer;
	QgsProject::instance()->addMapLayers(myList);

	askUserForDatumTransform(rasterLayer->crs(), QgsProject::instance()->crs(), rasterLayer);

	return true;
}

// Open a raster layer - this is the generic function which takes all parameters
// this method is a blend of addRasterLayer() functions (with and without provider)
// and addRasterLayers()
QgsRasterLayer *DGisApp::addRasterLayerPrivate(
	const QString &uri, const QString &name, const QString &providerKey,
	bool guiWarning, bool guiUpdate)
{
	std::unique_ptr<QgsCanvasRefreshBlocker> refreshBlocker;
	if (guiUpdate)
	{
		// let the user know we're going to possibly be taking a while
		// QApplication::setOverrideCursor( Qt::WaitCursor );
		refreshBlocker = qgis::make_unique<QgsCanvasRefreshBlocker>();
	}

	QString shortName = name;
	QRegularExpression reRasterFile(QStringLiteral("^/vsi(.+/)*([^ ]+)( .+)?$"), QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch matchRasterFile = reRasterFile.match(name);

	if (matchRasterFile.hasMatch())
	{
		if (!matchRasterFile.captured(2).isEmpty())
		{
			shortName = matchRasterFile.captured(2);
		}
	}

	QgsSettings settings;
	QString baseName = settings.value(QStringLiteral("qgis/formatLayerName"), false).toBool() ? QgsMapLayer::formatLayerName(shortName) : shortName;

	QgsDebugMsgLevel("Creating new raster layer using " + uri + " with baseName of " + baseName, 2);

	QgsRasterLayer *layer = nullptr;
	// XXX ya know QgsRasterLayer can snip out the basename on its own;
	// XXX why do we have to pass it in for it?
	// ET : we may not be getting "normal" files here, so we still need the baseName argument
	if (!providerKey.isEmpty() && uri.endsWith(QLatin1String(".adf"), Qt::CaseInsensitive))
	{
		QFileInfo fileInfo(uri);
		QString dirName = fileInfo.path();
		layer = new QgsRasterLayer(dirName, QFileInfo(dirName).completeBaseName(), QStringLiteral("gdal"));
	}
	else if (providerKey.isEmpty())
		layer = new QgsRasterLayer(uri, baseName); // fi.completeBaseName());
	else
		layer = new QgsRasterLayer(uri, baseName, providerKey);

	QgsDebugMsgLevel(QStringLiteral("Constructed new layer"), 2);

	QgsError error;
	QString title;
	bool ok = false;

	error = layer->error();
	title = tr("Invalid Layer");

	if (!layer->isValid())
	{
		error = layer->error();
		title = tr("Invalid Layer");

		if (shouldAskUserForGDALSublayers(layer))
		{
			QList<QgsMapLayer *> subLayers = askUserForGDALSublayers(layer);
			ok = true;

			// The first layer loaded is not useful in that case. The user can select it in
			// the list if he wants to load it.
			delete layer;
			for (QgsMapLayer *l : qgis::as_const(subLayers))
				askUserForDatumTransform(l->crs(), QgsProject::instance()->crs(), l);
			layer = !subLayers.isEmpty() ? qobject_cast<QgsRasterLayer *>(subLayers.at(0)) : nullptr;
		}
	}
	else
	{
		ok = addRasterLayer(layer);
		if (!ok)
		{
			error.append(QGS_ERROR_MESSAGE(tr("Error adding valid layer to map canvas"),
				tr("Raster layer")));
			title = tr("Error");
		}
	}

	if (!ok)
	{
		// don't show the gui warning if we are loading from command line
		if (guiWarning)
		{
			messageBar()->pushMessage(title, error.message(QgsErrorMessage::Text),
				Qgis::Critical, messageTimeout());
		}

		if (layer)
		{
			delete layer;
			layer = nullptr;
		}
	}

	if (layer)
		layer->loadDefaultMetadata(ok);

	return layer;
}




// should the GDAL sublayers dialog should be presented to the user?
bool DGisApp::shouldAskUserForGDALSublayers(QgsRasterLayer *layer)
{
	// return false if layer is empty or raster has no sublayers
	if (!layer || layer->providerType() != QLatin1String("gdal") || layer->subLayers().empty())
		return false;

	QgsSettings settings;
	QgsSublayersDialog::PromptMode promptLayers = settings.enumValue(QStringLiteral("qgis/promptForSublayers"), QgsSublayersDialog::PromptAlways);

	return promptLayers == QgsSublayersDialog::PromptAlways ||
		promptLayers == QgsSublayersDialog::PromptLoadAll ||
		(promptLayers == QgsSublayersDialog::PromptIfNeeded && layer->bandCount() == 0);
}

// present a dialog to choose GDAL raster sublayers
QList<QgsMapLayer *> DGisApp::askUserForGDALSublayers(QgsRasterLayer *layer)
{
	QList<QgsMapLayer *> result;
	if (!layer)
		return result;

	QStringList sublayers = layer->subLayers();
	QgsDebugMsgLevel(QStringLiteral("raster has %1 sublayers").arg(layer->subLayers().size()), 2);

	if (sublayers.empty())
		return result;

	// if promptLayers=Load all, load all sublayers without prompting
	QgsSettings settings;
	if (settings.enumValue(QStringLiteral("qgis/promptForSublayers"), QgsSublayersDialog::PromptAlways) == QgsSublayersDialog::PromptLoadAll)
	{
		result.append(loadGDALSublayers(layer->source(), sublayers));
		return result;
	}
	
	// We initialize a selection dialog and display it.
	QgsSublayersDialog chooseSublayersDialog(QgsSublayersDialog::Gdal, QStringLiteral("gdal"), this, Qt::WindowFlags(), layer->dataProvider()->dataSourceUri());
	chooseSublayersDialog.setShowAddToGroupCheckbox(true);

	QgsSublayersDialog::LayerDefinitionList layers;
	QStringList names;
	names.reserve(sublayers.size());
	layers.reserve(sublayers.size());
	for (int i = 0; i < sublayers.size(); i++)
	{
		// simplify raster sublayer name - should add a function in gdal provider for this?
		// code is copied from QgsGdalLayerItem::createChildren
		QString name = sublayers[i];
		QString path = layer->source();
		// if netcdf/hdf use all text after filename
		// for hdf4 it would be best to get description, because the subdataset_index is not very practical
		if (name.startsWith(QLatin1String("netcdf"), Qt::CaseInsensitive) ||
			name.startsWith(QLatin1String("hdf"), Qt::CaseInsensitive))
		{
			name = name.mid(name.indexOf(path) + path.length() + 1);
		}
		else if (name.startsWith(QLatin1String("GPKG"), Qt::CaseInsensitive))
		{
			const auto parts{ name.split(':') };
			if (parts.count() >= 3)
			{
				name = parts.at(parts.count() - 1);
			}
		}
		else
		{
			// remove driver name and file name
			name.remove(name.split(QgsDataProvider::sublayerSeparator())[0]);
			name.remove(path);
		}
		// remove any : or " left over
		if (name.startsWith(':'))
			name.remove(0, 1);

		if (name.startsWith('\"'))
			name.remove(0, 1);

		if (name.endsWith(':'))
			name.chop(1);

		if (name.endsWith('\"'))
			name.chop(1);

		names << name;

		QgsSublayersDialog::LayerDefinition def;
		def.layerId = i;
		def.layerName = name;
		layers << def;
	}

	chooseSublayersDialog.populateLayerTable(layers);

	if (chooseSublayersDialog.exec())
	{
		// create more informative layer names, containing filename as well as sublayer name
		QRegExp rx("\"(.*)\"");
		QString uri, name;

		QgsLayerTreeGroup *group = nullptr;
		bool addToGroup = settings.value(QStringLiteral("/qgis/openSublayersInGroup"), true).toBool();
		bool newLayersVisible = settings.value(QStringLiteral("/qgis/new_layers_visible"), true).toBool();
		if (addToGroup)
		{
			group = QgsProject::instance()->layerTreeRoot()->insertGroup(0, layer->name());
		}

		const auto constSelection = chooseSublayersDialog.selection();
		for (const QgsSublayersDialog::LayerDefinition &def : constSelection)
		{
			int i = def.layerId;
			if (rx.indexIn(sublayers[i]) != -1)
			{
				uri = rx.cap(1);
				name = sublayers[i];
				name.replace(uri, QFileInfo(uri).completeBaseName());
			}
			else
			{
				name = names[i];
			}

			QgsRasterLayer *rlayer = new QgsRasterLayer(sublayers[i], name);
			if (rlayer && rlayer->isValid())
			{
				if (addToGroup)
				{
					QgsProject::instance()->addMapLayer(rlayer, false);
					group->addLayer(rlayer);
				}
				else
				{
					addRasterLayer(rlayer);
				}
				result << rlayer;
			}
		}

		// Respect if user don't want the new group of layers visible.
		if (addToGroup && !newLayersVisible)
			group->setItemVisibilityCheckedRecursive(newLayersVisible);
	}
	return result;
}

// This method will load with GDAL the layers in parameter.
// It is normally triggered by the sublayer selection dialog.
QList<QgsMapLayer *> DGisApp::loadGDALSublayers(const QString &uri, const QStringList &list)
{
	QList<QgsMapLayer *> result;
	QString path, name;
	QgsRasterLayer *subLayer = nullptr;
	QgsSettings settings;
	QgsLayerTreeGroup *group = nullptr;
	bool addToGroup = settings.value(QStringLiteral("/qgis/openSublayersInGroup"), true).toBool();
	if (addToGroup)
		group = QgsProject::instance()->layerTreeRoot()->insertGroup(0, QFileInfo(uri).completeBaseName());

	//add layers in reverse order so they appear in the right order in the layer dock
	for (int i = list.size() - 1; i >= 0; i--)
	{
		path = list[i];
		// shorten name by replacing complete path with filename
		name = path;
		name.replace(uri, QFileInfo(uri).completeBaseName());
		subLayer = new QgsRasterLayer(path, name);
		if (subLayer)
		{
			if (subLayer->isValid())
			{
				if (addToGroup)
				{
					QgsProject::instance()->addMapLayer(subLayer, false);
					group->addLayer(subLayer);
				}
				else
				{
					addRasterLayer(subLayer);
				}
				result << subLayer;
			}
			else
				delete subLayer;
		}
	}
	return result;
}

QgsVectorLayer *DGisApp::addVectorLayer(const QString &vectorLayerPath, const QString &name, const QString &providerKey)
{
	return addVectorLayerPrivate(vectorLayerPath, name, providerKey);
}

QgsVectorLayer *DGisApp::addVectorLayerPrivate(const QString &vectorLayerPath, const QString &name, const QString &providerKey, const bool guiWarning)
{
	QgsSettings settings;

	QgsCanvasRefreshBlocker refreshBlocker;

	QString baseName = settings.value(QStringLiteral("qgis/formatLayerName"), false).toBool() ? QgsMapLayer::formatLayerName(name) : name;

	/* Eliminate the need to instantiate the layer based on provider type.
	   The caller is responsible for cobbling together the needed information to
	   open the layer
	   */
	QgsDebugMsgLevel("Creating new vector layer using " + vectorLayerPath + " with baseName of " + baseName + " and providerKey of " + providerKey, 2);

	// if the layer needs authentication, ensure the master password is set
	bool authok = true;

	// create the layer
	QgsVectorLayer::LayerOptions options{ QgsProject::instance()->transformContext() };
	// Default style is loaded later in this method
	options.loadDefaultStyle = false;
	QgsVectorLayer *layer = new QgsVectorLayer(vectorLayerPath, baseName, providerKey, options);

	if (authok && layer->isValid())
	{
		const bool layerIsSpecified = vectorLayerPath.contains(QLatin1String("layerid=")) ||
			vectorLayerPath.contains(QLatin1String("layername="));

		const QStringList sublayers = layer->dataProvider()->subLayers();
		if (!layerIsSpecified)
		{
			QgsDebugMsgLevel(QStringLiteral("got valid layer with %1 sublayers").arg(sublayers.count()), 2);
		}

		// If the newly created layer has more than 1 layer of data available, we show the
		// sublayers selection dialog so the user can select the sublayers to actually load.
		if (!layerIsSpecified && sublayers.count() > 1)
		{
			QList<QgsMapLayer *> addedLayers = askUserForOGRSublayers(layer, sublayers);
			// layer is no longer valid and has been nullified

			layer = addedLayers.isEmpty() ? nullptr : qobject_cast<QgsVectorLayer *>(addedLayers.at(0));
			for (QgsMapLayer *l : addedLayers)
				askUserForDatumTransform(l->crs(), QgsProject::instance()->crs(), l);
		}
		else
		{
			// Register this layer with the layers registry
			QList<QgsMapLayer *> myList;

			//set friendly name for datasources with only one layer
			if (!sublayers.isEmpty())
			{
				setupVectorLayer(vectorLayerPath, sublayers, layer,
					providerKey, options);
			}

			myList << layer;
			QgsProject::instance()->addMapLayers(myList);

			askUserForDatumTransform(layer->crs(), QgsProject::instance()->crs(), layer);

			bool ok;
			layer->loadDefaultStyle(ok);
			layer->loadDefaultMetadata(ok);
		}
	}
	else
	{
		if (guiWarning)
		{
			QString message = layer->dataProvider() ? layer->dataProvider()->error().message(QgsErrorMessage::Text) : tr("Invalid provider");
			QString msg = tr("The layer %1 is not a valid layer and can not be added to the map. Reason: %2").arg(vectorLayerPath, message);
			messageBar()->pushMessage(tr("Layer is not valid"), msg, Qgis::Critical, messageTimeout());
		}

		delete layer;
		return nullptr;
	}

	// Let render() do its own cursor management
	//  QApplication::restoreOverrideCursor();

	return layer;
}

// ------------------------------------------------layertree menu start----------------------------------------------
void DGisApp::setProjectCrsFromLayer()
{
	if (!(mLayerTreeView && mLayerTreeView->currentLayer()))
	{
		return;
	}

	QgsCoordinateReferenceSystem crs = mLayerTreeView->currentLayer()->crs();
	QgsCanvasRefreshBlocker refreshBlocker;
	QgsProject::instance()->setCrs(crs);
}

void DGisApp::setLayerCrs()
{
	if (!(mLayerTreeView && mLayerTreeView->currentLayer()))
	{
		return;
	}

	QgsProjectionSelectionDialog mySelector(this);
	mySelector.setCrs(mLayerTreeView->currentLayer()->crs());

	if (!mLayerTreeView->currentLayer()->crs().isValid())
		mySelector.showNoCrsForLayerMessage();

	if (!mySelector.exec())
	{
		QApplication::restoreOverrideCursor();
		return;
	}

	QgsCoordinateReferenceSystem crs = mySelector.crs();

	const auto constSelectedNodes = mLayerTreeView->selectedNodes();
	for (QgsLayerTreeNode *node : constSelectedNodes)
	{
		if (QgsLayerTree::isGroup(node))
		{
			const auto constFindLayers = QgsLayerTree::toGroup(node)->findLayers();
			for (QgsLayerTreeLayer *child : constFindLayers)
			{
				if (child->layer())
				{
					askUserForDatumTransform(crs, QgsProject::instance()->crs(), child->layer());
					child->layer()->setCrs(crs);
					child->layer()->triggerRepaint();
				}
			}
		}
		else if (QgsLayerTree::isLayer(node))
		{
			QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer(node);
			if (nodeLayer->layer())
			{
				askUserForDatumTransform(crs, QgsProject::instance()->crs(), nodeLayer->layer());
				nodeLayer->layer()->setCrs(crs);
				nodeLayer->layer()->triggerRepaint();
			}
		}
	}
	refreshMapCanvas();
}

QString DGisApp::saveAsFile(QgsMapLayer *layer, const bool onlySelected, const bool defaultToAddToMap)
{
	if (!layer)
		layer = activeLayer();

	if (!layer)
		return QString();

	QgsMapLayerType layerType = layer->type();
	switch (layerType)
	{
	case QgsMapLayerType::RasterLayer:
		return saveAsRasterFile(qobject_cast<QgsRasterLayer *>(layer), defaultToAddToMap);

	case QgsMapLayerType::VectorLayer:
		return saveAsVectorFileGeneral(qobject_cast<QgsVectorLayer *>(layer), true, onlySelected, defaultToAddToMap);

	case QgsMapLayerType::MeshLayer:
	case QgsMapLayerType::VectorTileLayer:
	case QgsMapLayerType::PluginLayer:
	case QgsMapLayerType::AnnotationLayer:
		return QString();
	}
	return QString();
}

//create a raster layer object and delegate to addRasterLayer(QgsRasterLayer *)
bool DGisApp::addRasterLayers(QStringList const &fileNameQStringList, bool guiWarning)
{
	if (fileNameQStringList.empty())
	{
		// no files selected so bail out, but
		// allow mMapCanvas to handle events
		// first
		return false;
	}

	QgsCanvasRefreshBlocker refreshBlocker;

	// this is messy since some files in the list may be rasters and others may
	// be ogr layers. We'll set returnValue to false if one or more layers fail
	// to load.
	bool returnValue = true;
	for (QStringList::ConstIterator myIterator = fileNameQStringList.begin();
		myIterator != fileNameQStringList.end();
		++myIterator)
	{
		QString errMsg;
		bool ok = false;

		// if needed prompt for zipitem layers
		QString vsiPrefix = QgsZipItem::vsiPrefix(*myIterator);
		if (!myIterator->startsWith(QLatin1String("/vsi"), Qt::CaseInsensitive) &&
			(vsiPrefix == QLatin1String("/vsizip/") || vsiPrefix == QLatin1String("/vsitar/")))
		{
			if (askUserForZipItemLayers(*myIterator))
				continue;
		}

		if (QgsRasterLayer::isValidRasterFileName(*myIterator, errMsg))
		{
			QFileInfo myFileInfo(*myIterator);

			// set the layer name to the file base name...
			QString layerName = myFileInfo.completeBaseName();

			// ...unless provided explicitly
			const QVariantMap uriDetails = QgsProviderRegistry::instance()->decodeUri(QStringLiteral("gdal"), *myIterator);
			if (!uriDetails[QStringLiteral("layerName")].toString().isEmpty())
			{
				layerName = uriDetails[QStringLiteral("layerName")].toString();
			}

			// try to create the layer
			QgsRasterLayer *layer = addRasterLayerPrivate(*myIterator, layerName,
				QString(), guiWarning, true);
			if (layer && layer->isValid())
			{
				//only allow one copy of a ai grid file to be loaded at a
				//time to prevent the user selecting all adfs in 1 dir which
				//actually represent 1 coverate,

				if (myFileInfo.fileName().endsWith(QLatin1String(".adf"), Qt::CaseInsensitive))
				{
					break;
				}
			}
			// if layer is invalid addRasterLayerPrivate() will show the error

		} // valid raster filename
		else
		{
			ok = false;

			// Issue message box warning unless we are loading from cmd line since
			// non-rasters are passed to this function first and then successfully
			// loaded afterwards (see main.cpp)
			if (guiWarning)
			{
				QString msg = tr("%1 is not a supported raster data source").arg(*myIterator);
				if (!errMsg.isEmpty())
					msg += '\n' + errMsg;

				messageBar()->pushMessage(tr("Unsupported Data Source"), msg, Qgis::Critical, messageTimeout());
			}
		}
		if (!ok)
		{
			returnValue = false;
		}
	}

	// Let render() do its own cursor management
	//  QApplication::restoreOverrideCursor();

	return returnValue;
}


QString DGisApp::saveAsRasterFile(QgsRasterLayer *rasterLayer, const bool defaultAddToCanvas)
{
	if (!rasterLayer)
		rasterLayer = qobject_cast<QgsRasterLayer *>(activeLayer());

	if (!rasterLayer)
	{
		return QString();
	}

	QgsRasterLayerSaveAsDialog d(rasterLayer, rasterLayer->dataProvider(),
		mMapCanvas->extent(), rasterLayer->crs(),
		mMapCanvas->mapSettings().destinationCrs(),
		this);
	d.setAddToCanvas(defaultAddToCanvas);
	if (d.exec() == QDialog::Rejected)
		return QString();

	QgsSettings settings;
	settings.setValue(QStringLiteral("UI/lastRasterFileDir"), QFileInfo(d.outputFileName()).absolutePath());

	QgsRasterFileWriter fileWriter(d.outputFileName());
	if (d.tileMode())
	{
		fileWriter.setTiledMode(true);
		fileWriter.setMaxTileWidth(d.maximumTileSizeX());
		fileWriter.setMaxTileHeight(d.maximumTileSizeY());
	}
	else
	{
		fileWriter.setOutputFormat(d.outputFormat());
	}

	// TODO: show error dialogs
	// TODO: this code should go somewhere else, but probably not into QgsRasterFileWriter
	// clone pipe/provider is not really necessary, ready for threads
	std::unique_ptr<QgsRasterPipe> pipe(nullptr);

	if (d.mode() == QgsRasterLayerSaveAsDialog::RawDataMode)
	{
		QgsDebugMsgLevel(QStringLiteral("Writing raw data"), 2);
		pipe.reset(new QgsRasterPipe());
		if (!pipe->set(rasterLayer->dataProvider()->clone()))
		{
			QgsDebugMsg(QStringLiteral("Cannot set pipe provider"));
			return QString();
		}

		QgsRasterNuller *nuller = new QgsRasterNuller();
		for (int band = 1; band <= rasterLayer->dataProvider()->bandCount(); band++)
		{
			nuller->setNoData(band, d.noData());
		}
		if (!pipe->insert(1, nuller))
		{
			QgsDebugMsg(QStringLiteral("Cannot set pipe nuller"));
			return QString();
		}

		// add projector if necessary
		if (d.outputCrs() != rasterLayer->crs())
		{
			QgsRasterProjector *projector = new QgsRasterProjector;
			projector->setCrs(rasterLayer->crs(), d.outputCrs(), QgsProject::instance()->transformContext());
			if (!pipe->insert(2, projector))
			{
				QgsDebugMsg(QStringLiteral("Cannot set pipe projector"));
				return QString();
			}
		}
	}
	else // RenderedImageMode
	{
		// clone the whole pipe
		QgsDebugMsgLevel(QStringLiteral("Writing rendered image"), 2);
		pipe.reset(new QgsRasterPipe(*rasterLayer->pipe()));
		QgsRasterProjector *projector = pipe->projector();
		if (!projector)
		{
			QgsDebugMsg(QStringLiteral("Cannot get pipe projector"));
			return QString();
		}
		projector->setCrs(rasterLayer->crs(), d.outputCrs(), QgsProject::instance()->transformContext());
	}

	if (!pipe->last())
	{
		return QString();
	}
	fileWriter.setCreateOptions(d.createOptions());

	fileWriter.setBuildPyramidsFlag(d.buildPyramidsFlag());
	fileWriter.setPyramidsList(d.pyramidsList());
	fileWriter.setPyramidsResampling(d.pyramidsResamplingMethod());
	fileWriter.setPyramidsFormat(d.pyramidsFormat());
	fileWriter.setPyramidsConfigOptions(d.pyramidsConfigOptions());

	bool tileMode = d.tileMode();
	bool addToCanvas = d.addToCanvas();
	QPointer<QgsRasterLayer> rlWeakPointer(rasterLayer);
	QString outputLayerName = d.outputLayerName();
	QString outputFormat = d.outputFormat();

	QgsRasterFileWriterTask *writerTask = new QgsRasterFileWriterTask(fileWriter, pipe.release(), d.nColumns(), d.nRows(),
		d.outputRectangle(), d.outputCrs(), QgsProject::instance()->transformContext());

	// when writer is successful:

	connect(writerTask, &QgsRasterFileWriterTask::writeComplete, this,
		[this, tileMode, addToCanvas, rlWeakPointer, outputLayerName, outputFormat](const QString &newFilename) {
		QString fileName = newFilename;
		if (tileMode)
		{
			QFileInfo outputInfo(fileName);
			fileName = QStringLiteral("%1/%2.vrt").arg(fileName, outputInfo.fileName());
		}

		if (addToCanvas)
		{
			if (outputFormat == QLatin1String("GPKG") && !outputLayerName.isEmpty())
			{
				addRasterLayers(QStringList(QStringLiteral("GPKG:%1:%2").arg(fileName, outputLayerName)));
			}
			else
			{
				addRasterLayers(QStringList(fileName));
			}
		}
		if (rlWeakPointer)
			emit layerSavedAs(rlWeakPointer, fileName);

		messageBar()->pushMessage(tr("Layer Exported"),
			tr("Successfully saved raster layer to <a href=\"%1\">%2</a>").arg(QUrl::fromLocalFile(newFilename).toString(), QDir::toNativeSeparators(newFilename)),
			Qgis::Success, messageTimeout());
	});

	// when an error occurs:
	connect(writerTask, qgis::overload<int, const QString &>::of(&QgsRasterFileWriterTask::errorOccurred), this, [=](int error, const QString &errorMessage) {
		if (error != QgsRasterFileWriter::WriteCanceled)
		{
			QString errorCodeStr;
			if (error == QgsRasterFileWriter::SourceProviderError)
				errorCodeStr = tr("source provider");
			else if (error == QgsRasterFileWriter::DestProviderError)
				errorCodeStr = tr("destination provider");
			else if (error == QgsRasterFileWriter::CreateDatasourceError)
				errorCodeStr = tr("data source creation");
			else if (error == QgsRasterFileWriter::WriteError)
				errorCodeStr = tr("write error");
			QString fullErrorMsg(tr("Cannot write raster. Error code: %1").arg(errorCodeStr));
			if (!errorMessage.isEmpty())
				fullErrorMsg += "\n" + errorMessage;
			QMessageBox::warning(this, tr("Save Raster"),
				fullErrorMsg,
				QMessageBox::Ok);
		}
	});

	QgsApplication::taskManager()->addTask(writerTask);
	return d.outputFileName();
}
// ------------------------------------------------layertree menu end----------------------------------------------

void DGisApp::projectCrsChanged()
{
	QgsDebugMsgLevel(QStringLiteral("QgisApp::setupConnections -1- : QgsProject::instance()->crs().description[%1]ellipsoid[%2]").arg(QgsProject::instance()->crs().description(), QgsProject::instance()->crs().ellipsoidAcronym()), 3);
	mMapCanvas->setDestinationCrs(QgsProject::instance()->crs());

	// handle datum transforms
	QList<QgsCoordinateReferenceSystem> alreadyAsked;
	QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
	for (QMap<QString, QgsMapLayer *>::const_iterator it = layers.constBegin(); it != layers.constEnd(); ++it)
	{
		if (!alreadyAsked.contains(it.value()->crs()))
		{
			alreadyAsked.append(it.value()->crs());
			askUserForDatumTransform(it.value()->crs(),
				QgsProject::instance()->crs(), it.value());
		}
	}
}

void DGisApp::layersWereAdded(const QList<QgsMapLayer *> &layers)
{
	const auto constLayers = layers;
	for (QgsMapLayer *layer : constLayers)
	{
		QgsDataProvider *provider = nullptr;

		QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>(layer);
		if (vlayer)
		{
			QgsVectorDataProvider *vProvider = vlayer->dataProvider();;
			provider = vProvider;
		}

		QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>(layer);
		if (rlayer)
		{
			provider = rlayer->dataProvider();
		}

		if (provider)
		{
			connect(provider, &QgsDataProvider::dataChanged, layer, [layer] { layer->triggerRepaint(); });
			connect(provider, &QgsDataProvider::dataChanged, this, [this] { refreshMapCanvas(); });
		}
	}
}