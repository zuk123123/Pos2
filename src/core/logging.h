#pragma once
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logCore)
Q_DECLARE_LOGGING_CATEGORY(logNet)

#define LCORE()  qCInfo(logCore)
#define LWARN()  qCWarning(logCore)
#define LERR()   qCCritical(logCore)
#define LNET()   qCInfo(logNet)
