#ifndef PARSERXML_H
#define PARSERXML_H

#include <QMainWindow>
#include <QObject>
#include <QtXml>

class QFile;

class TParserXML : public QObject
{
Q_OBJECT

public:
    explicit TParserXML(QObject *parent = 0);
    ~TParserXML();
    bool SaveXML(const QString &fileName, const QStringList strPRM);
    bool OpenXML(const QString &fileName, QStringList &listPRM);
private:
    static QString strVersion;
    QDomElement makeEliment(QDomDocument &domDoc,const QString& strName,const QString& strAttr,const QString& strText);
    QDomElement makeDeviceTIME(QDomDocument& domDoc,int nNumber,
                                const QString& strTimeRed="1:30",
                                const QString& strTimeBlinkRed="10",
                                const QString& strTimeGreen="1:20",
                                const QString& strTimeBlinkGreen="10");
    QDomElement makeDeviceRS485(QDomDocument& domDoc,int nNumber,
                                const QString& strDTime="150",
                                const QString& strBRate="19200",
                                const QString& strBSize="8",
                                const QString& strBStop="1",
                                const QString& strParity="0");
    void TraversNode(const QDomElement &element, QStringList &listPRM);
    void DefaultXML();
    void ConfigDefault(void);
};

#endif // PARSERXML_H
