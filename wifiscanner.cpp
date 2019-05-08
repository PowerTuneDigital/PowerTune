#include "wifiscanner.h"
#include "dashboard.h"
#include "connect.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include <QByteArrayMatcher>
#include <QByteArray>

QString outputline;
QStringList result;

WifiScanner::WifiScanner(QObject *parent)
    : QObject(parent)
    , m_dashboard(Q_NULLPTR)
{
}


WifiScanner::WifiScanner(DashBoard *dashboard, QObject *parent)
    : QObject(parent)
     , m_dashboard(dashboard)
{
}


void WifiScanner::initializeWifiscanner()
{

    /*
    findTimer = new QTimer();
    findTimer->setInterval(3000);
    connect(findTimer,&QTimer::timeout,this,&WifiScanner::findActiveWirelesses);
    findTimer->start();
    foundCount = 0;
    findActiveWirelesses();*/
   // QProcess *process = new QProcess(this);
    process = new QProcess(this);  // create on the heap, so it doesn't go out of scope
    connect (process, SIGNAL(readyReadStandardOutput()), this, SLOT(readData()));  // connect process signals with your code
    connect(process, SIGNAL(finished(int , QProcess::ExitStatus )), this, SLOT(finalize(int , QProcess::ExitStatus)));
    //process->start("sudo iwlist wlan0 scan");  // start the process
    result.clear();
    process->start("sudo iw wlan0 scan | egrep 'SSID'");
    process->waitForFinished();

}

void WifiScanner::findActiveWirelesses()
{
    QNetworkConfigurationManager ncm;
    netcfgList = ncm.allConfigurations();
    wifilist.clear();
    //WiFisList.clear();
    for (auto &x : netcfgList)
    {
        if (x.bearerType() == QNetworkConfiguration::BearerWLAN)
        {
            if(x.name() == "")
                wifilist << "Unknown(Other Network)";
            else
               wifilist << x.name();

           //  qDebug() << "Type" <<x.type();
           //  qDebug() << "Network found :"<<x.name();
           //  qDebug() << "State :"<<x.state();
        }
    }
    m_dashboard->setwifi(wifilist);
    qDebug() << wifilist ;
}

void WifiScanner::readData()
{
    QString line = process->readAllStandardOutput();        // read data from serial port
    outputline.append(line);
}

void WifiScanner::finalize(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "done :";
    int i =0;
    QStringList fields = outputline.split(QRegExp("[\n]"));
    //Parse List and delete all items without SSID in them
    qDebug() << "elements :" << fields.count();
            foreach (const QString &str, fields) {
                if (str.contains("SSID"))
                {
                    QString raw;
                    raw = str;
                    raw.replace(" ", "");
                    raw.remove(QChar('"'), Qt::CaseInsensitive);
                    //raw.remove(QChar('\'), Qt::CaseInsensitive);
                    raw.remove(0,6);
                    result += raw;
                }
            }
    m_dashboard->setwifi(result);
}