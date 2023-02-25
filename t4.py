def calculIndex(eeObject, functionStr, bandsLst, option, scale, roi=None):
    #global _dictA, _functionStr
    bandNames = eeObject.bandNames()
    bansStr = ["B{}".format(i) for i in bandsLst]
    dictA = {}
    for str1,num in zip(bansStr,bandsLst):
        dictA[str1] = eeObject.select(ee.String(bandNames.get(num)))
    indexImg = eeObject.expression(functionStr, dictA)
    #_dictA = dictA
    #_functionStr = functionStr
    roi = eeObject.geometry()

    statisValue = indexImg.reduceRegion(**{"reducer":ee.Reducer.max(),"geometry":roi,"scale":scale,"bestEffort":True})
    return indexImg.set("Max",statisValue)

#_dictA = ""
#_functionStr = ""
aa = calculIndex(firstImg,"(B5-B4)/(B5+B4)", [5,4], "Max",30)

lambda img:img.set(option, img.reduceRegion(**{"reducer":ee.Reducer.max(),"geometry":roi==None?img.geometry:roi,"scale":scale,"bestEffort":True}))