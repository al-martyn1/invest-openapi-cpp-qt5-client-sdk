#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QtConcurrent/QtConcurrent>

#include <iostream>
#include <exception>
#include <stdexcept>


#if defined(_MSC_VER)
    #pragma comment(lib, "tkf_invest_oa")
#endif


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
void pollMessageQueue()
{
    QTest::qWait(0);
}



inline
int mainImpl(int argc, char* argv[])
{
    using std::cout;
    using std::endl;

    try
    {
        return safeMain(argc, argv);
    }
    catch( const std::runtime_error &e )
    {
        cout<<"Error: runtime_error: "<<e.what()<<endl;
        return -1;
    }
    catch( const std::exception &e )
    {
        cout<<"Error: exception: "<<e.what()<<endl;
        return -1;
    }
    catch( ... )
    {
        cout<<"Error: unknown error"<<endl;
        return -1;
    }

    // return 0;
}


} // namespace invest_openapi

