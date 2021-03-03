#pragma once

#include <windows.h>

#include <exception>
#include <stdexcept>


namespace console_helpers
{



class SimpleHandleCtrlC
{

    inline
    volatile
    static
    bool 
    breaked = false;

    static
    BOOL WINAPI consoleCtrlHandler( DWORD dwCtrlType )
    {
        if (dwCtrlType == CTRL_C_EVENT)
        {
            breaked = true;
        }
       
        return FALSE;
    }


public:

    SimpleHandleCtrlC()
    {
        if (!SetConsoleCtrlHandler( &consoleCtrlHandler, TRUE))
            throw std::runtime_error("SimpleHandleCtrlC: failed to install Ctrl+C handler");
    }

    ~SimpleHandleCtrlC()
    {
        SetConsoleCtrlHandler( &consoleCtrlHandler, FALSE);
    }

    bool isBreaked() const
    {
        // #if defined(IOA_REAL_CTRL_C_HANDLER)
        return breaked;
        // #else
        // return false;
        // #endif
    }


};





} // namespace console_helpers



#if 0
BOOL WINAPI CtrlC_HandlerRoutine( DWORD dwCtrlType );


 
volatile bool running = true;
 
BOOL WINAPI CtrlC_HandlerRoutine( DWORD dwCtrlType )
{
    if (dwCtrlType == CTRL_C_EVENT)
    {
        running = false;
        //return TRUE;
    }
 
    return FALSE;
}
 

    if (!SetConsoleCtrlHandler( &CtrlC_HandlerRoutine, TRUE))
    {
        cout << "ERROR: Could not set control handler" << endl; 
        return 1;
    }


SetConsoleCtrlHandler( &CtrlC_HandlerRoutine, FALSE);

#endif

