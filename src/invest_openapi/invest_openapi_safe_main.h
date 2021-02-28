/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
//#include <QtConcurrent/QtConcurrent>

#include <iostream>
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

    try
    {
        return safeMain(argc, argv);
    }
    catch( const std::runtime_error &e )
    {
        cout<<"Error: runtime_error: "<<e.what()<<endl;
        return 1;
    }
    catch( const std::exception &e )
    {
        cout<<"Error: exception: "<<e.what()<<endl;
        return 2;
    }
    catch( ... )
    {
        cout<<"Error: unknown error"<<endl;
        return 3;
    }

    return 0;
}


} // namespace invest_openapi

