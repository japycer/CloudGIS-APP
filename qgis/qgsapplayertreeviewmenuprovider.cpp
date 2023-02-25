#include <QClipboard>
#include "qgsapplayertreeviewmenuprovider.h"
#include "../DGisApp.h"
#include "qgsapplication.h"
#include "qgsclipboard.h"
#include "qgscolorwidgets.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorswatchgrid.h"
#include "qgsgui.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeviewdefaultactions.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerstyleguiutils.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrenderer.h"
#include "qgssymbol.h"
#include "qgsstyle.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslayertreeregistrybridge.h"
#include "qgssymbolselectordialog.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmaplayerstylecategoriesmodel.h"
#include "qgssymbollayerutils.h"
#include "qgsxmlutils.h"
#include "qgsmessagebar.h"


QgsAppLayerTreeViewMenuProvider::QgsAppLayerTreeViewMenuProvider( QgsLayerTreeView *view, QgsMapCanvas *canvas )
  : mView( view )
  , mCanvas( canvas )
{
}

QMenu *QgsAppLayerTreeViewMenuProvider::createContextMenu()
{
  QMenu *menu = new QMenu;

  QgsLayerTreeViewDefaultActions *actions = mView->defaultActions();

  QModelIndex idx = mView->currentIndex();
  if ( !idx.isValid() )
  {
    // global menu
    menu->addAction( actions->actionAddGroup( menu ) );
    menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionExpandTree.svg" ) ), tr( "&Expand All" ), mView, &QgsLayerTreeView::expandAll );
    menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCollapseTree.svg" ) ), tr( "&Collapse All" ), mView, &QgsLayerTreeView::collapseAll );
    menu->addSeparator();
    // TODO: update drawing order
  }
  else if ( QgsLayerTreeNode *node = mView->layerTreeModel()->index2node( idx ) )
  {
    // layer or group selected
    if ( QgsLayerTree::isGroup( node ) )
    {
      menu->addAction( actions->actionZoomToGroup( mCanvas, menu ) );
      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );

      menu->addSeparator();
      menu->addAction( actions->actionAddGroup( menu ) );
      QAction *removeAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveLayer.svg" ) ), tr( "&Remove Group..." ), DGisApp::instance(), &DGisApp::removeLayer );
      removeAction->setEnabled( removeActionEnabled() );
      menu->addSeparator();

      menu->addAction( actions->actionMutuallyExclusiveGroup( menu ) );
      if ( QAction *checkAll = actions->actionCheckAndAllChildren( menu ) )
        menu->addAction( checkAll );

      if ( QAction *unCheckAll = actions->actionUncheckAndAllChildren( menu ) )
        menu->addAction( unCheckAll );

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == 0 ) )
      {
        menu->addAction( actions->actionMoveToTop( menu ) );
      }

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == idx.model()->rowCount() - 1 ) )
      {
        menu->addAction( actions->actionMoveToBottom( menu ) );
      }

      menu->addSeparator();

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

      QMenu *menuExportGroup = new QMenu( tr( "Export" ), menu );
      menu->addMenu( menuExportGroup );
    }
    else if ( QgsLayerTree::isLayer( node ) )
    {
      QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
      QgsRasterLayer *rlayer = qobject_cast<QgsRasterLayer *>( layer );
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );

      if ( layer && layer->isSpatial() )
      {
        QAction *zoomToLayer = actions->actionZoomToLayer( mCanvas, menu );
        zoomToLayer->setEnabled( layer->isValid() );
        menu->addAction( zoomToLayer );
        if ( vlayer )
        {
          QAction *actionZoomSelected = actions->actionZoomToSelection( mCanvas, menu );
          actionZoomSelected->setEnabled( vlayer->isValid() && !vlayer->selectedFeatureIds().isEmpty() );
          menu->addAction( actionZoomSelected );
        }
        menu->addAction( actions->actionShowInOverview( menu ) );
      }

      if ( vlayer )
      {
        QAction *showFeatureCount = actions->actionShowFeatureCount( menu );
        menu->addAction( showFeatureCount );
        showFeatureCount->setEnabled( vlayer->isValid() );
      }

      menu->addAction( actions->actionRenameGroupOrLayer( menu ) );
      menu->addSeparator();

      QAction *removeAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemoveLayer.svg" ) ), tr( "&Remove Layer..." ), DGisApp::instance(), &DGisApp::removeLayer );
      removeAction->setEnabled( removeActionEnabled() );

      menu->addSeparator();

      if ( node->parent() != mView->layerTreeModel()->rootGroup() )
        menu->addAction( actions->actionMoveOutOfGroup( menu ) );

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == 0 ) )
      {
        menu->addAction( actions->actionMoveToTop( menu ) );
      }

      if ( !( mView->selectedNodes( true ).count() == 1 && idx.row() == idx.model()->rowCount() - 1 ) )
      {
        menu->addAction( actions->actionMoveToBottom( menu ) );
      }

      QAction *checkAll = actions->actionCheckAndAllParents( menu );
      if ( checkAll )
        menu->addAction( checkAll );

      if ( mView->selectedNodes( true ).count() >= 2 )
        menu->addAction( actions->actionGroupSelected( menu ) );

      menu->addSeparator();

      if ( vlayer )
      {
        QAction *toggleEditingAction = DGisApp::instance()->actionToggleEditing();
        QAction *saveLayerEditsAction = DGisApp::instance()->actionSaveActiveLayerEdits();

        // attribute table
        QgsSettings settings;
        QgsAttributeTableFilterModel::FilterMode initialMode = settings.enumValue( QStringLiteral( "qgis/attributeTableBehavior" ),  QgsAttributeTableFilterModel::ShowAll );
        const auto lambdaOpenAttributeTable = [ = ] { DGisApp::instance()->attributeTable( initialMode ); };
        QAction *attributeTableAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ), tr( "&Open Attribute Table" ),
                                        DGisApp::instance(), lambdaOpenAttributeTable );
        attributeTableAction->setEnabled( vlayer->isValid() );

        // allow editing
        const QgsVectorDataProvider *provider = vlayer->dataProvider();
        if ( provider &&
             ( provider->capabilities() & QgsVectorDataProvider::EditingCapabilities ) )
        {
          if ( toggleEditingAction )
          {
            menu->addAction( toggleEditingAction );
            // toggleEditingAction->setChecked( vlayer->isEditable() );
            toggleEditingAction->setEnabled( vlayer->isValid() );
          }
          if ( saveLayerEditsAction && vlayer->isModified() )
          {
            menu->addAction( saveLayerEditsAction );
          }
        }
      }
      menu->addSeparator();

      if ( layer && layer->isSpatial() )
      {
        QMenu *menuSetCRS = new QMenu( tr( "Layer CRS" ), menu );

        const QList<QgsLayerTreeNode *> selectedNodes = mView->selectedNodes();
        QgsCoordinateReferenceSystem layerCrs;
        bool firstLayer = true;
        bool allSameCrs = true;
        for ( QgsLayerTreeNode *node : selectedNodes )
        {
          if ( QgsLayerTree::isLayer( node ) )
          {
            QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
            if ( nodeLayer->layer() )
            {
              if ( firstLayer )
              {
                layerCrs = nodeLayer->layer()->crs();
                firstLayer = false;
              }
              else if ( nodeLayer->layer()->crs() != layerCrs )
              {
                allSameCrs = false;
                break;
              }
            }
          }
        }

        QAction *actionCurrentCrs = new QAction( !allSameCrs ? tr( "Mixed CRS" )
            : layer->crs().isValid() ? layer->crs().userFriendlyIdentifier()
            : tr( "No CRS" ), menuSetCRS );
        actionCurrentCrs->setEnabled( false );
        menuSetCRS->addAction( actionCurrentCrs );

        if ( allSameCrs && layerCrs.isValid() )
        {
          // assign layer crs to project
          QAction *actionSetProjectCrs = new QAction( tr( "Set &Project CRS from Layer" ), menuSetCRS );
          connect( actionSetProjectCrs, &QAction::triggered, DGisApp::instance(), &DGisApp::setProjectCrsFromLayer );
          menuSetCRS->addAction( actionSetProjectCrs );
        }

        const QList< QgsCoordinateReferenceSystem> recentProjections = QgsCoordinateReferenceSystem::recentCoordinateReferenceSystems();
        if ( !recentProjections.isEmpty() )
        {
          menuSetCRS->addSeparator();
          int i = 0;
          for ( const QgsCoordinateReferenceSystem &crs : recentProjections )
          {
            if ( crs == layer->crs() )
              continue;

            QAction *action = menuSetCRS->addAction( tr( "Set to %1" ).arg( crs.userFriendlyIdentifier( QgsCoordinateReferenceSystem::ShortString ) ) );
            connect( action, &QAction::triggered, this, [ = ]
            {
              setLayerCrs( crs );
            } );

            i++;
            if ( i == 5 )
              break;
          }
        }

        // set layer crs
        menuSetCRS->addSeparator();
        QAction *actionSetLayerCrs = new QAction( tr( "Set Layer CRS..." ), menuSetCRS );
        connect( actionSetLayerCrs, &QAction::triggered, DGisApp::instance(), &DGisApp::setLayerCrs );
        menuSetCRS->addAction( actionSetLayerCrs );

        menu->addMenu( menuSetCRS );
      }

      menu->addSeparator();

      if ( vlayer )
      {
        // save as vector file
        QMenu *menuExportVector = new QMenu( tr( "Export" ), menu );
        QAction *actionSaveAs = new QAction( tr( "Save Features As..." ), menuExportVector );
        connect( actionSaveAs, &QAction::triggered, DGisApp::instance(), [ = ] { DGisApp::instance()->saveAsFile(); } );
        actionSaveAs->setEnabled( vlayer->isValid() );
        menuExportVector->addAction( actionSaveAs );
        QAction *actionSaveSelectedFeaturesAs = new QAction( tr( "Save Selected Features As..." ), menuExportVector );
        connect( actionSaveSelectedFeaturesAs, &QAction::triggered, DGisApp::instance(), [ = ] { DGisApp::instance()->saveAsFile( nullptr, true ); } );
        actionSaveSelectedFeaturesAs->setEnabled( vlayer->isValid() && vlayer->selectedFeatureCount() > 0 );
        menuExportVector->addAction( actionSaveSelectedFeaturesAs );
        menu->addMenu( menuExportVector );
      }
      else if ( rlayer )
      {
        QMenu *menuExportRaster = new QMenu( tr( "Export" ), menu );
        QAction *actionSaveAs = new QAction( tr( "Save As..." ), menuExportRaster );
        connect( actionSaveAs, &QAction::triggered, DGisApp::instance(), [ = ] { DGisApp::instance()->saveAsFile(); } );
        menuExportRaster->addAction( actionSaveAs );
        actionSaveAs->setEnabled( rlayer->isValid() );
        menu->addMenu( menuExportRaster );
      }

      menu->addSeparator();
      /*if ( layer && QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
        menu->addAction( tr( "&Properties..." ), DGisApp::instance(), &DGisApp::layerProperties );*/
    }
  }
  else if ( QgsLayerTreeModelLegendNode *node = mView->layerTreeModel()->index2legendNode( idx ) )
  {
    if ( QgsSymbolLegendNode *symbolNode = qobject_cast< QgsSymbolLegendNode * >( node ) )
    {
      // symbology item
      if ( symbolNode->flags() & Qt::ItemIsUserCheckable )
      {
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleAllLayers.svg" ) ), tr( "&Toggle Items" ),
                         symbolNode, &QgsSymbolLegendNode::toggleAllItems );
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayers.svg" ) ), tr( "&Show All Items" ),
                         symbolNode, &QgsSymbolLegendNode::checkAllItems );
        menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHideAllLayers.svg" ) ), tr( "&Hide All Items" ),
                         symbolNode, &QgsSymbolLegendNode::uncheckAllItems );
        menu->addSeparator();
      }

      if ( symbolNode->symbol() )
      {
        QgsColorWheel *colorWheel = new QgsColorWheel( menu );
        colorWheel->setColor( symbolNode->symbol()->color() );
        QgsColorWidgetAction *colorAction = new QgsColorWidgetAction( colorWheel, menu, menu );
        colorAction->setDismissOnColorSelection( false );
        connect( colorAction, &QgsColorWidgetAction::colorChanged, this, &QgsAppLayerTreeViewMenuProvider::setSymbolLegendNodeColor );
        //store the layer id and rule key in action, so we can later retrieve the corresponding
        //legend node, if it still exists
        colorAction->setProperty( "layerId", symbolNode->layerNode()->layerId() );
        colorAction->setProperty( "ruleKey", symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() );
        menu->addAction( colorAction );

        //add recent colors action
        QList<QgsRecentColorScheme *> recentSchemes;
        QgsApplication::colorSchemeRegistry()->schemes( recentSchemes );
        if ( !recentSchemes.isEmpty() )
        {
          QgsColorSwatchGridAction *recentColorAction = new QgsColorSwatchGridAction( recentSchemes.at( 0 ), menu, QStringLiteral( "symbology" ), menu );
          recentColorAction->setProperty( "layerId", symbolNode->layerNode()->layerId() );
          recentColorAction->setProperty( "ruleKey", symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString() );
          recentColorAction->setDismissOnColorSelection( false );
          menu->addAction( recentColorAction );
          connect( recentColorAction, &QgsColorSwatchGridAction::colorChanged, this, &QgsAppLayerTreeViewMenuProvider::setSymbolLegendNodeColor );
        }

        menu->addSeparator();
      }

      const QString layerId = symbolNode->layerNode()->layerId();
      const QString ruleKey = symbolNode->data( QgsLayerTreeModelLegendNode::RuleKeyRole ).toString();

      QAction *editSymbolAction = new QAction( tr( "Edit Symbol..." ), menu );
      connect( editSymbolAction, &QAction::triggered, this, [this, layerId, ruleKey ]
      {
        editSymbolLegendNodeSymbol( layerId, ruleKey );
      } );
      menu->addAction( editSymbolAction );

      QAction *copySymbolAction = new QAction( tr( "Copy Symbol" ), menu );
      connect( copySymbolAction, &QAction::triggered, this, [this, layerId, ruleKey ]
      {
        copySymbolLegendNodeSymbol( layerId, ruleKey );
      } );
      menu->addAction( copySymbolAction );

      bool enablePaste = false;
      std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
      if ( tempSymbol )
        enablePaste = true;

      QAction *pasteSymbolAction = new QAction( tr( "Paste Symbol" ), menu );
      connect( pasteSymbolAction, &QAction::triggered, this, [this, layerId, ruleKey]
      {
        pasteSymbolLegendNodeSymbol( layerId, ruleKey );
      } );
      pasteSymbolAction->setEnabled( enablePaste );
      menu->addAction( pasteSymbolAction );
    }
  }
  return menu;
}

void QgsAppLayerTreeViewMenuProvider::addLegendLayerAction( QAction *action, const QString &menu,
    QgsMapLayerType type, bool allLayers )
{
  mLegendLayerActionMap[type].append( LegendLayerAction( action, menu, allLayers ) );
}

bool QgsAppLayerTreeViewMenuProvider::removeLegendLayerAction( QAction *action )
{
  QMap< QgsMapLayerType, QList< LegendLayerAction > >::iterator it;
  for ( it = mLegendLayerActionMap.begin();
        it != mLegendLayerActionMap.end(); ++it )
  {
    for ( int i = 0; i < it->count(); i++ )
    {
      if ( ( *it )[i].action == action )
      {
        ( *it ).removeAt( i );
        return true;
      }
    }
  }
  return false;
}

void QgsAppLayerTreeViewMenuProvider::addLegendLayerActionForLayer( QAction *action, QgsMapLayer *layer )
{
  if ( !action || !layer )
    return;

  legendLayerActions( layer->type() );
  if ( !mLegendLayerActionMap.contains( layer->type() ) )
    return;

  QMap< QgsMapLayerType, QList< LegendLayerAction > >::iterator it
    = mLegendLayerActionMap.find( layer->type() );
  for ( int i = 0; i < it->count(); i++ )
  {
    if ( ( *it )[i].action == action )
    {
      ( *it )[i].layers.append( layer );
      return;
    }
  }
}

void QgsAppLayerTreeViewMenuProvider::removeLegendLayerActionsForLayer( QgsMapLayer *layer )
{
  if ( ! layer || ! mLegendLayerActionMap.contains( layer->type() ) )
    return;

  QMap< QgsMapLayerType, QList< LegendLayerAction > >::iterator it
    = mLegendLayerActionMap.find( layer->type() );
  for ( int i = 0; i < it->count(); i++ )
  {
    ( *it )[i].layers.removeAll( layer );
  }
}

QList< LegendLayerAction > QgsAppLayerTreeViewMenuProvider::legendLayerActions( QgsMapLayerType type ) const
{
#ifdef QGISDEBUG
  if ( mLegendLayerActionMap.contains( type ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "legendLayerActions for layers of type %1:" ).arg( static_cast<int>( type ) ), 2 );

    const auto legendLayerActions { mLegendLayerActionMap.value( type ) };
    for ( const LegendLayerAction &lyrAction : legendLayerActions )
    {
      Q_UNUSED( lyrAction )
      QgsDebugMsgLevel( QStringLiteral( "%1/%2 - %3 layers" ).arg( lyrAction.menu, lyrAction.action->text() ).arg( lyrAction.layers.count() ), 2 );
    }
  }
#endif

  return mLegendLayerActionMap.contains( type ) ? mLegendLayerActionMap.value( type ) : QList< LegendLayerAction >();
}

void QgsAppLayerTreeViewMenuProvider::addCustomLayerActions( QMenu *menu, QgsMapLayer *layer )
{
  if ( !layer )
    return;

  // add custom layer actions - should this go at end?
  QList< LegendLayerAction > lyrActions = legendLayerActions( layer->type() );

  if ( ! lyrActions.isEmpty() )
  {
    menu->addSeparator();
    QList<QMenu *> menus;
    for ( int i = 0; i < lyrActions.count(); i++ )
    {
      if ( lyrActions[i].allLayers || lyrActions[i].layers.contains( layer ) )
      {
        if ( lyrActions[i].menu.isEmpty() )
        {
          menu->addAction( lyrActions[i].action );
        }
        else
        {
          // find or create menu for given menu name
          // adapted from DGisApp::getPluginMenu( QString menuName )
          QString menuName = lyrActions[i].menu;
#ifdef Q_OS_MAC
          // Mac doesn't have '&' keyboard shortcuts.
          menuName.remove( QChar( '&' ) );
#endif
          QAction *before = nullptr;
          QMenu *newMenu = nullptr;
          QString dst = menuName;
          dst.remove( QChar( '&' ) );
          const auto constMenus = menus;
          for ( QMenu *menu : constMenus )
          {
            QString src = menu->title();
            src.remove( QChar( '&' ) );
            int comp = dst.localeAwareCompare( src );
            if ( comp < 0 )
            {
              // Add item before this one
              before = menu->menuAction();
              break;
            }
            else if ( comp == 0 )
            {
              // Plugin menu item already exists
              newMenu = menu;
              break;
            }
          }
          if ( ! newMenu )
          {
            // It doesn't exist, so create
            newMenu = new QMenu( menuName );
            menus.append( newMenu );
            // Where to put it? - we worked that out above...
            menu->insertMenu( before, newMenu );
          }
          // QMenu* menu = getMenu( lyrActions[i].menu, &beforeSep, &afterSep, &menu );
          newMenu->addAction( lyrActions[i].action );
        }
      }
    }
    menu->addSeparator();
  }
}

void QgsAppLayerTreeViewMenuProvider::editVectorSymbol( const QString &layerId )
{
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  if ( !singleRenderer )
    return;

  std::unique_ptr< QgsSymbol > symbol( singleRenderer->symbol() ? singleRenderer->symbol()->clone() : nullptr );
  QgsSymbolSelectorDialog dlg( symbol.get(), QgsStyle::defaultStyle(), layer, mView->window() );
  dlg.setWindowTitle( tr( "Symbol Selector" ) );
  QgsSymbolWidgetContext context;
  context.setMapCanvas( mCanvas );
  context.setMessageBar( DGisApp::instance()->messageBar() );
  dlg.setContext( context );
  if ( dlg.exec() )
  {
    singleRenderer->setSymbol( symbol.release() );
    layer->triggerRepaint();
    mView->refreshLayerSymbology( layer->id() );
    layer->emitStyleChanged();
    QgsProject::instance()->setDirty( true );
  }
}

void QgsAppLayerTreeViewMenuProvider::copyVectorSymbol( const QString &layerId )
{
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  if ( !singleRenderer )
    return;

  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::symbolToMimeData( singleRenderer->symbol() ) );
}

void QgsAppLayerTreeViewMenuProvider::pasteVectorSymbol( const QString &layerId )
{
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  if ( !singleRenderer )
    return;

  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( !tempSymbol )
    return;

  if ( !singleRenderer->symbol() || singleRenderer->symbol()->type() != tempSymbol->type() )
    return;

  singleRenderer->setSymbol( tempSymbol.release() );
  layer->triggerRepaint();
  layer->emitStyleChanged();
  mView->refreshLayerSymbology( layer->id() );
  QgsProject::instance()->setDirty( true );
}

void QgsAppLayerTreeViewMenuProvider::setVectorSymbolColor( const QColor &color )
{
  QAction *action = qobject_cast< QAction *>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
  QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
  if ( !layer )
    return;

  QgsSingleSymbolRenderer *singleRenderer = dynamic_cast< QgsSingleSymbolRenderer * >( layer->renderer() );
  QgsSymbol *newSymbol = nullptr;

  if ( singleRenderer && singleRenderer->symbol() )
    newSymbol = singleRenderer->symbol()->clone();

  const QgsSingleSymbolRenderer *embeddedRenderer = nullptr;
  if ( !newSymbol && layer->renderer()->embeddedRenderer() )
  {
    embeddedRenderer = dynamic_cast< const QgsSingleSymbolRenderer * >( layer->renderer()->embeddedRenderer() );
    if ( embeddedRenderer && embeddedRenderer->symbol() )
      newSymbol = embeddedRenderer->symbol()->clone();
  }

  if ( !newSymbol )
    return;

  newSymbol->setColor( color );
  if ( singleRenderer )
  {
    singleRenderer->setSymbol( newSymbol );
  }
  else if ( embeddedRenderer )
  {
    QgsSingleSymbolRenderer *newRenderer = embeddedRenderer->clone();
    newRenderer->setSymbol( newSymbol );
    layer->renderer()->setEmbeddedRenderer( newRenderer );
  }

  layer->triggerRepaint();
  layer->emitStyleChanged();
  mView->refreshLayerSymbology( layer->id() );
  QgsProject::instance()->setDirty( true );
}

void QgsAppLayerTreeViewMenuProvider::editSymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey )
{
  QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
  {
    DGisApp::instance()->messageBar()->pushWarning( tr( "No Symbol" ), tr( "There is no symbol associated with the rule." ) );
    return;
  }

  std::unique_ptr< QgsSymbol > symbol( originalSymbol->clone() );
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( node->layerNode()->layer() );
  QgsSymbolSelectorDialog dlg( symbol.get(), QgsStyle::defaultStyle(), vlayer, mView->window() );
  dlg.setWindowTitle( tr( "Symbol Selector" ) );
  QgsSymbolWidgetContext context;
  context.setMapCanvas( mCanvas );
  context.setMessageBar( DGisApp::instance()->messageBar() );
  dlg.setContext( context );
  if ( dlg.exec() )
  {
    node->setSymbol( symbol.release() );
    if ( vlayer )
    {
      vlayer->emitStyleChanged();
    }
    QgsProject::instance()->setDirty( true );
  }
}

void QgsAppLayerTreeViewMenuProvider::copySymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey )
{
  QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

  QApplication::clipboard()->setMimeData( QgsSymbolLayerUtils::symbolToMimeData( originalSymbol ) );
}

void QgsAppLayerTreeViewMenuProvider::pasteSymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey )
{
  QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( node->layerNode()->layer() );

  std::unique_ptr< QgsSymbol > tempSymbol( QgsSymbolLayerUtils::symbolFromMimeData( QApplication::clipboard()->mimeData() ) );
  if ( tempSymbol && tempSymbol->type() == originalSymbol->type() )
  {
    node->setSymbol( tempSymbol.release() );
    if ( vlayer )
    {
      vlayer->emitStyleChanged();
    }
    QgsProject::instance()->setDirty( true );
  }
}

void QgsAppLayerTreeViewMenuProvider::setSymbolLegendNodeColor( const QColor &color )
{
  QAction *action = qobject_cast< QAction *>( sender() );
  if ( !action )
    return;

  QString layerId = action->property( "layerId" ).toString();
  QString ruleKey = action->property( "ruleKey" ).toString();

  QgsSymbolLegendNode *node = qobject_cast<QgsSymbolLegendNode *>( mView->layerTreeModel()->findLegendNode( layerId, ruleKey ) );
  if ( !node )
    return;

  const QgsSymbol *originalSymbol = node->symbol();
  if ( !originalSymbol )
    return;

  std::unique_ptr< QgsSymbol > newSymbol( originalSymbol->clone() );
  newSymbol->setColor( color );
  node->setSymbol( newSymbol.release() );
  if ( QgsVectorLayer *layer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId ) )
  {
    layer->emitStyleChanged();
  }
  QgsProject::instance()->setDirty( true );
}

bool QgsAppLayerTreeViewMenuProvider::removeActionEnabled()
{
  const QList<QgsLayerTreeLayer *> selectedLayers = mView->selectedLayerNodes();
  for ( QgsLayerTreeLayer *nodeLayer : selectedLayers )
  {
    // be careful with the logic here -- if nodeLayer->layer() is false, will still must return true
    // to allow the broken layer to be removed from the project
    if ( nodeLayer->layer() && !nodeLayer->layer()->flags().testFlag( QgsMapLayer::Removable ) )
      return false;
  }
  return true;
}

void QgsAppLayerTreeViewMenuProvider::setLayerCrs( const QgsCoordinateReferenceSystem &crs )
{
  const auto constSelectedNodes = mView->selectedNodes();
  for ( QgsLayerTreeNode *node : constSelectedNodes )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
      if ( nodeLayer->layer() )
      {
        nodeLayer->layer()->setCrs( crs, true );
        nodeLayer->layer()->triggerRepaint();
      }
    }
  }
}
