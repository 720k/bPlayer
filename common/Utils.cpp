#include "Utils.h"

#include <QDebug>
#include <chrono>

#ifdef Q_OS_LINUX
    #include <unistd.h>
    #include <sys/time.h>
    #include <sys/syscall.h>
    #include <sys/types.h>
    #include <QDir>
    #include <QFileInfo>
    #include <QTextStream>
#endif
#ifdef Q_OS_WIN
    //#include <sysinfoapi.h>
    #include <windows.h>
#endif

namespace Utils {

quint64 getMicroSeconds() {
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

QString printableBA(const QByteArray &ba, const QString &prefix)    {
    auto b = ba.left(16);
    return QString("%1[%2] %3").arg(prefix).arg(ba.size(),8).arg(QString(b.toHex(' ')));
}

QString printableRawBA(const QByteArray &ba)    {
    auto b = ba.left(16);
    return QString(b.toHex(' '));
}

QString printableNumber(double no) {
    return QString::number(no,'f',3);
}

QString formatTime(quint64 seconds) {
    std::chrono::seconds secs(seconds);
    int hh = std::chrono::duration_cast<std::chrono::hours>(secs).count();
    int mm = std::chrono::duration_cast<std::chrono::minutes>(secs).count();
    int ss = seconds % 60;
    return QString("%1:%2.%3").arg(hh,2,10,QChar('0')).arg(mm,2,10,QChar('0')).arg(ss,2,10,QChar('0'));
}

#ifdef Q_OS_LINUX
QString threadName(QString text) {
    pid_t tid = static_cast<pid_t>(syscall (SYS_gettid));
    return QString("Thread [%2]:%1").arg(text).arg(tid);
}

QString     readContent(const QString& fileName) {
    QFile textfile(fileName);
    QString result;
    if (!textfile.open(QIODevice::ReadOnly)) return result;
    QTextStream text(&textfile);
    result = text.readAll();
    textfile.close();
    return result;
}

QMap<QString,QString>   extractProperties(const QString& text, QChar separator = QChar(':')) {
    QMap<QString,QString> result;
    for (const QString& line : text.split(QChar('\n')) ) {
        auto name = line.section(separator,0,0).trimmed();
        auto value = line.section(separator,1,1).trimmed();
        result[name]=value;
    }
    return result;
}

qint64          getProcessID(const QString& processName) {
    QDir procDir("/proc");
    for (const QString& dirName : procDir.entryList(QDir::AllDirs)) {
        bool isPid;
        int candidatePID = dirName.toInt(&isPid);
        if (isPid) {
            auto content = readContent( QString("/proc/%1/status").arg(candidatePID) );
            auto properties = extractProperties(content);
            if (properties.value("Name") == processName) {
                return candidatePID;
            }
        }
    }
    return -1;
}

QString portNameFromProcess(const QString &portName, const QString &ProcessName) {
    int pid = Utils::getProcessID(ProcessName);
    return pid>0 ? QString("/tmp/%1-%2/%3").arg(ProcessName).arg(pid).arg(portName) : "";
}

#endif

#ifdef Q_OS_WIN

QString threadName(QString text) {          Q_UNUSED(text)
    return QString("UNSUPPORTED");
}

qint64 getProcessID(const QString& processName) {       Q_UNUSED(processName)
    return -1;
}

void PrintLastErrorString() {
    DWORD error = GetLastError();
    if (error)  {
        LPVOID lpMsgBuf;
        DWORD bufLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
        if (bufLen) {
            //WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), lpMsgBuf, bufLen,NULL,NULL);
            qDebug() << "[ERROR] " << QString::fromWCharArray((LPCWSTR)lpMsgBuf);
            LocalFree(lpMsgBuf);
        }
    }
}

#endif
} // Namespace: Utils
