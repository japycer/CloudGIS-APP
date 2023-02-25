#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DGisApp.h"
#include"qgslayertreeregistrybridge.h"
#include"qgsvectorlayertools.h"
#include"qgsattributeeditorcontext.h"
#include"qgsvectorlayersaveasdialog.h"
#include"qgsattributetablefiltermodel.h"
#include "DEnums.h"

class DTitleBar;
class QgsMapCanvas;
class QgsLayerTreeView;
class QgsDockWidget;
class QgsLegendFilterButton;
class QgsLayerTreeMapCanvasBridge;
class QgsLayerTreeRegistryBridge;
class QgsCustomLayerOrderWidget;
class QgsMapLayer;
class DInterface;
class QgsLayerStylingWidget;
class QgsMessageBar;
class QgsMapLayerConfigWidgetFactory;
class QgsMapToolSelect;
class QgsMapTool;
class QgsMapToolIdentifyAction;
class QgsMapToolAddFeature;
class QgsUserInputWidget;
class QgsVectorLayerTools;
class QgsAttributeEditorContext;
class QgsCoordinateReferenceSystem;
class QgsClipboard;
class QgsFeatureStore;
class QStackedWidget;
class QgsCustomProjectOpenHandler;
class QDateTime;
class DPythonUtils;
class QTextEdit;
class QgsMapToolAddFeature;
class QgsAdvancedDigitizingDockWidget;
class DPyInterface;
//class QgsDataSourceManagerDialog;
//class QgsBrowserGuiModel;
//class QgsBrowserDockWidget;
class QgsRasterLayer;
class QgsCustomDropHandler;
class DMapToolAddFeature;
class QSettings;
class DDatabaseInterface;
class QgsStatusBar;
class QProgressBar;
class QgsStatusBarCoordinatesWidget;
class QgsStatusBarScaleWidget;
class QElapsedTimer;
class QTimer;
class QMetaObject;
class DCustomWindow;
class DMapToolTemporyDrawer;
class DTemporyDrawerManager;
class DClassifyManager;
class DMapToolDrawSample;
class DGeeObjectManager;
class DToolBoxManager;
class DToolBox;
class DWelComePage;
//class QgsGeometryValidationService;
//class QgsGeometryValidationModel;


class DGisApp : public QMainWindow
{
	Q_OBJECT

		//公有构造函数、析构函数
public:
	DGisApp(DEnums::User* usr, QWidget *parent = Q_NULLPTR);
	~DGisApp();
	//公有方法
public:
	//Returns a list of all map canvases open in the app.
	QList< QgsMapCanvas * > mapCanvases();
	//Freezes all map canvases (or thaws them if the \a frozen argument is FALSE).
	void freezeCanvases(bool frozen = true);
	QSize iconSize(bool dockedToolbar = false) const;
	QgsLayerTreeRegistryBridge::InsertionPoint layerTreeInsertionPoint() const;
	//! returns pointer to map legend
	QgsLayerTreeView *layerTreeView();

	QSettings *getSetting();

   //Opens a QGIS project file .returns FALSE if unable to open the project
	bool addProject(const QString &projectFile);
	//! Returns the active map layer.
	QgsMapLayer *activeLayer();
	//! Adds a widget to the user input tool bar.
	void addUserInputWidget(QWidget *widget);
	bool DGisApp::askUserForDatumTransform(const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, const QgsMapLayer *layer);

	QAction *actionHideAllLayers() { return ui.mActionHideAllLayers; }
	QAction *actionShowAllLayers() { return ui.mActionShowAllLayers; }
	QAction *actionHideSelectedLayers() { return ui.mActionHideSelectedLayers; }
	QAction *actionToggleSelectedLayers() { return ui.mActionToggleSelectedLayers; }
	QAction *actionToggleSelectedLayersIndependently() { return ui.mActionToggleSelectedLayersIndependently; }
	QAction *actionHideDeselectedLayers() { return ui.mActionHideDeselectedLayers; }
	QAction *actionShowSelectedLayers() { return ui.mActionShowSelectedLayers; }

	QgsMessageBar *messageBar();
	QgsMapCanvas *mapCanvas();
	QgsClipboard *clipboard();
	int messageTimeout();

	//! Returns the CAD dock widget
	QgsAdvancedDigitizingDockWidget *cadDockWidget();
	bool runString(QString& content, QString &msgOnError = QString(), bool single = true);

public slots:
	static DGisApp *instance() { return sInstance; }
	void slotAddRasterLayer(const QString &fileName);
	void slotAddVectorLayer(const QString &fileName, const QString &encode, const QString &providerKey);
	void slotAddWmsLayer();
	QgsVectorLayer *addVectorLayer(const QString &vectorLayerPath, const QString &baseName, const QString &providerKey);
	QList<QgsMapLayer *> loadGDALSublayers(const QString &uri, const QStringList &list);
	QgsRasterLayer *addRasterLayer(const QString &rasterFile, const QString &baseName, bool guiWarning = true);
	QgsRasterLayer *addRasterLayer(QString const &uri, QString const &baseName, QString const &providerKey);
	bool DGisApp::addRasterLayer(QgsRasterLayer *rasterLayer);
	//! Open a raster layer - this is the generic function which takes all parameters
	QgsRasterLayer *addRasterLayerPrivate(const QString &uri, const QString &baseName,
		const QString &providerKey, bool guiWarning,
		bool guiUpdate);
	void layerTreeViewDoubleClicked(const QModelIndex & index);
	void updateLabelToolButtons();
	void runScript(const QString &filePath);
	/**
  * \brief dataSourceManager Open the DataSourceManager dialog
  * \param pageName the page name, usually the provider name or "browser" (for the browser panel)
  *        or "ogr" (vector layers) or "raster" (raster layers)
  */
	//void dataSourceManager(const QString &pageName = QString());
	/**
   * Access the vector layer tools. This will be an instance of {\see QgsGuiVectorLayerTools}
   * by default.
   * \returns  The vector layer tools
   */
   /**
	* Creates a default attribute editor context using the main map canvas and the main edit tools and message bar
	* \since QGIS 3.12
	*/
	QgsAttributeEditorContext createAttributeEditorContext();
	QgsVectorLayerTools *vectorLayerTools();
	/**
   * Save edits of a layer
   * \param leaveEditable leave the layer in editing mode when done
   * \param triggerRepaint send layer signal to repaint canvas when done
   */
	void saveEdits(QgsMapLayer *layer, bool leaveEditable = true, bool triggerRepaint = true);
	//! Save current edits for selected layer(s) and start new transaction(s)
	void saveEdits();
	//! Remove a layer from the map and legend
	void removeLayer();
	//! show the attribute table for the currently selected layer
	void attributeTable(QgsAttributeTableFilterModel::FilterMode filter = QgsAttributeTableFilterModel::ShowAll);

	void addTabifiedDockWidget(Qt::DockWidgetArea area, QDockWidget *dockWidget, const QStringList &tabifyWith, bool raiseTab);
	//! Deletes the selected attributes for the currently selected vector layer
	void deleteSelected(QgsMapLayer *layer = nullptr, QWidget *parent = nullptr, bool checkFeaturesVisible = false);
	//有关剪贴板复制粘贴
	void cutSelectionToClipboard(QgsMapLayer *layerContainingSelection = nullptr);
	void copySelectionToClipboard(QgsMapLayer *layerContainingSelection = nullptr);
	void pasteFromClipboard(QgsMapLayer *destinationLayer = nullptr);
	void DGisApp::pasteFeatures(QgsVectorLayer *pasteVectorLayer, int invalidGeometriesCount, int nTotalFeatures, QgsFeatureList &features);
	//! copies features on the clipboard to a new vector layer
	void pasteAsNewVector();
	//! copies features on the clipboard to a new memory vector layer
	QgsVectorLayer *pasteAsNewMemoryVector(const QString &layerName = QString());
	/**
   * \brief overloaded version of the private addLayer method that takes a list of
   * file names instead of prompting user with a dialog.
   * \param enc encoding type for the layer
   * \param dataSourceType type of ogr datasource
   * \returns TRUE if successfully added layer
   */
	bool addVectorLayers(const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType);
	void copyFeatures(QgsFeatureStore &featureStore);
	//! Save project. Returns true if the user selected a file to save to, false if not.
	bool fileSave();

	//---------------------------edit function---------------------------
	//! starts/stops editing mode of the current layer
	void toggleEditing();

	//! starts/stops editing mode of a layer
	bool toggleEditing(QgsMapLayer *layer, bool allowCancel = true);

	//! Save edits for active vector layer and start new transactions
	void saveActiveLayerEdits();
	QAction *actionToggleEditing() { return ui.mActionToggleEditing; }
	QAction *actionSaveActiveLayerEdits() { return ui.mActionSaveLayerEdits; }
	void setProjectCrsFromLayer();
	void setLayerCrs();
	bool addRasterLayers(QStringList const &fileNameQStringList, bool guiWarning = true);
	QString saveAsFile(QgsMapLayer *layer = nullptr, const bool onlySelected = false, const bool defaultToAddToMap = true);
	QString saveAsRasterFile(QgsRasterLayer *rasterLayer, const bool defaultAddToCanvas);
	//get inside pythonRunner object
	DPythonUtils *getPythonRunner();

	// test a wms link
	void testWmsLink(const QString &URL, const QString &layerName, bool *res);

	//公有成员
public:
	enum Interface
	{
		ToWelcome,
		ToMap,
		ToCodeEditor,
		ToDatabase
	};
	void changeInterfaceTo(DGisApp::Interface);

signals:
	void layerSavedAs(QgsMapLayer *l, const QString &path);
	void activeLayerChanged(QgsMapLayer *layer);
	void newProject();
	/**
   * Emitted when a project file is successfully read
   * \note This is useful for plug-ins that store properties with project files.  A
   * plug-in can connect to this signal.  When it is emitted, the plug-in
   * knows to then check the project properties for any relevant state.
	*/
	void projectRead();

	//私有成员函数
private:
	void initLayerTreeView();//初始化layertree
	void bindSignalsSlots();
	void createCanvasTools();
	void createStatusBar();

	void loadPythonSupport();
	void testPython();		//临时的测试函数
	QList<QgsMapLayer *> askUserForGDALSublayers(QgsRasterLayer *layer);
	bool shouldAskUserForGDALSublayers(QgsRasterLayer *layer);
	/**
   * Paste features from clipboard into a new memory layer.
   * If no features are in clipboard an empty layer is returned.
   * Returns a new memory layer or NULLPTR if the operation failed.
   */
	std::unique_ptr<QgsVectorLayer> pasteToNewMemoryVector();
	QString saveAsVectorFileGeneral(QgsVectorLayer *vlayer = nullptr, bool symbologyOption = true, bool onlySelected = false, bool defaultToAddToMap = true);

	QString saveAsVectorFileGeneral(QgsVectorLayer *vlayer, bool symbologyOption, bool onlySelected, bool defaultToAddToMap,
		const std::function<void(const QString &newFilename,
			bool addToCanvas,
			const QString &layerName,
			const QString &encoding,
			const QString &vectorFileName)> &onSuccess,
		const std::function<void(int error, const QString &errorMessage)> &onFailure,
		int dialogOptions = QgsVectorLayerSaveAsDialog::AllOptions,
		const QString &dialogTitle = QString());
	bool addVectorLayersPrivate(const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType, bool guiWarning = true);
	/**
   * This method will open a dialog so the user can select GDAL sublayers to load
   * \returns TRUE if any items were loaded
   */
	bool askUserForZipItemLayers(const QString &path);
	/**
   * This method will open a dialog so the user can select OGR sublayers to load,
   * and then returns a list of these layers. It will destroy the passed layer
   * in the process
   */
	QList<QgsMapLayer *> askUserForOGRSublayers(QgsVectorLayer *&layerTransferOwnership, const QStringList &subLayers);
	//! Checks for running tasks dependent on the open project

	//project functions
	bool checkTasksDependOnProject();
	/**
   * Check to see if the current project file is dirty and if so, prompt the user to save it.
   * \returns TRUE if saved or discarded, FALSE if canceled
   */
	bool saveDirty();
	void showProgress(int progress, int totalSteps);
	/**
	   * Checks for unsaved changes in open layers and prompts the user to save
	   * or discard these changes for each layer.
	   *
	   * Returns TRUE if there are no unsaved layer edits remaining, or the user
	   * opted to discard them all. Returns FALSE if the user opted to cancel
	   * on any layer.
	   */
	bool checkUnsavedLayerEdits();

	/**
	   * Checks whether memory layers (with features) exist in the project, and if so
	   * shows a warning to users that their contents will be lost on
	   * project unload.
	   *
	   * Returns TRUE if there are no memory layers present, or if the
	   * user opted to discard their contents. Returns FALSE if there
	   * are memory layers present and the user clicked 'Cancel' on
	   * the warning dialog.
	   */
	bool checkMemoryLayers();

	void commitError(QgsVectorLayer *vlayer);
	// 下面是加载数据界面所需要的函数
	void openAddDatasetManager(int currentIndex = 1);

private slots:
	//! refresh map canvas
	void refreshMapCanvas(bool redrawAllLayers = false);
	void onActiveLayerChanged(QgsMapLayer *layer);
	void updateNewLayerInsertionPoint();
	//! connected to layer tree registry bridge, selects first of the newly added map layers
	void autoSelectAddedLayer(QList<QgsMapLayer *> layers);
	void updateFilterLegend();
	/**
	* Activates or deactivates actions depending on the current maplayer type.
	* Is called from the legend when the current legend item has changed.
	*/
	void activateDeactivateLayerRelatedActions(QgsMapLayer *layer);
	void toggleFilterLegendByExpression(bool);

	//有关图层显示的槽函数
	//reimplements method from base (gui) class
	void hideAllLayers();
	//reimplements method from base (gui) class
	void showAllLayers();
	//reimplements method from base (gui) class
	void hideSelectedLayers();
	//! Toggles the visibility of the selected layers depending on the state of the first layer in selection (first clicked)
	void toggleSelectedLayers();
	//! Toggles the visibility of the selected layers independently
	void toggleSelectedLayersIndependently();
	//! Hides any layers which are not selected
	void hideDeselectedLayers();
	//reimplements method from base (gui) class
	void showSelectedLayers();

	//! shows the map styling dock
	void mapStyleDock(bool enabled);

	//! Sets the layer for the map style dock. Doesn't show the style dock
	void setMapStyleDockLayer(QgsMapLayer *layer);
	bool setActiveLayer(QgsMapLayer *layer);

	//! start "busy" progress bar
	void canvasRefreshStarted();
	//! stop "busy" progress bar
	void canvasRefreshFinished();

	//选择工具
	//! activates the rectangle selection tool
	void selectFeatures();
	void selectByPolygon();
	void selectByFreehand();
	//! deselect features from all layers
	void deselectAll();
	void invertSelection();
	void selectAll();
	void deselectActiveLayer();
	//常用地图工具
	void zoomOut();
	//! Sets map tool to Zoom in
	void zoomIn();
	//! Sets map tool to pan
	void pan();
	void panToSelected();
	void zoomToSelected();
	void zoomFull();
	void zoomToLayerExtent();
	void zoomToPrevious();
	void zoomToNext();
	//identify
	void identify();
	void measure();
	void measureArea();

	//! clear out any stuff from project
	void closeProject();
	void fileClose();
	bool fileNew();
	//! Create a new blank project (no template)
	bool fileNewBlank();
	bool fileNewFromTemplate(const QString &fileName);

	/**
	   * Close the current open project and show the welcome screen again.
	   */
	//void fileClose();

	//! As above but allows forcing without prompt and forcing blank project
	bool fileNew(bool promptToSaveFlag, bool forceBlank = false);
	//! What type of project to open after launch
	//void fileOpenAfterLaunch();
	//! After project read, set any auto-opened project as successful
	//void fileOpenedOKAfterLaunch();
	//void fileNewFromTemplateAction(QAction *qAction);
	void fileNewFromDefaultTemplate();
	//! Open a project
	void fileOpen();

	//-------------------------------------layer----------------------------------------
	//! Create a new empty vector layer
	void newVectorLayer();
	void newMemoryLayer();

	//! activates the add feature tool
	void addFeature();
	/**
   * Enables the action that toggles digitizing with curve
   * \since QGIS 3.16
   */
	void enableDigitizeWithCurveAction(bool enable);

	//! activates the split features tool
	void splitFeatures();
	void moveFeatureCopy();
	void moveFeature();
	void registerCustomDropHandler(QgsCustomDropHandler *handler);
	QgsVectorLayer *addVectorLayerPrivate(const QString &vectorLayerPath, const QString &baseName, const QString &providerKey, bool guiWarning = true);
	// QgsProject
	void projectCrsChanged();
	void layersWereAdded(const QList<QgsMapLayer *> &layers);

	//私有成员变量
private:
	Ui::DGisAppClass ui;
	static DGisApp *sInstance;
	DTitleBar *mTitleBar = nullptr;
	QSettings *mSettings = nullptr; //系统环境初始化的变量
	QgsMapCanvas *mMapCanvas = nullptr;
	QgsLayerTreeView *mLayerTreeView = nullptr;
	QStackedWidget *mCentralContainer = nullptr;
	QgsDockWidget *mLayerTreeDock = nullptr;
	QgsDockWidget *mMapStylingDock = nullptr;
	// DTemporyDrawerManager
	QgsDockWidget *mTemporyDrawerDock = nullptr;
	DTemporyDrawerManager * mTemporyDrawerManager = nullptr;
	// classfy manager
	QgsDockWidget *mClassifyManagerDock = nullptr;
	DClassifyManager * mClassifyManager = nullptr;
	QgsLayerStylingWidget *mMapStyleWidget = nullptr;
	QgsDockWidget *mLogDock = nullptr;

	QgsStatusBar *mStatusBar = nullptr; // 状态栏
	QProgressBar *mProgressBar = nullptr;
	QgsStatusBarCoordinatesWidget *mCoordsEdit = nullptr;
	QgsStatusBarScaleWidget *mScaleWidget = nullptr;
	QToolButton *mMessageLog = nullptr;
	// 计时器、有关地图刷新时的进度条
	QElapsedTimer mLastRenderTime;
	double mLastRenderTimeSeconds = 0;
	QTimer mRenderProgressBarTimer;
	QMetaObject::Connection mRenderProgressBarTimerConnection;


	QgsDockWidget *testDockW = nullptr;
	// record all tempory drawer geometry
	//QMap<QString, int> *mTemporyRecords = nullptr;
	//! Data Source Manager
	//QgsDataSourceManagerDialog *mDataSourceManagerDialog = nullptr;
	QDateTime mProjectLastModified;
	//maptree action
	QAction *mActionFilterLegend = nullptr;
	QAction * mActionStyleDock = nullptr;
	QgsLayerTreeMapCanvasBridge *mLayerTreeCanvasBridge = nullptr;
	//mLegendExpressionFilterButton = new QgsLegendFilterButton(this);
	QgsLegendFilterButton *mLegendExpressionFilterButton = nullptr;
	QgsCustomLayerOrderWidget *mMapLayerOrder = nullptr;
	QgsDockWidget *mLayerOrderDock = nullptr;
	QList<QgsMapLayer*> mapCanvasLayerSet; // 地图画布所用的图层集合

	//! A tool bar for user input
	QgsUserInputWidget *mUserInputDockWidget = nullptr;
	int mFreezeCount = 0;
	bool mReleased = false;
	QgsMapTool *mNonEditMapTool = nullptr;

	QgsClipboard *mInternalClipboard = nullptr;
	QgsMessageBar *mInfoBar = nullptr;
	QList<QgsMapLayerConfigWidgetFactory *> mMapLayerPanelFactories;

	QVector<QPointer<QgsCustomProjectOpenHandler>> mCustomProjectOpenHandlers;
	//! Returns the CAD dock widget
	QgsAdvancedDigitizingDockWidget *mAdvancedDigitizingDockWidget = nullptr;
	//选择工具私有变量
	//std::unique_ptr<QgsGeometryValidationService> mGeometryValidationService;
	//QgsGeometryValidationModel *mGeometryValidationModel = nullptr;
	//QgsBrowserGuiModel *mBrowserModel = nullptr;
	//QgsBrowserDockWidget *mBrowserWidget = nullptr;
	QgsVectorLayerTools *mVectorLayerTools = nullptr;
	QTextEdit *editIuput = nullptr;
	QTextEdit *editOut = nullptr;
	/**
   * String containing supporting raster file formats
	Suitable for a QFileDialog file filter.  Build in ctor.
	*/
	//QString mRasterFileFilter;
	//QString mVectorFileFilter;
	//QgsVectorLayer *mpStratchLayer;
	DPythonUtils * mPythonRunner;	//dxl
	DInterface* mDInterface = nullptr;
	DPyInterface  *mPyInterface = nullptr;
	DDatabaseInterface *mDbInterface = nullptr;
	//QDockWidget *mPyInterfaceDock = nullptr;
	DMapToolAddFeature *mpToolAddPolygon = nullptr;
	bool mSaveRollbackInProgress;
	QVector<QPointer<QgsCustomDropHandler>> mCustomDropHandlers;

	// custom....
	DCustomWindow *mToolContainer = nullptr;
	QgsRasterLayer *mPlanToLoadRasterLayer = nullptr;	// 用于加载wms

	DGeeObjectManager* mGeeObjectManager = nullptr;
	QDockWidget* mGeeObjectManagerDock = nullptr;

	DToolBoxManager* mToolBoxManager = nullptr;
	DToolBox* mDToolBox;
	QDockWidget* mDToolBoxDock = nullptr;
	DWelComePage* mWelcomPage = nullptr;

	class QgsCanvasRefreshBlocker
	{
	public:

		QgsCanvasRefreshBlocker()
		{
			if (DGisApp::instance()->mFreezeCount++ == 0)
			{
				// going from unfrozen to frozen, so freeze canvases
				DGisApp::instance()->freezeCanvases(true);
			}
		}
		QgsCanvasRefreshBlocker(const QgsCanvasRefreshBlocker &other) = delete;
		QgsCanvasRefreshBlocker &operator=(const QgsCanvasRefreshBlocker &other) = delete;

		void release()
		{
			if (mReleased)
				return;

			mReleased = true;
			if (--DGisApp::instance()->mFreezeCount == 0)
			{
				// going from frozen to unfrozen, so unfreeze canvases
				DGisApp::instance()->freezeCanvases(false);
				DGisApp::instance()->refreshMapCanvas();
			}
		}

		~QgsCanvasRefreshBlocker()
		{
			if (!mReleased)
				release();
		}

	private:

		bool mReleased = false;
	};

	class Tools
	{
	public:
		Tools() = default;

		QgsMapTool *mZoomIn = nullptr;
		QgsMapTool *mZoomOut = nullptr;
		QgsMapTool *mPan = nullptr;
		QgsMapToolIdentifyAction *mIdentify = nullptr;
		//QgsMapTool *mFeatureAction = nullptr;
		QgsMapTool *mMeasureDist = nullptr;
		QgsMapTool *mMeasureArea = nullptr;
		//QgsMapTool *mMeasureAngle = nullptr;
		QgsMapToolAddFeature *mAddFeature = nullptr;
		DMapToolTemporyDrawer *mTemporyDrawer = nullptr;
		DMapToolDrawSample *mDrawSample = nullptr;
		//QgsMapTool *mCircularStringCurvePoint = nullptr;
		//QgsMapTool *mCircularStringRadius = nullptr;
		//QgsMapTool *mCircle2Points = nullptr;
		//QgsMapTool *mCircle3Points = nullptr;
		//QgsMapTool *mCircle3Tangents = nullptr;
		//QgsMapTool *mCircle2TangentsPoint = nullptr;
		//QgsMapTool *mCircleCenterPoint = nullptr;
		//QgsMapTool *mEllipseCenter2Points = nullptr;
		//QgsMapTool *mEllipseCenterPoint = nullptr;
		//QgsMapTool *mEllipseExtent = nullptr;
		//QgsMapTool *mEllipseFoci = nullptr;
		//QgsMapTool *mRectangleCenterPoint = nullptr;
		//QgsMapTool *mRectangleExtent = nullptr;
		//QgsMapTool *mRectangle3PointsDistance = nullptr;
		//QgsMapTool *mRectangle3PointsProjected = nullptr;
		//QgsMapTool *mRegularPolygon2Points = nullptr;
		//QgsMapTool *mRegularPolygonCenterPoint = nullptr;
		//QgsMapTool *mRegularPolygonCenterCorner = nullptr;
		QgsMapTool *mMoveFeature = nullptr;
		QgsMapTool *mMoveFeatureCopy = nullptr;
		//QgsMapTool *mOffsetCurve = nullptr;
		//QgsMapTool *mReshapeFeatures = nullptr;
		QgsMapTool *mSplitFeatures = nullptr;
		//QgsMapTool *mSplitParts = nullptr;
		QgsMapToolSelect *mSelectFeatures = nullptr;
		QgsMapToolSelect *mSelectPolygon = nullptr;
		QgsMapToolSelect *mSelectFreehand = nullptr;
		//QgsMapToolSelect *mSelectRadius = nullptr;
		//QgsMapTool *mVertexAdd = nullptr;
		//QgsMapTool *mVertexMove = nullptr;
		//QgsMapTool *mVertexDelete = nullptr;
		//QgsMapTool *mAddRing = nullptr;
		//QgsMapTool *mFillRing = nullptr;
		//QgsMapTool *mAddPart = nullptr;
		//QgsMapTool *mSimplifyFeature = nullptr;
		//QgsMapTool *mDeleteRing = nullptr;
		//QgsMapTool *mDeletePart = nullptr;
		//QgsMapTool *mVertexTool = nullptr;
		//QgsMapTool *mVertexToolActiveLayer = nullptr;
		//QgsMapTool *mRotatePointSymbolsTool = nullptr;
		//QgsMapTool *mOffsetPointSymbolTool = nullptr;
		//QgsMapTool *mAnnotation = nullptr;
		//QgsMapTool *mFormAnnotation = nullptr;
		//QgsMapTool *mHtmlAnnotation = nullptr;
		//QgsMapTool *mSvgAnnotation = nullptr;
		//QgsMapTool *mTextAnnotation = nullptr;
		//QgsMapTool *mPinLabels = nullptr;
		//QgsMapTool *mShowHideLabels = nullptr;
		//QgsMapTool *mMoveLabel = nullptr;
		//QgsMapTool *mRotateFeature = nullptr;
		//QgsMapTool *mRotateLabel = nullptr;
		//QgsMapTool *mChangeLabelProperties = nullptr;
		//QgsMapTool *mReverseLine = nullptr;
		//QgsMapTool *mTrimExtendFeature = nullptr;
	} mMapTools;
	friend class DInterface;
};

/**
 * Field value converter for export as vector layer
 * \note Not available in Python bindings
 */
class QgisAppFieldValueConverter : public QgsVectorFileWriter::FieldValueConverter
{
public:
	QgisAppFieldValueConverter(QgsVectorLayer *vl, const QgsAttributeList &attributesAsDisplayedValues);

	QgsField fieldDefinition(const QgsField &field) override;

	QVariant convert(int idx, const QVariant &value) override;

	QgisAppFieldValueConverter *clone() const override;

private:
	QPointer<QgsVectorLayer> mLayer;
	QgsAttributeList mAttributesAsDisplayedValues;
};