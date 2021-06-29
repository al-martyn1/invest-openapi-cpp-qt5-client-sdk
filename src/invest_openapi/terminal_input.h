#pragma once


#include <vector>


#if defined(WIN32) || defined(_WIN32)

    #include <windows.h>

#endif


namespace invest_openapi
{


class SimpleTerminalInput
{

    #if defined(WIN32) || defined(_WIN32)

        HANDLE hStdin;

    #endif


public:

    SimpleTerminalInput()
    {
        #if defined(WIN32) || defined(_WIN32)

            hStdin = GetStdHandle(STD_INPUT_HANDLE);

        #endif
    }


    //! Non-blocking reading of keys pressed
    std::vector<int> readInput() const
    {
        #if defined(WIN32) || defined(_WIN32)

            // https://docs.microsoft.com/en-us/windows/console/readconsoleinput
            // https://docs.microsoft.com/en-us/windows/console/input-record-str
            // https://docs.microsoft.com/en-us/windows/console/reading-input-buffer-events

            if (hStdin == INVALID_HANDLE_VALUE)
                return std::vector<int>();

            DWORD fdwSaveOldMode;
            if (!GetConsoleMode(hStdin, &fdwSaveOldMode) )
                return std::vector<int>();

            if (!SetConsoleMode(hStdin, ENABLE_WINDOW_INPUT  /* | ENABLE_MOUSE_INPUT */  /* fdwMode */ ) )
                return std::vector<int>();

            DWORD waitRes = WaitForSingleObject( hStdin, 0 );
            if (waitRes!=WAIT_OBJECT_0)
                return std::vector<int>(); // No events available on console

            INPUT_RECORD irInBuf[32];
            DWORD cNumRead;

            if (!ReadConsoleInput( hStdin, &irInBuf[0], 32, &cNumRead ))
                return std::vector<int>();

            SetConsoleMode(hStdin, fdwSaveOldMode);

            std::vector<int> resVec;

            for(auto i=0u; i!=32u; ++i)
            {
                switch(irInBuf[i].EventType)
                {
                    case KEY_EVENT:
                    {
                        if (irInBuf[i].Event.KeyEvent.bKeyDown && irInBuf[i].Event.KeyEvent.uChar.AsciiChar!=0) // Also skip non-alpha keys
                        {
                            resVec.insert( resVec.end(), irInBuf[i].Event.KeyEvent.wRepeatCount, irInBuf[i].Event.KeyEvent.uChar.AsciiChar);
                        }
                    }
                }
            }

            return resVec;

        #else

            return std::vector<int>();

        #endif
    }

}; // class SimpleTerminalInput





} // namespace invest_openapi

//----------------------------------------------------------------------------
