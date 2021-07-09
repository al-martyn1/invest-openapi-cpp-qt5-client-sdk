/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QtGlobal>
//#include <QtConcurrent/QtConcurrent>

#include <iostream>
#include <sstream>
#include <exception>
#include <stdexcept>



int safeMain(int argc, char* argv[]);


#define INVEST_OPENAPI_MAIN()                                     \
               int main(int argc, char* argv[])                   \
               {                                                  \
                   return invest_openapi::mainImpl(argc, argv);   \
               }                                                  \
                                                                  \
               int safeMain(int argc, char* argv[])



namespace invest_openapi
{



inline
int mainImpl(int argc, char* argv[])
{
    using std::cout;
    using std::endl;

    std::ostringstream oss;

    try
    {
        int res = safeMain(argc, argv);
        return res;
    }
    catch( const std::runtime_error &e )
    {
        cout << "Error: runtime_error: " << e.what() << endl;
        oss  << "Abnormal program termination: runtime_error: " << e.what();
        qFatal(oss.str().c_str());
        return 1;
    }
    catch( const std::exception &e )
    {
        cout << "Error: exception: " << e.what() << endl;
        oss  << "Abnormal program termination: exception: " << e.what();
        qFatal(oss.str().c_str());
        return 2;
    }
    catch( ... )
    {
        cout << "Error: unknown error" << endl;
        oss  << "Abnormal program termination: unknown error";
        qFatal(oss.str().c_str());
        return 3;
    }

    return 0;
}


} // namespace invest_openapi

