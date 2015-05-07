#include <QMessageBox>
#include <QStringList>
#include "parserxml.h"

QString TParserXML::strVersion = "v1.2";
// construction
TParserXML::TParserXML(QObject *parent)
        : QObject(parent)
{
}
//distruction
TParserXML::~TParserXML()
{
}
// xml
QDomElement TParserXML::makeEliment(QDomDocument& domDoc,const QString& strName,const QString& strAttr="",const QString& strText="")
{
    QDomElement domElement=domDoc.createElement(strName);
    if(!strAttr.isEmpty()){
        QDomAttr domAttr = domDoc.createAttribute("number");
        domAttr.setValue(strAttr);
        domElement.setAttributeNode(domAttr);
        }
    if(!strText.isEmpty()){
        QDomText domText =domDoc.createTextNode(strText);
        domElement.appendChild(domText);
        }

    return domElement;
}
// xml data
QDomElement TParserXML::makeDeviceTIME(QDomDocument& domDoc, int nNumber,
                                       const QString& strTimeRed,
                                       const QString& strTimeBlinkRed,
                                       const QString& strTimeGreen,
                                       const QString& strTimeBlinkGreen)
{
QDomElement domElement=makeEliment(domDoc,"TIME",QString().setNum(nNumber));
domElement.appendChild(makeEliment(domDoc,"secRed","",strTimeRed));
domElement.appendChild(makeEliment(domDoc,"secRedBlink","",strTimeBlinkRed));
domElement.appendChild(makeEliment(domDoc,"secGreen","",strTimeGreen));
domElement.appendChild(makeEliment(domDoc,"secGreenBlink","",strTimeBlinkGreen));
return domElement;
}
QDomElement TParserXML::makeDeviceRS485(QDomDocument& domDoc,int nNumber,
                                        const QString& strDTime,
                                        const QString& strBRate,
                                        const QString& strBSize,
                                        const QString& strBStop,
                                        const QString& strParity)
{
QDomElement domElement=makeEliment(domDoc,"RS485",QString().setNum(nNumber));
domElement.appendChild(makeEliment(domDoc,"DelayTime","",strDTime));
domElement.appendChild(makeEliment(domDoc,"BaudRate","",strBRate));
domElement.appendChild(makeEliment(domDoc,"ByteSize","",strBSize));
domElement.appendChild(makeEliment(domDoc,"StopBit","",strBStop));
domElement.appendChild(makeEliment(domDoc,"Parity","",strParity));
return domElement;
}
// save new parametr
bool TParserXML::SaveXML(const QString &fileName, const QStringList strPRM)
{
QDomDocument doc("CONFIG");
QDomElement domElement = doc.createElement("COUNTDOWN");
doc.appendChild(domElement);
// общие параметры
domElement.appendChild(makeEliment(doc,"Version","",strVersion));
// индивидуальные параметры для каждого устройства
domElement.appendChild(makeDeviceTIME(doc,1,strPRM.at(0),strPRM.at(1),strPRM.at(2),strPRM.at(3)));
domElement.appendChild(
            makeDeviceRS485(doc,1,strPRM.at(4),strPRM.at(5),strPRM.at(6),strPRM.at(7),strPRM.at(8)));
// create file device.xml
QFile file(fileName);
if(file.open(QIODevice::WriteOnly|QIODevice::Text)){
    QTextStream(&file)<<doc.toString();
    file.close();
    return true;
    }
return false;
}
//parser xml
void TParserXML::TraversNode(const QDomElement &element, QStringList &listPRM)
{
QDomElement child = element.firstChildElement();
if(element.tagName()=="TIME"){
    while(!child.isNull())
        {
        if(child.tagName()=="secRed")listPRM.append(child.text());
        if(child.tagName()=="secRedBlink")listPRM.append(child.text());
        if(child.tagName()=="secGreen") listPRM.append(child.text());
        if(child.tagName()=="secGreenBlink")listPRM.append(child.text());
        child = child.nextSiblingElement();
        }
    }
if(element.tagName()=="RS485"){
    while(!child.isNull())
        {
        if(child.tagName()=="DelayTime")listPRM.append(child.text());
        if(child.tagName()=="BaudRate")listPRM.append(child.text());
        if(child.tagName()=="ByteSize")listPRM.append(child.text());
        if(child.tagName()=="StopBit")listPRM.append(child.text());
        if(child.tagName()=="Parity")listPRM.append(child.text());
        child = child.nextSiblingElement();
        }
    }
child = element.nextSiblingElement();
if(!child.isNull())TraversNode(child,listPRM);
}
// parser xml
bool TParserXML::OpenXML(const QString &fileName,QStringList &listPRM)
{
QDomDocument doc;
QString errorStr;
int errorLine;
int errorColumn;
bool Return = false;

QFile file(fileName);
if(file.open(QIODevice::ReadOnly|QIODevice::Text)){
    if(doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn)){
        QDomElement domElelment = doc.documentElement();
        if(domElelment.tagName()!="COUNTDOWN"){
            QMessageBox::information(0, tr("Configuration File"),
                                     tr("This file does not configure the device!"));
            }else{
            QDomElement child = domElelment.firstChildElement("Version");
            if(child.text()==strVersion){ // XML file is the same version
                child = domElelment.firstChildElement("TIME");
                TraversNode(child,listPRM);
                }
            Return = true;
            }
        }else{
        QMessageBox::information(0, tr("Configuration File"),
                                 tr("Error on line %1, column %2:\n%3!")
                                 .arg(errorLine)
                                 .arg(errorColumn)
                                 .arg(errorStr));
        }
    file.close();
    }else{
    DefaultXML();
    OpenXML(fileName,listPRM);
    }
return Return;
}
void TParserXML::DefaultXML()
{
QStringList strParm;
strParm<<"1:30"<<"10"<<"1:30"<<"10"<<"150"<<"19200"<<"8"<<"0"<<"0";
SaveXML("devices.xml",strParm);
}
