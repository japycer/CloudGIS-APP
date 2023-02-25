#include "DInterface.h"
#include"../DGisApp.h"
#include <QFileInfo>
#include <QString>
#include <QMenu>
#include <QDialog>
#include <QAbstractButton>
#include <QSignalMapper>
#include <QTimer>
#include <QUiLoader>
//#include "qgisappstylesheet.h"
//#include "qgsapplayertreeviewmenuprovider.h"
#include "qgsdatumtransformdialog.h"
#include "qgsgui.h"
#include "qgsmaplayer.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgslayertreeview.h"
//#include "qgslayoutdesignerdialog.h"
#include "qgsshortcutsmanager.h"
#include "qgsattributedialog.h"
#include "qgsfields.h"
#include "qgsvectordataprovider.h"
#include"../qgis/qgsfeatureaction.h"
#include "qgsactionmanager.h"
#include"../qgis/qgsattributetabledialog.h"
#include "qgslocatorwidget.h"
#include "qgslocator.h"


DInterface::DInterface(DGisApp *_qgis)
	: qgis(_qgis)
{

}


QgsPluginManagerInterface *DInterface::pluginManagerInterface()
{
	return nullptr;
}

QgsLayerTreeView *DInterface::layerTreeView()
{
	return qgis->layerTreeView();
}

void DInterface::addCustomActionForLayerType(QAction *action,
	QString menu, QgsMapLayerType type, bool allLayers)
{
	//QgsAppLayerTreeViewMenuProvider *menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider *>(qgis->layerTreeView()->menuProvider());
	//if (!menuProvider)
	//	return;
	return;
	//menuProvider->addLegendLayerAction(action, menu, type, allLayers);
}

void DInterface::addCustomActionForLayer(QAction *action, QgsMapLayer *layer)
{
	/*QgsAppLayerTreeViewMenuProvider *menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider *>(qgis->layerTreeView()->menuProvider());
	if (!menuProvider)
		return;
	menuProvider->addLegendLayerActionForLayer(action, layer);*/
	return;
}

bool DInterface::removeCustomActionForLayerType(QAction *action)
{
	/*QgsAppLayerTreeViewMenuProvider *menuProvider = dynamic_cast<QgsAppLayerTreeViewMenuProvider *>(qgis->layerTreeView()->menuProvider());
	if (!menuProvider)
		return false;

	return menuProvider->removeLegendLayerAction(action);*/
	return false;
}

void DInterface::zoomFull()
{
	qgis->zoomFull();
}

void DInterface::zoomToPrevious()
{
	qgis->zoomToPrevious();
}

void DInterface::zoomToNext()
{
	qgis->zoomToNext();
}

void DInterface::zoomToActiveLayer()
{
	qgis->zoomToLayerExtent();
}

QgsVectorLayer *DInterface::addVectorLayer(const QString &vectorLayerPath, const QString &baseName, const QString &providerKey)
{
	QString nonNullBaseBame = baseName;
	if (nonNullBaseBame.isEmpty())
	{
		QFileInfo fi(vectorLayerPath);
		nonNullBaseBame = fi.completeBaseName();
	}
	return qgis->addVectorLayer(vectorLayerPath, nonNullBaseBame, providerKey);
}

QgsRasterLayer *DInterface::addRasterLayer(const QString &rasterLayerPath, const QString &baseName)
{
	QString nonNullBaseName = baseName;
	if (nonNullBaseName.isEmpty())
	{
		QFileInfo fi(rasterLayerPath);
		nonNullBaseName = fi.completeBaseName();
	}
	return qgis->addRasterLayer(rasterLayerPath, nonNullBaseName);
	//return nullptr;
}

QgsRasterLayer *DInterface::addRasterLayer(const QString &url, const QString &baseName, const QString &providerKey)
{
	return qgis->addRasterLayer(url, baseName, providerKey);
	//return nullptr;
}

QgsMeshLayer *DInterface::addMeshLayer(const QString &url, const QString &baseName, const QString &providerKey)
{
	//return qgis->addMeshLayer(url, baseName, providerKey);
	return nullptr;
}

QgsVectorTileLayer *DInterface::addVectorTileLayer(const QString &url, const QString &baseName)
{
	//return qgis->addVectorTileLayer(url, baseName);
	return nullptr;
}

bool DInterface::addProject(const QString &projectName)
{
	return qgis->addProject(projectName);
}

bool DInterface::newProject(bool promptToSaveFlag)
{
	return qgis->fileNew(promptToSaveFlag);
}

void DInterface::reloadConnections()
{
	//qgis->reloadConnections();
	qgis->bindSignalsSlots();
}

QgsMapLayer *DInterface::activeLayer()
{
	return qgis->activeLayer();
}

bool DInterface::setActiveLayer(QgsMapLayer *layer)
{
	return qgis->setActiveLayer(layer);
}

void DInterface::copySelectionToClipboard(QgsMapLayer *layer)
{
	return qgis->copySelectionToClipboard(layer);
}

void DInterface::pasteFromClipboard(QgsMapLayer *layer)
{
	return qgis->pasteFromClipboard(layer);
}

void DInterface::addPluginToMenu(const QString &name, QAction *action)
{
	//qgis->addPluginToMenu(name, action);
}

void DInterface::insertAddLayerAction(QAction *action)
{
	//qgis->insertAddLayerAction(action);
}

void DInterface::removeAddLayerAction(QAction *action)
{
	//qgis->removeAddLayerAction(action);
}

void DInterface::removePluginMenu(const QString &name, QAction *action)
{
	//qgis->removePluginMenu(name, action);
}

void DInterface::addPluginToDatabaseMenu(const QString &name, QAction *action)
{
	//qgis->addPluginToDatabaseMenu(name, action);
}

void DInterface::removePluginDatabaseMenu(const QString &name, QAction *action)
{
	//qgis->removePluginDatabaseMenu(name, action);
}

void DInterface::addPluginToRasterMenu(const QString &name, QAction *action)
{
	//qgis->addPluginToRasterMenu(name, action);
}

void DInterface::removePluginRasterMenu(const QString &name, QAction *action)
{
	//qgis->removePluginRasterMenu(name, action);
}

void DInterface::addPluginToVectorMenu(const QString &name, QAction *action)
{
	//qgis->addPluginToVectorMenu(name, action);
}

void DInterface::removePluginVectorMenu(const QString &name, QAction *action)
{
	//qgis->removePluginVectorMenu(name, action);
}

void DInterface::addPluginToWebMenu(const QString &name, QAction *action)
{
	//qgis->addPluginToWebMenu(name, action);
}

void DInterface::removePluginWebMenu(const QString &name, QAction *action)
{
	//qgis->removePluginWebMenu(name, action);
}

int DInterface::addToolBarIcon(QAction *qAction)
{
	//return qgis->addPluginToolBarIcon(qAction);
	return 0;
}

QAction *DInterface::addToolBarWidget(QWidget *widget)
{
	//return qgis->addPluginToolBarWidget(widget);
	return nullptr;
}

void DInterface::removeToolBarIcon(QAction *qAction)
{
	//qgis->removePluginToolBarIcon(qAction);
}

int DInterface::addRasterToolBarIcon(QAction *qAction)
{
	//return qgis->addRasterToolBarIcon(qAction);
	return 0;
}

QAction *DInterface::addRasterToolBarWidget(QWidget *widget)
{
	//return qgis->addRasterToolBarWidget(widget);
	return nullptr;
}

void DInterface::removeRasterToolBarIcon(QAction *qAction)
{
	//qgis->removeRasterToolBarIcon(qAction);
}

int DInterface::addVectorToolBarIcon(QAction *qAction)
{
	//return qgis->addVectorToolBarIcon(qAction);
	return 0;
}

QAction *DInterface::addVectorToolBarWidget(QWidget *widget)
{
	return nullptr;
	//return qgis->addVectorToolBarWidget(widget);
}

void DInterface::removeVectorToolBarIcon(QAction *qAction)
{
	//qgis->removeVectorToolBarIcon(qAction);
}

int DInterface::addDatabaseToolBarIcon(QAction *qAction)
{
	//return qgis->addDatabaseToolBarIcon(qAction);
	return 0;
}

QAction *DInterface::addDatabaseToolBarWidget(QWidget *widget)
{
	return nullptr;
	//return qgis->addDatabaseToolBarWidget(widget);
}

void DInterface::removeDatabaseToolBarIcon(QAction *qAction)
{
	//qgis->removeDatabaseToolBarIcon(qAction);
}

int DInterface::addWebToolBarIcon(QAction *qAction)
{
	return 0;
	//return qgis->addWebToolBarIcon(qAction);
}

QAction *DInterface::addWebToolBarWidget(QWidget *widget)
{
	return nullptr;
	//return qgis->addWebToolBarWidget(widget);
}

void DInterface::removeWebToolBarIcon(QAction *qAction)
{
	//qgis->removeWebToolBarIcon(qAction);
}

QToolBar *DInterface::addToolBar(const QString &name)
{
	return qgis->addToolBar(name);
}

void DInterface::addToolBar(QToolBar *toolbar, Qt::ToolBarArea area)
{
	//return qgis->addToolBar(toolbar, area);
}

void DInterface::openURL(const QString &url, bool useQgisDocDirectory)
{
	//qgis->openURL(url, useQgisDocDirectory);
}

QgsMapCanvas *DInterface::mapCanvas()
{
	return qgis->mapCanvas();
}

QList<QgsMapCanvas *> DInterface::mapCanvases()
{
	return qgis->mapCanvases();
}

QgsMapCanvas *DInterface::createNewMapCanvas(const QString &name)
{
	//return qgis->createNewMapCanvas(name);
	return nullptr;
}

void DInterface::closeMapCanvas(const QString &name)
{
	//qgis->closeMapCanvas(name);
}

QSize DInterface::iconSize(bool dockedToolbar) const
{
	return qgis->iconSize(dockedToolbar);
}

QgsLayerTreeMapCanvasBridge *DInterface::layerTreeCanvasBridge()
{
	return nullptr;
	//return qgis->layerTreeCanvasBridge();
}

QWidget *DInterface::mainWindow()
{
	return qgis;
}

QgsMessageBar *DInterface::messageBar()
{
	return qgis->messageBar();
}

void DInterface::openMessageLog()
{
	//qgis->openMessageLog();
}


void DInterface::addUserInputWidget(QWidget *widget)
{
	qgis->addUserInputWidget(widget);
}

void DInterface::showLayoutManager()
{
	//qgis->showLayoutManager();
}

QList<QgsLayoutDesignerInterface *> DInterface::openLayoutDesigners()
{
	QList<QgsLayoutDesignerInterface *> designerInterfaceList;
	//if (qgis)
	//{
	//	const QSet<QgsLayoutDesignerDialog *> designerList = qgis->layoutDesigners();
	//	QSet<QgsLayoutDesignerDialog *>::const_iterator it = designerList.constBegin();
	//	for (; it != designerList.constEnd(); ++it)
	//	{
	//		if (*it)
	//		{
	//			QgsLayoutDesignerInterface *v = (*it)->iface();
	//			if (v)
	//			{
	//				designerInterfaceList << v;
	//			}
	//		}
	//	}
	//}
	return designerInterfaceList;
}

QgsLayoutDesignerInterface *DInterface::openLayoutDesigner(QgsMasterLayoutInterface *layout)
{
	//QgsLayoutDesignerDialog *designer = qgis->openLayoutDesignerDialog(layout);
	//if (designer)
	//{
	//	return designer->iface();
	//}
	return nullptr;
}

void DInterface::showOptionsDialog(QWidget *parent, const QString &currentPage)
{
	//return qgis->showOptionsDialog(parent, currentPage);
}

void DInterface::showProjectPropertiesDialog(const QString &currentPage)
{
	//return qgis->showProjectProperties(currentPage);
}

QMap<QString, QVariant> DInterface::defaultStyleSheetOptions()
{
	return *new QMap<QString, QVariant>();
	//return qgis->styleSheetBuilder()->defaultOptions();
}

void DInterface::buildStyleSheet(const QMap<QString, QVariant> &opts)
{
	//qgis->styleSheetBuilder()->buildStyleSheet(opts);
}

void DInterface::saveStyleSheetOptions(const QMap<QString, QVariant> &opts)
{
	//qgis->styleSheetBuilder()->saveToSettings(opts);
}

QFont DInterface::defaultStyleSheetFont()
{
	//return qgis->styleSheetBuilder()->defaultFont();
	return nullptr;
}

void DInterface::addDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget)
{
	qgis->addDockWidget(area, dockwidget);
}

void DInterface::addTabifiedDockWidget(Qt::DockWidgetArea area, QDockWidget *dockwidget, const QStringList &tabifyWith, bool raiseTab)
{
	qgis->addTabifiedDockWidget(area, dockwidget, tabifyWith, raiseTab);
}

void DInterface::removeDockWidget(QDockWidget *dockwidget)
{
	qgis->removeDockWidget(dockwidget);
}


QgsAdvancedDigitizingDockWidget *DInterface::cadDockWidget()
{
	//return qgis->cadDockWidget();
	return nullptr;
}

void DInterface::showLayerProperties(QgsMapLayer *l)
{
	if (l && qgis)
	{
		//qgis->showLayerProperties(l);
	}
}

QDialog *DInterface::showAttributeTable(QgsVectorLayer *l, const QString &filterExpression)
{
	if (l)
	{
		QgsAttributeTableDialog *dialog = new QgsAttributeTableDialog(l, QgsAttributeTableFilterModel::ShowFilteredList);
		dialog->setFilterExpression(filterExpression);
		dialog->show();
		return dialog;
	}
	return nullptr;
}

void DInterface::addWindow(QAction *action)
{
	//qgis->addWindow(action);
}

void DInterface::removeWindow(QAction *action)
{
	//qgis->removeWindow(action);
}

bool DInterface::registerMainWindowAction(QAction *action, const QString &defaultShortcut)
{
	return QgsGui::shortcutsManager()->registerAction(action, defaultShortcut);
}

bool DInterface::unregisterMainWindowAction(QAction *action)
{
	return QgsGui::shortcutsManager()->unregisterAction(action);
}

void DInterface::registerMapLayerConfigWidgetFactory(QgsMapLayerConfigWidgetFactory *factory)
{
	//qgis->registerMapLayerPropertiesFactory(factory);
}

void DInterface::unregisterMapLayerConfigWidgetFactory(QgsMapLayerConfigWidgetFactory *factory)
{
	//qgis->unregisterMapLayerPropertiesFactory(factory);
}

void DInterface::registerOptionsWidgetFactory(QgsOptionsWidgetFactory *factory)
{
	//qgis->registerOptionsWidgetFactory(factory);
}

void DInterface::unregisterOptionsWidgetFactory(QgsOptionsWidgetFactory *factory)
{
	//qgis->unregisterOptionsWidgetFactory(factory);
}

void DInterface::registerProjectPropertiesWidgetFactory(QgsOptionsWidgetFactory *factory)
{
	//qgis->registerProjectPropertiesWidgetFactory(factory);
}

void DInterface::unregisterProjectPropertiesWidgetFactory(QgsOptionsWidgetFactory *factory)
{
	//qgis->unregisterProjectPropertiesWidgetFactory(factory);
}

void DInterface::registerDevToolWidgetFactory(QgsDevToolWidgetFactory *factory)
{
	//qgis->registerDevToolFactory(factory);
}

void DInterface::unregisterDevToolWidgetFactory(QgsDevToolWidgetFactory *factory)
{
	//qgis->unregisterDevToolFactory(factory);
}

void DInterface::registerApplicationExitBlocker(QgsApplicationExitBlockerInterface *blocker)
{
	//qgis->registerApplicationExitBlocker(blocker);
}

void DInterface::unregisterApplicationExitBlocker(QgsApplicationExitBlockerInterface *blocker)
{
	//qgis->unregisterApplicationExitBlocker(blocker);
}

void DInterface::registerMapToolHandler(QgsAbstractMapToolHandler *handler)
{
	//qgis->registerMapToolHandler(handler);
}

void DInterface::unregisterMapToolHandler(QgsAbstractMapToolHandler *handler)
{
	//qgis->unregisterMapToolHandler(handler);
}

void DInterface::registerCustomDropHandler(QgsCustomDropHandler *handler)
{
	//qgis->registerCustomDropHandler(handler);
}

void DInterface::registerCustomLayoutDropHandler(QgsLayoutCustomDropHandler *handler)
{
	//qgis->registerCustomLayoutDropHandler(handler);
}

void DInterface::unregisterCustomLayoutDropHandler(QgsLayoutCustomDropHandler *handler)
{
	//qgis->unregisterCustomLayoutDropHandler(handler);
}

void DInterface::unregisterCustomDropHandler(QgsCustomDropHandler *handler)
{
	//qgis->unregisterCustomDropHandler(handler);
}

void DInterface::registerCustomProjectOpenHandler(QgsCustomProjectOpenHandler *handler)
{
	//qgis->registerCustomProjectOpenHandler(handler);
}

void DInterface::unregisterCustomProjectOpenHandler(QgsCustomProjectOpenHandler *handler)
{
	//qgis->unregisterCustomProjectOpenHandler(handler);
}

QMenu *DInterface::projectMenu() { return nullptr; }
QMenu *DInterface::editMenu() { return nullptr; }
QMenu *DInterface::viewMenu() { return nullptr; }
QMenu *DInterface::layerMenu() { return nullptr; }
QMenu *DInterface::newLayerMenu() { return nullptr; }
QMenu *DInterface::addLayerMenu() { return nullptr; }
QMenu *DInterface::settingsMenu() { return nullptr; }
QMenu *DInterface::pluginMenu() { return nullptr; }
QMenu *DInterface::pluginHelpMenu() { return nullptr; }
QMenu *DInterface::rasterMenu() { return nullptr; }
QMenu *DInterface::vectorMenu() { return nullptr; }
QMenu *DInterface::databaseMenu() { return nullptr; }
QMenu *DInterface::webMenu() { return nullptr; }
QMenu *DInterface::firstRightStandardMenu() { return nullptr; }
QMenu *DInterface::windowMenu() { return nullptr; }
QMenu *DInterface::helpMenu() { return nullptr; }

QToolBar *DInterface::fileToolBar() { return nullptr; }
QToolBar *DInterface::layerToolBar() { return nullptr; }
QToolBar *DInterface::dataSourceManagerToolBar() { return nullptr; }
QToolBar *DInterface::mapNavToolToolBar() { return nullptr; }
QToolBar *DInterface::digitizeToolBar() { return nullptr; }
QToolBar *DInterface::advancedDigitizeToolBar() { return nullptr; }
QToolBar *DInterface::shapeDigitizeToolBar() { return nullptr; }
QToolBar *DInterface::attributesToolBar() { return nullptr; }
QToolBar *DInterface::selectionToolBar() { return nullptr; }
QToolBar *DInterface::pluginToolBar() { return nullptr; }
QToolBar *DInterface::helpToolBar() { return nullptr; }
QToolBar *DInterface::rasterToolBar() { return nullptr; }
QToolBar *DInterface::vectorToolBar() { return nullptr; }
QToolBar *DInterface::databaseToolBar() { return nullptr; }
QToolBar *DInterface::webToolBar() { return nullptr; }
QActionGroup *DInterface::mapToolActionGroup() { return nullptr; }
QAction *DInterface::actionNewProject() { return qgis->ui.mActionNewProject; }
QAction *DInterface::actionOpenProject() { return qgis->ui.mActionOpenProject; }
QAction *DInterface::actionSaveProject() { return qgis->ui.mActionSaveProject; }
QAction *DInterface::actionSaveProjectAs() { return nullptr; }
QAction *DInterface::actionSaveMapAsImage() { return nullptr; }
QAction *DInterface::actionProjectProperties() { return nullptr; }
QAction *DInterface::actionCreatePrintLayout() { return nullptr; }
QAction *DInterface::actionShowLayoutManager() { return nullptr; }
QAction *DInterface::actionExit() { return nullptr; }

QAction *DInterface::actionCutFeatures() { return qgis->ui.mActionCutFeatures; }
QAction *DInterface::actionCopyFeatures() { return qgis->ui.mActionCopyFeatures; }
QAction *DInterface::actionPasteFeatures() { return qgis->ui.mActionPasteFeatures; }
QAction *DInterface::actionAddFeature() { return qgis->ui.mActionAddFeature; }
QAction *DInterface::actionDeleteSelected() { return qgis->ui.mActionDeleteSelected; }
QAction *DInterface::actionMoveFeature() { return qgis->ui.mActionMoveFeature; }
QAction *DInterface::actionSplitFeatures() { return nullptr; }
QAction *DInterface::actionSplitParts() { return nullptr; }
QAction *DInterface::actionAddRing() { return nullptr; }
QAction *DInterface::actionAddPart() { return nullptr; }
QAction *DInterface::actionSimplifyFeature() { return nullptr; }
QAction *DInterface::actionDeleteRing() { return nullptr; }
QAction *DInterface::actionDeletePart() { return nullptr; }
QAction *DInterface::actionVertexTool() { return nullptr; }
QAction *DInterface::actionVertexToolActiveLayer() { return nullptr; }

QAction *DInterface::actionPan() { return qgis->ui.mActionPan; }
QAction *DInterface::actionPanToSelected() { return qgis->ui.mActionPanToSelected; }
QAction *DInterface::actionZoomIn() { return qgis->ui.mActionZoomIn; }
QAction *DInterface::actionZoomOut() { return qgis->ui.mActionZoomOut; }
QAction *DInterface::actionSelect() { return qgis->ui.mActionSelectFeatures; }
QAction *DInterface::actionSelectRectangle() { return qgis->ui.mActionSelectPolygon; }
QAction *DInterface::actionSelectPolygon() { return qgis->ui.mActionSelectPolygon; }
QAction *DInterface::actionSelectFreehand() { return qgis->ui.mActionSelectFreehand; }
QAction *DInterface::actionSelectRadius() { return nullptr; }
QAction *DInterface::actionIdentify() { return qgis->ui.mActionIdentify; }
QAction *DInterface::actionFeatureAction() { return nullptr; }
QAction *DInterface::actionMeasure() { return qgis->ui.mActionMeasure; }
QAction *DInterface::actionMeasureArea() { return qgis->ui.mActionMeasureArea; }
QAction *DInterface::actionZoomFullExtent() { return qgis->ui.mActionZoomFullExtent; }
QAction *DInterface::actionZoomToLayer() { return qgis->ui.mActionZoomToLayer; }
QAction *DInterface::actionZoomToSelected() { return qgis->ui.mActionZoomToSelected; }
QAction *DInterface::actionZoomLast() { return qgis->ui.mActionZoomLast; }
QAction *DInterface::actionZoomNext() { return qgis->ui.mActionZoomNext; }
QAction *DInterface::actionZoomActualSize() { return nullptr; }
QAction *DInterface::actionMapTips() { return nullptr; }
QAction *DInterface::actionNewBookmark() { return nullptr; }
QAction *DInterface::actionShowBookmarks() { return nullptr; }
QAction *DInterface::actionDraw() { return qgis->ui.mActionDraw; }
QAction *DInterface::actionCircle2Points() { return nullptr; }
QAction *DInterface::actionCircle3Points() { return nullptr; }
QAction *DInterface::actionCircle3Tangents() { return nullptr; }
QAction *DInterface::actionCircle2TangentsPoint() { return nullptr; }
QAction *DInterface::actionCircleCenterPoint() { return nullptr; }
QAction *DInterface::actionEllipseCenter2Points() { return nullptr; }
QAction *DInterface::actionEllipseCenterPoint() { return nullptr; }
QAction *DInterface::actionEllipseExtent() { return nullptr; }
QAction *DInterface::actionEllipseFoci() { return nullptr; }
QAction *DInterface::actionRectangleCenterPoint() { return nullptr; }
QAction *DInterface::actionRectangleExtent() { return nullptr; }
QAction *DInterface::actionRectangle3PointsDistance() { return nullptr; }
QAction *DInterface::actionRectangle3PointsProjected() { return nullptr; }
QAction *DInterface::actionRegularPolygon2Points() { return nullptr; }
QAction *DInterface::actionRegularPolygonCenterPoint() { return nullptr; }
QAction *DInterface::actionRegularPolygonCenterCorner() { return nullptr; }
//! Layer menu actions
QAction *DInterface::actionNewVectorLayer() { return qgis->ui.mActionNewVectorLayer; }
QAction *DInterface::actionAddOgrLayer() { return nullptr; }
QAction *DInterface::actionAddRasterLayer() { return qgis->ui.mActionAddRasterLayer; }
QAction *DInterface::actionAddPgLayer() { return nullptr; }
QAction *DInterface::actionAddWmsLayer() { return nullptr; }
QAction *DInterface::actionAddXyzLayer() { return nullptr; }
QAction *DInterface::actionAddVectorTileLayer() { return nullptr; }
QAction *DInterface::actionAddAfsLayer() { return nullptr; }
QAction *DInterface::actionAddAmsLayer() { return nullptr; }
QAction *DInterface::actionCopyLayerStyle() { return nullptr; }
QAction *DInterface::actionPasteLayerStyle() { return nullptr; }
QAction *DInterface::actionOpenTable() { return nullptr; }
QAction *DInterface::actionOpenFieldCalculator() { return nullptr; }
QAction *DInterface::actionOpenStatisticalSummary() { return nullptr; }
QAction *DInterface::actionToggleEditing() { return qgis->ui.mActionToggleEditing; }
QAction *DInterface::actionSaveActiveLayerEdits() { return nullptr; }
QAction *DInterface::actionAllEdits() { return qgis->ui.mActionSaveAllEdits; }
QAction *DInterface::actionSaveEdits() { return qgis->ui.mActionSaveEdits; }
QAction *DInterface::actionSaveAllEdits() { return qgis->ui.mActionSaveAllEdits; }
QAction *DInterface::actionRollbackEdits() { return nullptr; }
QAction *DInterface::actionRollbackAllEdits() { return nullptr; }
QAction *DInterface::actionCancelEdits() { return nullptr; }
QAction *DInterface::actionCancelAllEdits() { return nullptr; }
QAction *DInterface::actionLayerSaveAs() { return nullptr; }
QAction *DInterface::actionDuplicateLayer() { return nullptr; }
QAction *DInterface::actionLayerProperties() { return nullptr; }
QAction *DInterface::actionAddToOverview() { return nullptr; }
QAction *DInterface::actionAddAllToOverview() { return nullptr; }
QAction *DInterface::actionRemoveAllFromOverview() { return nullptr; }
QAction *DInterface::actionHideAllLayers() { return qgis->ui.mActionHideAllLayers; }
QAction *DInterface::actionShowAllLayers() { return qgis->ui.mActionShowAllLayers; }
QAction *DInterface::actionHideSelectedLayers() { return qgis->ui.mActionHideSelectedLayers; }
QAction *DInterface::actionToggleSelectedLayers() { return nullptr; }
QAction *DInterface::actionToggleSelectedLayersIndependently() { return nullptr; }
QAction *DInterface::actionHideDeselectedLayers() { return nullptr; }
QAction *DInterface::actionShowSelectedLayers() { return nullptr; }

QAction *DInterface::actionManagePlugins() { return nullptr; }
QAction *DInterface::actionPluginListSeparator() { return nullptr; }
QAction *DInterface::actionShowPythonDialog() { return qgis->ui.mActionShowPythonDialog; }

QAction *DInterface::actionToggleFullScreen() { return nullptr; }
QAction *DInterface::actionOptions() { return nullptr; }
QAction *DInterface::actionCustomProjection() { return nullptr; }

QAction *DInterface::actionHelpContents() { return nullptr; }
QAction *DInterface::actionQgisHomePage() { return nullptr; }
QAction *DInterface::actionCheckQgisVersion() { return nullptr; }
QAction *DInterface::actionAbout() { return qgis->ui.mActionAbout; }

bool DInterface::openFeatureForm(QgsVectorLayer *vlayer, QgsFeature &f, bool updateFeatureOnly, bool showModal)
{
	Q_UNUSED(updateFeatureOnly)
		if (!vlayer)
			return false;

	QgsFeatureAction action(tr("Attributes changed"), f, vlayer, QString(), -1, DGisApp::instance());
	if (vlayer->isEditable())
	{
		return action.editFeature(showModal);
	}
	else
	{
		action.viewFeatureForm();
		return true;
	}
}

void DInterface::preloadForm(const QString &uifile)
{
	QTimer::singleShot(0, this, [=]
	{
		cacheloadForm(uifile);
	});
}

void DInterface::cacheloadForm(const QString &uifile)
{
	QFile file(uifile);

	if (file.open(QFile::ReadOnly))
	{
		QUiLoader loader;

		QFileInfo fi(uifile);
		loader.setWorkingDirectory(fi.dir());
		QWidget *myWidget = loader.load(&file);
		file.close();
		delete myWidget;
	}
}

QgsAttributeDialog *DInterface::getFeatureForm(QgsVectorLayer *l, QgsFeature &feature)
{
	QgsDistanceArea myDa;

	myDa.setSourceCrs(l->crs(), QgsProject::instance()->transformContext());
	myDa.setEllipsoid(QgsProject::instance()->ellipsoid());

	QgsAttributeEditorContext context(DGisApp::instance()->createAttributeEditorContext());
	context.setDistanceArea(myDa);
	QgsAttributeDialog *dialog = new QgsAttributeDialog(l, &feature, false, qgis, true, context);
	if (!feature.isValid())
	{
		dialog->setMode(QgsAttributeEditorContext::AddFeatureMode);
	}
	return dialog;
}

QgsVectorLayerTools *DInterface::vectorLayerTools()
{
	return qgis->vectorLayerTools();
}

QList<QgsMapLayer *> DInterface::editableLayers(bool modified) const
{
	//return qgis->editableLayers(modified);
	return *new QList<QgsMapLayer *>();
}

int DInterface::messageTimeout()
{
	return qgis->messageTimeout();
}

QgsStatusBar *DInterface::statusBarIface()
{
	//return qgis->statusBarIface();
	return nullptr;
}

void DInterface::locatorSearch(const QString &searchText)
{
	//qgis->mLocatorWidget->invalidateResults();
	//qgis->mLocatorWidget->search(searchText);
}

void DInterface::registerLocatorFilter(QgsLocatorFilter *filter)
{
	//qgis->mLocatorWidget->locator()->registerFilter(filter);
}

void DInterface::deregisterLocatorFilter(QgsLocatorFilter *filter)
{
	//qgis->mLocatorWidget->locator()->deregisterFilter(filter);
}

void DInterface::invalidateLocatorResults()
{
	//qgis->mLocatorWidget->invalidateResults();
}

bool DInterface::askForDatumTransform(QgsCoordinateReferenceSystem sourceCrs, QgsCoordinateReferenceSystem destinationCrs)
{
	return qgis->askUserForDatumTransform(sourceCrs, destinationCrs, nullptr);
}

void DInterface::takeAppScreenShots(const QString &saveDirectory, const int categories)
{
	//qgis->takeAppScreenShots(saveDirectory, categories);
}

QgsBrowserGuiModel *DInterface::browserModel()
{
	//return qgis->mBrowserModel;
	return nullptr;
}

QgsLayerTreeRegistryBridge::InsertionPoint DInterface::layerTreeInsertionPoint()
{
	return qgis->layerTreeInsertionPoint();
}

void DInterface::setGpsPanelConnection(QgsGpsConnection *connection)
{
	//qgis->setGpsPanelConnection(connection);
}
