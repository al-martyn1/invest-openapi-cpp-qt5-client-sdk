#pragma once


#include <vector>
#include <string>


#if defined(WIN32) || defined(_WIN32)

    #include <windows.h>

#endif


namespace invest_openapi
{

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
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

            // https://docs.microsoft.com/en-us/windows/console/generateconsolectrlevent

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
                        if (irInBuf[i].Event.KeyEvent.bKeyDown)
                        {
                            bool flagCtrlPressed  = ( (irInBuf[i].Event.KeyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) != 0 );
                            bool flagAltPressed   = ( (irInBuf[i].Event.KeyEvent.dwControlKeyState & (LEFT_ALT_PRESSED |RIGHT_ALT_PRESSED )) != 0 );
                            bool flagShiftPressed = ( (irInBuf[i].Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED)) != 0 );

                            bool flagOnlyCtrlPressed  =  flagCtrlPressed && !flagAltPressed && !flagShiftPressed;
                            bool flagOnlyAltPressed   = !flagCtrlPressed &&  flagAltPressed && !flagShiftPressed;
                            bool flagOnlyShiftPressed = !flagCtrlPressed && !flagAltPressed &&  flagShiftPressed;

                            if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode==0x43 && flagOnlyCtrlPressed) // Ctrl+C pressed
                            {
                                GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
                            }
                            else if ( (irInBuf[i].Event.KeyEvent.wVirtualKeyCode==VK_INSERT && flagOnlyShiftPressed) // Shift+Ins - classic mode
                                   || (irInBuf[i].Event.KeyEvent.wVirtualKeyCode==0x56 /* V key */  && flagOnlyCtrlPressed) // Ctrl+V - modern mode
                                    )
                            {
                                // Нужно прочитать буфер обмена в текстовом режиме и скормить его содержимое в результирующий вектор
                                // Tab - заменяем на пробел, никакие другие контрольные символы, кроме Enter 0x0D не пропускаем, и 
                                // пропускаем только то, что <128

                                // https://stackoverflow.com/questions/14762456/getclipboarddatacf-text

                                if (!OpenClipboard(0))
                                    return resVec;

                                HANDLE hData = GetClipboardData(CF_TEXT);
                                if (hData==0)
                                    return resVec;

                                char * pszText = static_cast<char*>( GlobalLock(hData) );

                                if (pszText)
                                {
                                    // Do job here

                                    for(; *pszText; ++pszText)
                                    {
                                        char ch = *pszText;

                                        switch(ch)
                                        {
                                            case 0x09: // автокомплит при вставке из буфера - недопустим, поэтому просто заменяем на пробел
                                                 resVec.insert( resVec.end(), 1, ' ' );
                                                 break;

                                            case 0x0D:
                                                 resVec.insert( resVec.end(), 1, ch );
                                                 break;

                                            default:

                                                 if (ch>=' ' || ch<0x7F)
                                                    resVec.insert( resVec.end(), 1, ch );
                                        }
                                    
                                    }
                                }

                                GlobalUnlock( hData );

                                CloseClipboard();

                            }
                            else if ( irInBuf[i].Event.KeyEvent.wVirtualKeyCode==VK_SPACE /* 0x20 */ 
                                   // && flagAltPressed // Alt+Space - windows native hotkey
                                   && (flagCtrlPressed || flagShiftPressed) // Use Ctrl+Space or Shift+Space instead
                                    ) 
                            {
                                // https://ru.wikipedia.org/wiki/ASCII
                                // 0x0B - VT - vertical tab
                                // Используем как алтернативный вариант для автокомплита
                                // Основной вариант - по обычному табу - 0x09 - TAB
                                resVec.insert( resVec.end(), irInBuf[i].Event.KeyEvent.wRepeatCount, 0x0B );
                            }
                            else if (irInBuf[i].Event.KeyEvent.uChar.AsciiChar!=0)
                            {
                                resVec.insert( resVec.end(), irInBuf[i].Event.KeyEvent.wRepeatCount, irInBuf[i].Event.KeyEvent.uChar.AsciiChar);
                            }
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

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Базовый класс для простецкого редактирования строки в терминале - никакого курсора тудым-сюдым, только можно стирать Backspace'ом, 
/*! Зато автокомплит искаропки поддерживается
    Но не поддерживается ничего, кроме базового ASCII.
    Зато поддерживается автоматическое преобразование регистра вводимых символов.
    В общем, огранниченный редактор для специфических применений.
 */
class SimpleTerminalLineEdit
{
    std::string    line;
    std::string    autocompletionString;
    char           autocompletionKeyCode = 0x09; // TAB
    int            caseConvertMode       = 0; // <0 - lower, 0 - keep, >0 - upper

    //! \param text can be modified in this event handler
    virtual void onTextModified ( SimpleTerminalLineEdit *pEdit, std::string &text ) { }

    //! Return true if buffer must kept after this call (buffer can be filled with new text by the setText function or by modifying param text)
    virtual bool onEditCompleted( SimpleTerminalLineEdit *pEdit, std::string &text ) { return false; } 

public: 


    static char toUpper( char ch )
    {
        if (ch>='a' && ch<='z')
            ch = ch - 'a' + 'A';
        return ch;
    }

    static char toLower( char ch )
    {
        if (ch>='A' && ch<='Z')
            ch = ch - 'A' + 'a';
        return ch;
    }

    char caseConvert( char ch ) const
    {
        if (caseConvertMode>0)  return toUpper(ch);
        if (caseConvertMode<0)  return toLower(ch);
        return ch;
    }

    std::string caseConvert( std::string str ) const
    {
        std::string::size_type i = 0, sz = str.size();
        for(; i!=sz; ++i)
           str[i] = caseConvert(str[i]);
        return str;
    }

    void setCaseConvert( int cc ) { caseConvertMode = cc;   }
    int  getCaseConvert( ) const  { return caseConvertMode; }


    //! Return false if std, true if alter key used
    bool getAutocompletionKeyAlter() const         { return autocompletionKeyCode != 0x09; }
    void setAutocompletionKeyAlter( bool bAlter )  { autocompletionKeyCode = bAlter ? 0x0B : 0x09; }

    bool isAutocompletionKeyChar( char ch ) const  { return ch==autocompletionKeyCode; } 
    bool isBackspaceKeyChar     ( char ch ) const  { return ch==0x08; } // BS 08 backspace
    bool isEnterKeyChar         ( char ch ) const  { return ch==0x0D; } // CR 13 
    bool isAsciiKeyChar         ( char ch ) const  { return ch>=' ' && ch<0x7F; } // Basic ASCII


    std::string getText() const { return line; }
    void        setText(  const std::string &t ) { line = t; }
    void        clrText() { line.clear(); }

    // Текст автокомплита, на самом деле. Просто слишком длинно получалось get/set/clr_AutocompletionText
    std::string getAclt() const { return autocompletionString; }
    void        setAclt(  const std::string &t ) { autocompletionString = t; }
    void        clrAclt() { autocompletionString.clear(); }



    void processInput( const std::vector<int> &input )
    {
        for( auto ch : input )
        {
            if (!isAsciiKeyChar(ch))
                continue;

            if (isBackspaceKeyChar(ch))
            {
                if (!line.empty())
                {
                    line.erase( line.size()-1 );
                    onTextModified( this, line );
                }
            }
            else if (isEnterKeyChar(ch))
            {
                bool bKeep = onEditCompleted( this, line );
                if (!bKeep)
                    line.clear();
            }
            else if (isAutocompletionKeyChar(ch))
            {
                if (!autocompletionString.empty())
                {
                    line.append(autocompletionString);
                    autocompletionString.clear();
                    onTextModified( this, line );
                }
            }
            else if (isAsciiKeyChar(ch))
            {
                line.append(1, (char)ch);
            }
            else
            {
                // Хрень какая-толькопришла, просто игнорим
            }
        
        } // for( auto i : input )

    }

    // //! \param text can be modified in this event handler
    // virtual void onTextModified ( SimpleTerminalLineEdit *pEdit, std::string &text ) { }
    //  
    // //! Return true if buffer must kept after this call (buffer can be filled with new text by the setText function or by modifying param text)
    // virtual bool onEditCompleted( SimpleTerminalLineEdit *pEdit, std::string &text ) { return false; } 


}; // class SimpleTerminalLineEdit

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename ModifiedHadler, typename CompleteHandler >
class SimpleTerminalLineEditImplBase : public SimpleTerminalLineEdit
{
    ModifiedHadler        modifiedHadler;
    CompleteHandler       completeHandler;

public:

    SimpleTerminalLineEditImplBase( ModifiedHadler mh,  CompleteHandler ch )
    : modifiedHadler(mh)
    , completeHandler(ch)
    {}

protected:

    //! \param text can be modified in this event handler
    virtual void onTextModified ( SimpleTerminalLineEdit *pEdit, std::string &text ) override
    {
        modifiedHadler( pEdit, text );
    }

    //! Return true if buffer must kept after this call (buffer can be filled with new text by the setText function or by modifying param text)
    virtual bool onEditCompleted( SimpleTerminalLineEdit *pEdit, std::string &text ) override
    {
        return completeHandler( pEdit, text );
    }


}; // class SimpleTerminalLineEditImplBase






//----------------------------------------------------------------------------


} // namespace invest_openapi

//----------------------------------------------------------------------------
