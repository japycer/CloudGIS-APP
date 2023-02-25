#include "DMapToolDrawSample.h"
#include"qgsvectorlayerutils.h"
#include"qgsjsonutils.h"
#include "DPyGeeUtils.h"
#include "DGeeObjectManager.h"
#include <QMessageBox>

DMapToolDrawSample::DMapToolDrawSample(QgsMapCanvas* pMapCanvas) :
	QgsMapToolEdit(pMapCanvas)
{
	//QgsCoordinateReferenceSystem crs_src(QgsProject::instance()->crs());
	QgsCoordinateReferenceSystem crs_src("EPSG:3857");
	QgsCoordinateReferenceSystem dist_src("EPSG:4326");		//description >> "WGS 84"
	mTc = QgsCoordinateTransform(crs_src, dist_src, QgsProject::instance());

	mRubberBands = new QMap<QString, QList<DSample *>*>();
	mCurrentDrawColor = QColor("#ff0000");

	for (int i = 20; i > 0; i--)
		mClassIdStore.append(i);
}


// 画布鼠标移动事件
void DMapToolDrawSample::canvasMoveEvent(QgsMapMouseEvent * e)
{
	if (!mpRubberBand || !mIsDrawing)
		return;
	mpRubberBand->movePoint(e->mapPoint());
}


// 画布鼠标双击事件
void DMapToolDrawSample::canvasDoubleClickEvent(QgsMapMouseEvent *e)
{
	clearRubberBand();
	// 停止绘制
	mIsDrawing = false;
}


// 画布鼠标按下事件
void DMapToolDrawSample::canvasPressEvent(QgsMapMouseEvent * e)
{
	// 若当前 RubberBand 为空，则创建
	if (!mpRubberBand)
	{
		if (mDrawColors.count() <= 0) return;
		mpRubberBand = createRubberBand(QgsWkbTypes::PolygonGeometry);
		mpRubberBand->setColor(mCurrentDrawColor);
	}

	// 左键按下
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		mIsDrawing = true;
		mpRubberBand->addPoint(e->mapPoint());
	}
	// 右键按下
	else if (e->button() == Qt::MouseButton::RightButton)
	{
		if (mpRubberBand->numberOfVertices() >= 3)
		{
			if (mDrawColors.count() <= 0) return;
			DSample * sample = new DSample();
			sample->classID = mClassIds.value(currentClassName);
			int lastSampleId = -1;
			if (mRubberBands->value(currentClassName)->count() <= 0)
				lastSampleId = sample->classID * 1000;
			else
				lastSampleId = mRubberBands->value(currentClassName)->last()->ID;
			sample->ID = lastSampleId + 1;
			sample->pRubberhand = mpRubberBand;
			mRubberBands->value(currentClassName)->append(sample);
			mpRubberBand = nullptr;
			// 停止绘制
			mIsDrawing = false;
			mHasModified = true;
			emit succeedDrawAGeometry(lastSampleId + 1);
		}
	}
}


// 改变当前正在绘制的类别
void DMapToolDrawSample::changeCurrentDrawClass(QString &className)
{
	currentClassName = className;
	mCurrentDrawColor = mDrawColors.value(className);
}

// 添加一个新类别
void DMapToolDrawSample::addANewClass(QString &className, QColor &color)
{
	if (mDrawColors.count() == 0)
	{
		mCurrentDrawColor = color;
		currentClassName = className;
	}
	mDrawColors.insert(className, color);
	int clsId = mClassIdStore.takeLast();
	mClassIds.insert(className, clsId);
	mRubberBands->insert(className, new QList<DSample*>());
}

// 删除一个类别
void DMapToolDrawSample::removeAClass(QString &className)
{
	QList<DSample *>* rubbers = mRubberBands->take(className);
	int clsId = rubbers->at(0)->classID;
	int nums = rubbers->count();
	for (int i = 0; i < nums; i++)
		delete rubbers->at(i)->pRubberhand;
	delete rubbers;
	mDrawColors.remove(className);
	mClassIds.remove(className);

	mClassIdStore.append(clsId);
	mHasModified = true;
}

// 删除一个geom
void DMapToolDrawSample::removeAGeom(QString &className, int sampleID)
{
	// QMap<QString, QList<DSample *>*>*
	int index = 0;
	QList<DSample *>* rubbers = mRubberBands->value(className);
	for each (DSample* sample in *rubbers)
	{
		if (sample->ID == sampleID)
			break;
		index++;
	}
	DSample *sam = rubbers->takeAt(index);
	mHasModified = true;
	delete sam->pRubberhand;
	delete sam;
}


// 改变某类别的绘制颜色
void DMapToolDrawSample::changeDrawStyle(QString &className, QColor & drawColor)
{
	mDrawColors.insert(className, drawColor);
	QList<DSample *>* rubbers = mRubberBands->value(className);
	int nums = rubbers->count();
	for (int i = 0; i < nums; i++)
		rubbers->at(i)->pRubberhand->setColor(drawColor);

	if (className.compare(currentClassName) == 0)
		mCurrentDrawColor = drawColor;
}


// 清除之前的rubber
void DMapToolDrawSample::clearRubberBand()
{
	if (!mpRubberBand)
	{
		return;
	}
	delete mpRubberBand;
	mpRubberBand = nullptr;
}


// 改变某些绘制对象的可见性
void DMapToolDrawSample::changeItemsVisible(QList<QString> & ids, bool isVisible)
{
	int clsId = ids.at(0).toInt() / 1000;
	QString clsName = mClassIds.key(clsId);

	QList<DSample*> *samples = mRubberBands->value(clsName);
	for each (QString idStr in ids)
	{
		int id = idStr.toInt();
		QgsRubberBand *rubber = nullptr;
		for each (DSample* pSample in *samples)
			if (pSample->ID == id)
				rubber = pSample->pRubberhand;
		if (rubber)
			rubber->setVisible(isVisible);
	}
}

// 改变某个绘制对象的可见性
void DMapToolDrawSample::changeItemVisible(QString& id, bool isVisible)
{
	int clsId = id.toInt() / 1000;
	QString clsName = mClassIds.key(clsId);

	QList<DSample*> *samples = mRubberBands->value(clsName);
	QgsRubberBand *rubber = nullptr;
	for each (DSample* pSample in *samples)
		if (pSample->ID == id.toInt())
			rubber = pSample->pRubberhand;

	if (rubber)
		rubber->setVisible(isVisible);
}

// 改变某个类所有对象的可见性
void DMapToolDrawSample::changeAllItemVisible(QString &clsName, bool isVisible)
{
	int nums = mRubberBands->value(clsName)->count();
	for (int i = 0; i < nums; i++)
		mRubberBands->value(clsName)->at(i)->pRubberhand->setVisible(isVisible);
}


// 导出样本
void DMapToolDrawSample::exportSamples(QString& exportTo, QString& exportName)
{
	if (exportTo.compare("PyObj") == 0) {
		QStringList jsonStringLst;
		QString featureTemplate("{\"type\":\"Feature\",\"properties\":{\"clsName\":\"%1\",\"clsID\":%2},\"geometry\":%3}");

		// 迭代遍历所有类
		QMapIterator<QString, QList<DSample *>*> temIterator(*mRubberBands);
		while (temIterator.hasNext()) {
			temIterator.next();
			QList<DSample *>* rubbers = temIterator.value();
			QString className = temIterator.key();
			int classID = rubbers->at(0)->classID;
			QString strClsID = QString("%1").arg(classID);

			// 遍历每个类中的所有对象
			for (int i = 0; i < rubbers->count(); i++)
			{
				DSample *temSample = rubbers->at(i);
				QgsGeometry temGeom = temSample->pRubberhand->asGeometry();
				temGeom.transform(mTc);
				
				QString strCoordinates = temGeom.asJson(6);
				QString jsonFeature = featureTemplate.arg(className, strClsID, strCoordinates);
				jsonStringLst << jsonFeature;
			}
		}
		QString prefix = QString("%1=ee.FeatureCollection([").arg(exportName.simplified());
		QString res = prefix.append(jsonStringLst.join(',')).append(QString("])"));
		bool ok = DPyGeeUtils::runString(res);
		if (ok)
			DGeeObjectManager::Instance()->addOneRecord(exportName.simplified(), DGeeObjectManager::GeeObjType::FeatureCollection, jsonStringLst.count());
		else
		{
			QString errRes = DPyGeeUtils::getLatestRunOut(false);
			QMessageBox::critical(nullptr, QString("Transfrom samples"), errRes, QMessageBox::Ok);
		}
		// emit toRunACommand(res);
		return;
	}

	if (!mCurrentExportLayer && mHasModified)
	{
		if(!mCurrentExportLayer) delete mCurrentExportLayer;
		mCurrentExportLayer = new QgsVectorLayer("MultiPolygon?crs=epsg:3857&field=clsID:integer&field=clsName:string(20)", exportName, "memory");
		if (!mCurrentExportLayer) return;
		mCurrentExportLayer->startEditing();
		
		// 迭代遍历所有类
		QMapIterator<QString, QList<DSample *>*> temIterator(*mRubberBands);
		while (temIterator.hasNext()) {
			temIterator.next();
			QList<DSample *>* rubbers = temIterator.value();
			QString className = temIterator.key();
			int classID = rubbers->at(0)->classID;

			// 遍历每个类中的所有对象
			for (int i = 0; i < rubbers->count(); i++)
			{
				DSample *temSample = rubbers->at(i);
				QgsGeometry temGeom = temSample->pRubberhand->asGeometry();
				//temGeom.transform(mTc);
				//temGeom.convertToSingleType();
				QgsFeature feature = QgsVectorLayerUtils::createFeature(mCurrentExportLayer, temGeom);
				bool res1 = feature.setAttribute("clsName", className);
				bool res2 = feature.setAttribute("clsID", classID);
				mCurrentExportLayer->addFeature(feature);
			}
		}
		mCurrentExportLayer->endEditCommand();
		if (!mCurrentExportLayer->commitChanges())
			return;
	}

	mHasModified = false;
	//"PyObj" << "Shp" << "VLayer"
	if (exportTo.compare("VLayer") == 0) {
		QgsProject::instance()->addMapLayer(mCurrentExportLayer->clone());
		return;
	}

	if (exportTo.compare("Shp") == 0) {
		//QgsProject::instance()->addMapLayer(mCurrentExportLayer);
		emit toSaveAsFile(mCurrentExportLayer);
		return;
	}
}