/*! \file
    \brief 
 */

//----------------------------------------------------------------------------
#pragma once

#include <QString>
#include <QSettings>

#include <exception>
#include <stdexcept>

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
struct TerminalConfig
{

    std::vector< FieldFormat >  fieldsFormat;



    /*
    
     1) Считываем список полей/столбцов - terminal.columns = СПИСОК ID


     2) Считываем параметры столбцов 

        terminal.columns.ID.caption
        terminal.columns.ID.align.caption
        terminal.columns.ID.left
        terminal.columns.ID.right
        terminal.columns.ID.width
        terminal.columns.ID.align
        terminal.columns.ID.align.dot - выравнивание по десятичной точке - сколько пробелов добивать справа, чтобы выровнять по точке


     3) Где брать значения по умолчанию?

        Перво-наперво считываем параметры terminal.columns.defaults.*
        Их потом и подставляем.

        Если не задан caption, используем ID.


     4) Параметр align(ment)

        Может принимать значения: -1 - left
                                   0 - center
                                   1 - right

        И строка и число должны нормас читаться.
        Читаем строку, uppercase и сами по строке получаем? Ну, ока, не айс, но по-быстрому не придумывается.
        Тут может быть проблема с числами - в число бы оно сконвертилось, а строка - не та. Но наверное - хусим

     */



/*
    QString  token;
    QString  sanboxToken;
    bool     sandboxMode = false;

    QString  brokerAccountId       ;
    QString  sandboxBrokerAccountId;



    void load( const QSettings &settings )
    {
        token                  = settings.value("token"        ).toString();
        brokerAccountId        = settings.value("broker-account-id").toString();

        sanboxToken            = settings.value("sandbox-token").toString();
        sandboxBrokerAccountId = settings.value("sandbox-broker-account-id" ).toString();  

        sandboxMode            = settings.value("sandbox-mode" ).toBool();
    }

    void checkValid() const
    {
        // if (token.isEmpty())
        //     throw std::runtime_error("Token is empty ('token')");
    }

    AuthConfig( const QSettings &settings )
    {
        load(settings);
        checkValid();
    }

    AuthConfig( const QString &settingsFile )
    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        load(settings);
        checkValid();
    }

    AuthConfig( const QString &tk, const QString &smbxtk, bool sm )
    {
        token       = tk;
        sanboxToken = smbxtk;
        sandboxMode = sm;
    }


    void setDefaultBrokerAccountForOpenApi( QSharedPointer<IOpenApi> pOpenApi ) const;
*/

}; // struct TerminalConfig

//----------------------------------------------------------------------------




} // namespace invest_openapi


