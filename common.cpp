#include "common.h"
#include <QMutex>
#include <QWaitCondition>

namespace glb {

/* глобальный флаг завершения работы ПО */
bool rqexit;

/* контроль события завершения работы ПО */
QMutex mtxexit;
QWaitCondition cvexit;

bool waitQuitFor (int ms)
{
    QMutexLocker lck(&mtxexit);
    return isNeedExit() || (ms >=0 && cvexit.wait(&mtxexit, ms));
}

bool isNeedExit()
{
    return rqexit;
}

void globalExit()
{
    rqexit = true;
    cvexit.wakeAll();
}

QString hex(uchar bt)
{
    return QString("%1").arg(bt,2,16,QChar('0')).toUpper();
}

QString hex(const void* const ptr, size_t len, char spacer)
{
    const char * const data = (char*) ptr;
    QString tmp;
    for (size_t i = 0; i < len; i++)
    {
        tmp += hex(data[i]) + spacer;
    }
    return tmp.trimmed();
}

QString hex(const QByteArray & array, char spacer)
{
    return hex(array.constData(), array.length(), spacer);
}

QString hex(const QString& str, char spacer)
{
    return hex(str.toLocal8Bit(), spacer);
}

QString bin(uchar bt)
{
    QString tmp;
    for (int i = 0; i < 8; i++)
        tmp += (bt & (0x80 >> i)) ? "1" : "0";
    return tmp;
}

QString bin(const void* const ptr, size_t len, char spacer)
{
    const char * const data = (char*) ptr;
    QString tmp;
    for (size_t i = 0; i < len; i++)
    {
        tmp += bin(data[i]) + spacer;
    }
    return tmp.trimmed();
}

QString bin(const QByteArray& array, char spacer)
{
    return bin(array.data(), array.length(), spacer);
}
}
