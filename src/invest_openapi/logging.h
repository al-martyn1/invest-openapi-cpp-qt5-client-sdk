#pragma once

#include <QString>
#include <QSettings>
#include <QCoreApplication>
#include <QtGlobal>
#include <QFile>

#include <exception>
#include <stdexcept>
#include <cstdio>
#include <iostream>

#include "qt_filename_helpers.h"
#include "logging_config.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
// logging::impl::getCurrentLogHandlers()
// logging::impl::getCurrentLogDateTimeFormat()
// logging::impl::getCurrentLogLevel()

namespace logging {
namespace impl {



//----------------------------------------------------------------------------
inline
std::map<QString, QString>& getCurrentLogHandlers()
{
    static  std::map<QString, QString>  handlers;
    return handlers;
}

inline
QString& getCurrentLogDateTimeFormat()
{
    static QString logDateTimeFormat;
    return logDateTimeFormat;
}

inline
int& getCurrentLogLevel()
{
    static int logLevel = 7; // All by default
    return logLevel;
}

inline
bool& getCurrentLogContextValue()
{
    static bool logContext = false;
    return logContext;
}

//----------------------------------------------------------------------------

/*
    https://en.wikipedia.org/wiki/Syslog#Severity_level

        0 Emergency
        1 Alert
        2 Critical
        3 Error
        4 Warning
        5 Notice
        6 Informational
        7 Debug
        
    and same
        
        0 (KERN_EMERG) The system is unusable.
        1 (KERN_ALERT) Actions that must be taken care of immediately.
        2 (KERN_CRIT) Critical conditions.
        3 (KERN_ERR) Non-critical error conditions.
        4 (KERN_WARNING) Warning conditions that should be taken care of.
        5 (KERN_NOTICE) Normal, but significant events.
        6 (KERN_INFO) Informational messages that require no action.
        7 (KERN_DEBUG) Kernel debugging messages, output by the kernel if the developer enabled debugging at compile time.
*/

inline
void logMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static const char* levelNames[9] = { "EMERG " // "FATAL" - 0
                                       , "ALERT " // 1 - not used
                                       , "CRIT  " // 2
                                       , "ERROR " // 3 - not used
                                       , "WARN  " // 4
                                       , "NOTICE" // 5 - not used
                                       , "INFO  " // 6
                                       , "DEBUG " // 7
                                       };
    
    int msgLogLevel = 0;

    switch (type)
    {
        case QtFatalMsg   : msgLogLevel = 0; break;
        // No alert level : msgLogLevel = 1;
        case QtCriticalMsg: msgLogLevel = 2; break;
        // No error level : msgLogLevel = 3;
        case QtWarningMsg : msgLogLevel = 4; break;
        // No notice level: msgLogLevel = 5;
        case QtInfoMsg    : msgLogLevel = 6; break;
        case QtDebugMsg   : msgLogLevel = 7; break;

        default           : msgLogLevel = 7;
    }

    if (msgLogLevel>getCurrentLogLevel())
        return;


    QDateTime      dtNow    = QDateTime::currentDateTime();
    const QString& dtFormat = getCurrentLogDateTimeFormat();

    QString dtString;

    if (!dtFormat.isEmpty())
    {
        dtString = QString("[") + dtNow.toString(dtFormat) + QString("] ");
    }

    QString llString = QString("[") + QString(levelNames[msgLogLevel]) + QString("] ");


    QString contextString;

    if (getCurrentLogContextValue())
    {
        if (context.file)
            contextString = context.file;
       
        if (context.function)
        {
            if (!contextString.isEmpty())
                contextString += ":";
            contextString += context.function;
        }

        if (!contextString.isEmpty())
        {
            contextString = QString("[") + contextString + QString("] ");
        }
    }


    std::string logInfo = (dtString+llString+contextString).toStdString();


    const std::map<QString, QString> &logHandlers = getCurrentLogHandlers();

    std::map<QString, QString>::const_iterator lit = logHandlers.begin();

    for(; lit!=logHandlers.end(); ++lit)
    {
        if (lit->first=="cout" || lit->first=="cerr")
        {
            auto qbaMsg = msg.toLocal8Bit();
            auto msgRaw = qbaMsg.toStdString();

            std::string fullMsg = logInfo + msgRaw;

            if (lit->first=="cout")
            {
                std::cout << fullMsg << std::endl;
            }
            else
            {
                std::cerr << fullMsg << std::endl;
            }
        }
        else if (lit->first=="file")
        {
            auto utfMsg = msg.toUtf8();
            auto msgRaw = utfMsg.toStdString();
            std::string fullMsg = logInfo + msgRaw + std::string("\n");

            QString logFileName = lit->second;

            QFile qfLog( logFileName );

            // https://doc.qt.io/qt-5/qiodevice.html
            // https://doc.qt.io/qt-5/qiodevice.html#OpenModeFlag-enum

            bool openGood = qfLog.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
            if (openGood)
            {
                qfLog.write( fullMsg.data(), fullMsg.size() );
                qfLog.close();
            }

        } // add more log handlers here
    
    } // for(; lit!=logHandlers.end(); ++lit)

} // logMessageOutput

} // namespace impl
} // namespace logging

//----------------------------------------------------------------------------



inline
void initLogging( const std::map<QString, QString> &handlers
                , const QString                    &logDateTimeFormat
                , int                               logLevel
                , bool                              logWithContext = false
                )
{
    if (handlers.empty())
        return;

    qInstallMessageHandler(0); // Reset logger function - if our function already installed, we also disable it to prevent the concurent access to settings

    QTest::qWait(10);

    logging::impl::getCurrentLogHandlers()       = handlers;
    logging::impl::getCurrentLogDateTimeFormat() = logDateTimeFormat;
    logging::impl::getCurrentLogLevel()          = logLevel;
    logging::impl::getCurrentLogContextValue()   = logWithContext;

    qInstallMessageHandler(&logging::impl::logMessageOutput);

}

/*
logLevel
*/

//std::map<QString, QString> getLogHandlers( QString configFullName ) const


/*
case QtDebugMsg:        // 4
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:     // 3
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:  // 2
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg: // 1
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:    // 0
*/

//----------------------------------------------------------------------------



} // namespace invest_openapi


