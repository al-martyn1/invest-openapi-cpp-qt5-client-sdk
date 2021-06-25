/*! \file
    \brief 
 */

//----------------------------------------------------------------------------
#pragma once

#include <QString>
#include <QSettings>

#include <map>
#include <vector>
#include <exception>
#include <stdexcept>


#include "format_helpers.h"

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


    //------------------------------
    static
    const std::map< QString, int >& getAlignmentIdMap()
    {
        static std::map< QString, int > m;
        if (!m.empty())
            return m;

        m["LEFT"  ] = -1;
        m["-1"    ] = -1;

        m["CENTER"] =  0;
        m["0"     ] =  0;

        m["RIGHT" ] =  1;
        m["1"     ] =  1;

        return m;
    }


    //------------------------------
    static
    QString alignmentToString( int a )
    {
        if (a<0) return "LEFT";
        else if (a>0) return "RIGHT";
        else return "CENTER";
    }


    //------------------------------
    static
    int alignmentFromString( QString val, QString valName )
    {
        auto &m = getAlignmentIdMap();
        auto it = m.find(val.toUpper());
        if (it==m.end())
            throw std::runtime_error( std::string("Invalid value for parameter '") + valName.toStdString() + std::string("' taken") );
        return it->second;
    }


    //------------------------------
    static 
    FieldFormat getDefaultFieldFormat()
    {
        return FieldFormat{ 1 // leftSpace
                          , 0 // rightSpace
                          , 0 // fieldWidth
                          , 1 // alignment - right
                          , 2 // dotAlignment
                          };
    }


    //------------------------------
    FieldFormat loadFieldFormat( const QSettings &settings, QString columnId, const FieldFormat &defaults )
    {
        if (columnId!="default")
            columnId = columnId.toUpper();

        QString baseName = QString("terminal.columns.") + columnId + QString(".");

        FieldFormat ff;

        ff.id               = columnId;
        ff.caption          = settings.value(baseName + "caption", QVariant(columnId)).toString();

        ff.leftSpace        = (std::size_t)settings.value(baseName + "left" , QVariant((int)defaults.leftSpace)) .toInt();
        ff.rightSpace       = (std::size_t)settings.value(baseName + "right", QVariant((int)defaults.rightSpace)).toInt();
        ff.fieldWidth       =              settings.value(baseName + "width", QVariant((int)defaults.fieldWidth)).toInt();

        ff.dotAlignment     = (std::size_t)settings.value(baseName + "align.dot" , QVariant((int)defaults.dotAlignment)).toInt();

        ff.alignment        = alignmentFromString( settings.value(baseName + "align" , QVariant(alignmentToString(defaults.alignment)) ).toString()
                                                 , baseName + "align"
                                                 );
        ff.captionAlignment = alignmentFromString( settings.value(baseName + "align.caption" , QVariant(alignmentToString(defaults.alignment)) ).toString()
                                                 , baseName + "align.caption"
                                                 );

        return ff;

    }


    //------------------------------
    void load( const QSettings &settings )
    {

        QStringList columnsList;
        columnsList = settings.value("terminal.columns" ).toStringList();

        FieldFormat defaults = getDefaultFieldFormat();

        defaults = loadFieldFormat( settings, "default", defaults );

        for( auto id : columnsList )
        {
            fieldsFormat.push_back( loadFieldFormat( settings, id, defaults ) );
        }

    }


    //------------------------------
    void checkValid() const
    {
        // if (token.isEmpty())
        //     throw std::runtime_error("Token is empty ('token')");
    }


    //------------------------------
    TerminalConfig( const QSettings &settings )
    {
        load(settings);
        checkValid();
    }


    //------------------------------
    TerminalConfig( const QString &settingsFile )
    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        load(settings);
        checkValid();
    }

    //------------------------------


}; // struct TerminalConfig

//----------------------------------------------------------------------------




} // namespace invest_openapi


