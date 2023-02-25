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


// ��������ƶ��¼�
void DMapToolDrawSample::canvasMoveEvent(QgsMapMouseEvent * e)
{
	if (!mpRubberBand || !mIsDrawing)
		return;
	mpRubberBand->movePoint(e->mapPoint());
}


// �������˫���¼�
void DMapToolDrawSample::canvasDoubleClickEvent(QgsMapMouseEvent *e)
{
	clearRubberBand();
	// ֹͣ����
	mIsDrawing = false;
}


// ������갴���¼�
void DMapToolDrawSample::canvasPressEvent(QgsMapMouseEvent * e)
{
	// ����ǰ RubberBand Ϊ�գ��򴴽�
	if (!mpRubberBand)
	{
		if (mDrawColors.count() <= 0) return;
		mpRubberBand = createRubberBand(QgsWkbTypes::PolygonGeometry);
		mpRubberBand->setColor(mCurrentDrawColor);
	}

	// �������
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		mIsDrawing = true;
		mpRubberBand->addPoint(e->mapPoint());
	}
	// �Ҽ�����
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
			// ֹͣ����
			mIsDrawing = false;
			mHasModified = true;
			emit succeedDrawAGeometry(lastSampleId + 1);
		}
	}
}


// �ı䵱ǰ���ڻ��Ƶ����
void DMapToolDrawSample::changeCurrentDrawClass(QString &className)
{
	currentClassName = className;
	mCurrentDrawColor = mDrawColors.value(className);
}

// ���һ�������
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

// ɾ��һ�����
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

// ɾ��һ��geom
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


// �ı�ĳ���Ļ�����ɫ
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


// ���֮ǰ��rubber
void DMapToolDrawSample::clearRubberBand()
{
	if (!mpRubberBand)
	{
		return;
	}
	delete mpRubberBand;
	mpRubberBand = nullptr;
}


// �ı�ĳЩ���ƶ���Ŀɼ���
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

// �ı�ĳ�����ƶ���Ŀɼ���
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

// �ı�ĳ�������ж���Ŀɼ���
void DMapToolDrawSample::changeAllItemVisible(QString &clsName, bool isVisible)
{
	int nums = mRubberBands->value(clsName)->count();
	for (int i = 0; i < nums; i++)
		mRubberBands->value(clsName)->at(i)->pRubberhand->setVisible(isVisible);
}


// ��������
void DMapToolDrawSample::exportSamples(QString& exportTo, QString& exportName)
{
	if (exportTo.compare("PyObj") == 0) {
		QStringList jsonStringLst;
		QString featureTemplate("{\"type\":\"Feature\",\"properties\":{\"clsName\":\"%1\",\"clsID\":%2},\"geometry\":%3}");

		// ��������������
		QMapIterator<QString, QList<DSample *>*> temIterator(*mRubberBands);
		while (temIterator.hasNext()) {
			temIterator.next();
			QList<DSample *>* rubbers = temIterator.value();
			QString className = temIterator.key();
			int classID = rubbers->at(0)->classID;
			QString strClsID = QString("%1").arg(classID);

			// ����ÿ�����е����ж���
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
		
		// ��������������
		QMapIterator<QString, QList<DSample *>*> temIterator(*mRubberBands);
		while (temIterator.hasNext()) {
			temIterator.next();
			QList<DSample *>* rubbers = temIterator.value();
			QString className = temIterator.key();
			int classID = rubbers->at(0)->classID;

			// ����ÿ�����е����ж���
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