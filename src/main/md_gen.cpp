/*! \file
    \brief Configs lookup test

 */

//----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <iomanip>
#include <vector>
#include <cstring>
#include <string>
#include <map>
#include <vector>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>
#include <QVariant>

#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep

#include "invest_openapi/invest_openapi.h"

#include "cpp/cpp.h"

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
std::string q()
{
    return "\"";
}

//------------------------------
std::string q( const std::string &str )
{
    return q() + str + q();
}

//------------------------------
std::string q( const std::string &str1, const std::string &str2 )
{
    return q( str1+std::string(" ")+str2 );
}

//----------------------------------------------------------------------------
template <typename StringType> inline bool starts_with( const StringType &str, const StringType &prefix )
{
    if (str.size()<prefix.size())
        return false;

    return str.compare( 0, prefix.size(), prefix )==0;
}

//------------------------------
template <typename StringType> inline bool ends_with( const StringType &str, const StringType &postfix )
{
    if (str.size()<postfix.size())
        return false;

    return str.compare( str.size()-postfix.size(), postfix.size(), postfix )==0;
}

//----------------------------------------------------------------------------
YAML::Node parse(std::istream& input)
{
    return YAML::Load(input);
    //std::cout << doc << "\n";
  //} catch (const YAML::Exception& e) {
  //  std::cerr << e.what() << "\n";
  //}
}

//------------------------------
YAML::Node parse(const char *inputName)
{
    if (std::strcmp(inputName,"-")==0)
    {
        return parse(std::cin);
    }

    std::ifstream fin;
    fin.open(inputName);
    return parse(fin);
}

//----------------------------------------------------------------------------
std::string trimHelper( const std::string &str )
{
    return QString::fromStdString(str).trimmed().toStdString();
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> parseSchemaIni(std::istream& input)
{
    using std::cout;
    using std::cerr;
    using std::endl;

    std::map<std::string, std::string> resMap;

    std::string str;

    //cerr << "Reading INI" << endl;

    while( std::getline( input, str) )
    {
        //cerr << "INI line: " << str << endl;

        str = trimHelper(str);
        if (str.empty() || str[0]=='#' || str[0]==';')
            continue;

        std::string::size_type sepPos = str.find_first_of('=');
        if (sepPos==std::string::npos)
        {
            resMap[trimHelper( std::string( str, 0, sepPos ) )] = std::string();
        }
        else
        {
            resMap[trimHelper( std::string( str, 0, sepPos ) )] = trimHelper( std::string( str, sepPos+1, std::string::npos ) );
        }

    }

    //cerr << endl;

    return resMap;
}

//----------------------------------------------------------------------------
std::map<std::string, std::string> parseSchemaIni(const char *inputName)
{
    std::ifstream fin;
    fin.open(inputName);
    return parseSchemaIni(fin);
}

//----------------------------------------------------------------------------
std::string getSqlSpec( const std::map<std::string, std::string> &sqlSpec
                      , const std::string &modelTypeName
                      , const std::string &fieldName
                      , const std::string &fieldType
                      , const std::string &fieldTypeFormat
                      , std::string *lookupForItems = 0
                      )
{
    std::vector< std::string > lookupFor;

    lookupFor.push_back( std::string("::") + fieldType );

    if (!fieldTypeFormat.empty())
        lookupFor.push_back( std::string("::") + fieldType + std::string("::") + fieldTypeFormat );

    lookupFor.push_back( formatName( fieldName, cpp::NameStyle::defineStyle ) );
    lookupFor.push_back( std::string("::") + formatName( modelTypeName, cpp::NameStyle::pascalStyle ) + std::string("::") + fieldType );
    lookupFor.push_back( std::string("::") + formatName( modelTypeName, cpp::NameStyle::defineStyle ) + std::string("::") + fieldType );

    if (!fieldTypeFormat.empty())
    {
        lookupFor.push_back( std::string("::") + formatName( modelTypeName, cpp::NameStyle::pascalStyle ) + std::string("::") + fieldType + std::string("::") + fieldTypeFormat );
        lookupFor.push_back( std::string("::") + formatName( modelTypeName, cpp::NameStyle::defineStyle ) + std::string("::") + fieldType + std::string("::") + fieldTypeFormat );
    }


    if (lookupForItems)
        *lookupForItems = invest_openapi::mergeString( lookupFor, ", ");
        // invest_openapi::mergeString( QString::fromStdString(str), QString(", ") ).toStdString();

    std::string resStr;

    using std::cout;
    using std::cerr;
    using std::endl;

    cerr<<"SQL spec order: ";
    for (auto it = lookupFor.begin(); it!=lookupFor.end(); ++it )
    {
        if (it != lookupFor.begin())
            cerr<<", ";
        cerr<< *it;
    }

    cerr << endl;



    for (auto it = lookupFor.begin(); it!=lookupFor.end(); ++it )
    {
        std::string strSqlSpecName = trimHelper(*it);
        if (strSqlSpecName.empty())
            continue;

        auto fountSpecIt = sqlSpec.find(*it);
        if (fountSpecIt==sqlSpec.end())
            continue;

        auto curSpecString = fountSpecIt->second;
        if (curSpecString.empty())
            continue;

        /*
           Префиксы
           '-' - сбрасываем всё предыдущее, присваиваем новое значение и продолжаем поиск
           '+' - добавляем к ранее собранному и продолжаем поиск
           '=' - возвращаем только найденное на данном шаге
       
           Без префикса - переназначаемо, пока примем, что это тоже, что и префикс '+'
        
        */

        char ruleSpec = '+';

        if (curSpecString[0]=='+' || curSpecString[0]=='-' || curSpecString[0]=='=')
        {
            ruleSpec = curSpecString[0];
            curSpecString.erase(0,1);
            curSpecString = trimHelper(curSpecString);
        }

        switch(ruleSpec)
        {
            case '-':
                      resStr = curSpecString;
                      break;

            case '+':
                      if (!resStr.empty())
                          resStr.append(1, ' ');
                      resStr.append(curSpecString);
                      break;

            case '=':
                      resStr = curSpecString;
                      return resStr;

            default :
                      throw std::runtime_error("getSqlSpec - invalid SQL spec");
        }
    }

    return resStr;
}

//----------------------------------------------------------------------------
std::string getSqlModelIdSpec( const std::map<std::string, std::string> &sqlSpec
                             , const std::string &modelTypeName
                             )
{
    std::vector< std::string > lookupFor;

    lookupFor.push_back( std::string("::schema::") + formatName( modelTypeName, cpp::NameStyle::pascalStyle ) + std::string("::ID") );
    lookupFor.push_back( std::string("::schema::") + formatName( modelTypeName, cpp::NameStyle::defineStyle ) + std::string("::ID"));

    for (auto it = lookupFor.begin(); it!=lookupFor.end(); ++it )
    {
        std::string strSqlSpecName = trimHelper(*it);
        if (strSqlSpecName.empty())
            continue;

        auto fountSpecIt = sqlSpec.find(*it);
        if (fountSpecIt==sqlSpec.end())
            continue;

        return trimHelper(fountSpecIt->second);
    }

    return std::string();
}

//----------------------------------------------------------------------------
std::string getSqlModelFieldExtraSpec( const std::map<std::string, std::string> &sqlSpec
                                     , const std::string &modelTypeName
                                     , const std::string &fieldName
                                     , const std::string &orderKeyword // before/after
                                     )
{
    std::vector< std::string > lookupFor;

    lookupFor.push_back( std::string("::schema::") + formatName( modelTypeName, cpp::NameStyle::pascalStyle ) + std::string("::") + orderKeyword + std::string("::") + formatName( fieldName, cpp::NameStyle::defineStyle ) );
    lookupFor.push_back( std::string("::schema::") + formatName( modelTypeName, cpp::NameStyle::defineStyle ) + std::string("::") + orderKeyword + std::string("::") + formatName( fieldName, cpp::NameStyle::defineStyle ) );

    for (auto it = lookupFor.begin(); it!=lookupFor.end(); ++it )
    {
        std::string strSqlSpecName = trimHelper(*it);
        if (strSqlSpecName.empty())
            continue;

        auto fountSpecIt = sqlSpec.find(*it);
        if (fountSpecIt==sqlSpec.end())
            continue;

        return trimHelper(fountSpecIt->second);
    }

    return std::string();
}

//----------------------------------------------------------------------------
bool splitSqlFieldSpec( const std::string &sqlFieldSpec, std::string &fieldName, std::string &fieldSqlSpec )
{
    auto spacePos = sqlFieldSpec.find_first_of(' ');
    if (spacePos==std::string::npos)
        return false;

    fieldName    = std::string( sqlFieldSpec, 0, spacePos );
    fieldName    = trimHelper(fieldName);

    fieldSqlSpec = std::string( sqlFieldSpec, spacePos+1  );
    fieldSqlSpec = trimHelper(fieldSqlSpec);

    return true;
}

//----------------------------------------------------------------------------
inline
std::string toString( YAML::NodeType::value v )
{
    switch(v)
    {
        case YAML::NodeType::Undefined: return "Undefined";
        case YAML::NodeType::Null     : return "Null";
        case YAML::NodeType::Scalar   : return "Scalar";
        case YAML::NodeType::Sequence : return "Sequence";
        case YAML::NodeType::Map      : return "Map";
        default                       : return "Unknown";
    }
}

//----------------------------------------------------------------------------
bool isResponseType( const std::string &str )
{
    if (ends_with(str, std::string("Response")))
        return true;
    return false;
}

//----------------------------------------------------------------------------
bool isRequestType( const std::string &str )
{
    if (ends_with(str, std::string("Request")))
        return true;
    return false;
}

//----------------------------------------------------------------------------
inline
bool isRefTypeName( const std::string &typeName )
{
    if (typeName.empty())
        return false;

    return typeName[0] == '#';
}

//----------------------------------------------------------------------------
inline
std::string extractTypeNameFromRef( const std::string &typeName )
{
    std::string::size_type slashPos = typeName.find_last_of('/');
    if (slashPos==std::string::npos)
    {
        return false;
    }
   
    return std::string( typeName, slashPos+1, std::string::npos );
}

//----------------------------------------------------------------------------
bool getProperyType( const YAML::Node &propProperties, std::string &propType )
{
    try
    {
        propType = propProperties["type"].as<std::string>();
        return true;
    }
    catch(...)
    {
        try
        {
            propType = propProperties["$ref"].as<std::string>();
            return true;
        }
        catch(...)
        {
            return false;
        }
    
    }

    return true;
}

//----------------------------------------------------------------------------
bool getProperyTypeFormat( const YAML::Node &propProperties, std::string &propTypeFormat )
{
    try
    {
        propTypeFormat = propProperties["format"].as<std::string>();
        return true;
    }
    catch(...)
    {
    }

    return false;
}

//----------------------------------------------------------------------------
bool isProperyTypeArray( const std::string &propType )
{
    return propType=="array";
}

//----------------------------------------------------------------------------
bool skipNoTypeOrRefOrArray( const std::string &typeName, const YAML::Node &propertiesNode )
{
    using std::cout;
    using std::cerr;
    using std::endl;

    //const YAML::Node &propertiesNode = typeIt->second["properties"];
    YAML::Node::const_iterator propIt = propertiesNode.begin();
    for (; propIt!=propertiesNode.end(); ++propIt)
    {
        std::string propName = propIt->first.as<std::string>();

        const YAML::Node &propProperties = propIt->second;

        std::string propType;
        if (!getProperyType( propProperties, propType ))
        {
            cerr<<"Skip request type: " << typeName << " due has no 'type' or '$ref' attribute" << endl << endl << endl;
            //cerr<<"Property: " << propName << "has no 'type' or '$ref' attribute" << endl << flush;
            return true;
        }

        if (isProperyTypeArray(propType))
        {
            cerr<<"Skip request type: " << typeName << " due has 'array' type" << endl << endl << endl;
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
bool isSequenceOrMap(YAML::NodeType::value v)
{
    switch(v)
    {
        case YAML::NodeType::Sequence : 
        case YAML::NodeType::Map      : return true;
        default                       : return false;
    }
}

//----------------------------------------------------------------------------
bool isTagMeanfull( const std::string &tagName )
{
    if (tagName=="?" || tagName=="!")
        return false;
    return true;
}

//----------------------------------------------------------------------------
bool hasPayloadPropery( const std::string &typeName, const YAML::Node &typeInfoNode )
{
    using std::cout;
    using std::cerr;
    using std::endl;

    try
    {
        cerr << typeName << ".hasPayloadPropery (1)" << endl;
        const YAML::Node &propsNode   = typeInfoNode["properties"];
        if (propsNode.Type()!=YAML::NodeType::Map)
        {
            cerr << typeName << " - 'properties' is not a map" << endl;
            //cerr << typeName << " - no 'properties'" << endl;
            cerr << typeName << ".hasPayloadPropery - false" << endl << endl;
            return false;
        }

        cerr << typeName << ".hasPayloadPropery (2)" << endl;
        const YAML::Node &payloadNode = propsNode["payload"];

        YAML::NodeType::value typeInfoNodeType = typeInfoNode.Type();
        cerr << typeName << ", typeInfoNode - type: " << toString(typeInfoNodeType) << endl;

        YAML::NodeType::value payloadNodeType = payloadNode.Type();
        cerr << typeName << ", payloadNode - type: " << toString(payloadNodeType) << endl;

        if (propsNode.Type()!=YAML::NodeType::Map)
        {
            cerr << typeName << " - no 'payload'" << endl;
            cerr << typeName << ".hasPayloadPropery - false" << endl << endl;
            return false;
        }

        cerr << typeName << ".hasPayloadPropery - true" << endl << endl;
        return true;
    }
    catch (const YAML::Exception& e)
    {
        cerr << "YAML parsing exception: " << e.what() << endl;
        cerr << typeName << ".hasPayloadPropery - false" << endl << endl;
        return false;
    }
    catch(...)
    {
        cerr << "YAML parsing unknown exception" << endl;
        cerr << typeName << ".hasPayloadPropery - false" << endl << endl;
        return false;
    }
}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
static const int maxRecurseLevel = 0x7FFFFFFF;

void printNode( const YAML::Node &node, int recurseCounter = maxRecurseLevel, const std::string &indend = "" )
{
    using std::cout;
    using std::endl;

    recurseCounter--;
    if (recurseCounter<0)
        return;

    static const std::string deltaIndend = "    ";

    auto nodeType = node.Type();

    
    cout << indend << "Node type: " << toString(nodeType);
    if (isSequenceOrMap(nodeType))
        cout << endl;
    else
        cout << ", ";

    //cout << indend << "Node type: " << toString(nodeType) << endl;

    switch(nodeType)
    {
        case YAML::NodeType::Undefined:
        case YAML::NodeType::Null     :
             {
                 cout  /* << indend */  << "Value    : " << node.as<std::string>() << endl;
             }
             break;

        case YAML::NodeType::Scalar   :
             {
                 cout  /* << indend */  << "Value    : " << node.as<std::string>();
                 if (isTagMeanfull(node.Tag()))
                     cout << ", " << "Tag  : " << node.Tag();
                 cout << endl;
             }
             break;

        case YAML::NodeType::Sequence :
             {
                 cout << indend << "Value    : " << "<NULL>" << ", ";
                 //cout << indend << "Tag  : " << node.Tag()             << endl;

                 //if (!fRecurse)
                 //    break;

                 if (recurseCounter!=0)
                 {
                     cout << endl;
                     cout << indend << "Items: "                           << endl;
                     cout << endl;
                 }

                 for (YAML::Node::const_iterator it=node.begin(); it!=node.end(); ++it)
                 {
                     if (recurseCounter!=0)
                         cout << indend << "  Item"                           << endl;
                     printNode(*it, recurseCounter, indend + deltaIndend );

                     cout << endl;
                 }
             }
             break;

        case YAML::NodeType::Map      :
             {
                 cout << indend << "Value    : " << "<NULL>" << ", " << endl;
                 //cout << indend << "Name"
                 //cout << indend << "Tag  : " << node.Tag()             << endl;
                 //if (!fRecurse)
                 //    break;

                 if (recurseCounter!=0)
                 {
                     cout << endl;
                     cout << indend << "Items: "                           << endl;
                     cout << endl;
                 }

                 for (YAML::Node::const_iterator it=node.begin(); it!=node.end(); ++it)
                 {
                     if (recurseCounter!=0)
                         cout << indend << "  First"                           << endl;
                     printNode(it->first , recurseCounter, indend + deltaIndend );

                     if (recurseCounter!=0)
                         cout << indend << "  Second"                          << endl;
                     printNode(it->second, recurseCounter, indend + deltaIndend );

                     cout << endl;
                 }
             }
             break;

        default                       :
             {
             }
    }

}

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("md_gen");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::cerr;
    using std::endl;
    using std::flush;


    if (argc<4)
    {
        std::cerr<<"No input files taken";
        return -1;
    }

    try
    {
        std::string arg1 = argv[1];
        std::string arg2 = argv[2];
        std::string arg3 = argv[3];

        YAML::Node rootNode = parse(argv[1]);
        std::map<std::string, std::string> sqlSchemaMap = parseSchemaIni(argv[2]);

        // Schema file name - argv[2]
        // Output file name - argv[3]

        if (argc>4)
        {
            QVariant qvDepth = 1;

            if (argc>5)
            {
                QLatin1String arg4latin1 = QLatin1String( argv[5] );
                qvDepth = arg4latin1;
            }

            int depth = qvDepth.toInt();
            depth++;

            std::string subNodeName = argv[4];

            if (subNodeName=="")
            {
                std::cout << "Invalid (empty) subNodeName" << endl;
                return -1;
            }
            else if (subNodeName=="root-node")
            {
                printNode( rootNode, depth );
                return 0;
            }
            else if (subNodeName=="openapi" || subNodeName=="info" || subNodeName=="tags" || subNodeName=="paths"
                  || subNodeName=="servers" || subNodeName=="externalDocs" || subNodeName=="components" // || subNodeName=="" || subNodeName==""
                    )
            {
                printNode( rootNode[subNodeName], depth );
                return 0;
            }
            else if (subNodeName=="components/schemas" || subNodeName=="/components/schemas")
            {
                printNode( rootNode["components"]["schemas"], depth );
                return 0;
            }
            else
            {
                std::cout << "Invalid (unknown) subNodeName: " << subNodeName << endl;
                return -1;
            }
        }


        // Do main job, not test

        std::ofstream foutStream;
        std::ostream *pOut = &std::cout;
        if (std::strcmp(argv[3],"-")!=0)
        {
            foutStream.open(argv[3]);
            pOut = &foutStream;
        }

        std::ostream &fout = *pOut;

        fout<<"/*! \\file"                              <<endl
            <<"    \\brief Autogenerated file"          <<endl
            <<" */"                                     <<endl
                                                        <<endl
            <<"#pragma once"                            <<endl
                                                        <<endl
            <<"#include <QVector>"                      <<endl
            <<"#include <QString>"                      <<endl
                                                        <<endl
            <<"#include \"models.h\""                   <<endl
            <<"#include \"models_helpers.h\""           <<endl
            <<"#include \"utility.h\""                  <<endl
                                                        <<endl
            <<"namespace invest_openapi"                <<endl
            <<"{"                                       <<endl
                                                        <<endl
            <<"using namespace OpenAPI;"                <<endl
                                                        <<endl
                                                        <<endl
            <<"//----------------------------------------------------------------------------" << endl;
            ;


        const YAML::Node &schemasNode = rootNode["components"]["schemas"];

        if (schemasNode.Type()!=YAML::NodeType::Map)
        {
            cout << "Expected type 'Map' for node '/components/schemas', but got the '" << toString(schemasNode.Type()) << "'\n";
            return -1;
        
        }

        std::set<std::string>  skippingSet;

        // fout<<"template <typename ModelType> QString modelMakeSqlSchemaStringVector( );" << endl << endl;
        // fout<<"//----------------------------------------------------------------------------" << endl;

        //!!! First iteration - generating prototypes for 'modelToStrings'

        YAML::Node::const_iterator typeIt = schemasNode.begin();
        for (; typeIt!=schemasNode.end(); ++typeIt)
        {
            //fout << "Type: " << it->first << endl;
            std::string typeName = typeIt->first.as<std::string>();

            if (isRequestType(typeName))
            {
                cerr<<"Skipping request type: " << typeName << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }

            if (isResponseType(typeName))
            {
                cerr<<"Skipping response type: " << typeName << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }


            if (hasPayloadPropery(typeName, typeIt->second))
            {
                cerr<<"Skipping type with payload: " << typeName << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }

            if (skipNoTypeOrRefOrArray( typeName, typeIt->second["properties"] ))
            {
                skippingSet.insert(typeName);
                continue;
            }

            const YAML::Node &propertiesNode = typeIt->second["properties"];
            if (!propertiesNode.IsDefined())
            {
                cerr<<"Skipping type: " << typeName << " due propertiesNode !IsDefined()" << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }

            if (propertiesNode.IsNull())
            {
                cerr<<"Skipping type: " << typeName << " due propertiesNode IsNull()" << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }

            if (propertiesNode.begin()==propertiesNode.end())
            {
                cerr<<"Skipping type: " << typeName << " due no properties" << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }


            fout<<"QVector<QString> modelToStrings ( const " << typeName << " &v );" << endl;
            //fout<<"template <> QVector<QString> modelMakeSchema< " << typeName << " >( );" << endl;
        }

        fout << "//----------------------------------------------------------------------------" << endl;
        fout << endl << endl << endl; // << endl;
        fout << "//----------------------------------------------------------------------------" << endl;

        typeIt = schemasNode.begin();
        for (; typeIt!=schemasNode.end(); ++typeIt)
        {
            //fout << "Type: " << it->first << endl;
            std::string typeName = typeIt->first.as<std::string>();

            if (skippingSet.find(typeName)!=skippingSet.end())
                continue;


            const YAML::Node &propertiesNode = typeIt->second["properties"];

            if (!propertiesNode.IsDefined() || propertiesNode.IsNull() || propertiesNode.begin()==propertiesNode.end())
                continue;


            if (skipNoTypeOrRefOrArray( typeName, typeIt->second["properties"] ))
                continue;

            fout<<"template <> QVector<QString> modelMakeSqlSchemaStringVector< " << typeName << " >( const QString &nameOrPrefix );" << endl;
        }


        fout << "//----------------------------------------------------------------------------" << endl << endl << endl ;

        //!!! Second iteration - generating 'modelToStrings' methods

        std::map< std::string, std::string > usedPropTypes;

        typeIt = schemasNode.begin();
        for (; typeIt!=schemasNode.end(); ++typeIt)
        {
            //fout << "Type: " << it->first << endl;
            std::string typeName = typeIt->first.as<std::string>();

            if (skippingSet.find(typeName)!=skippingSet.end())
                continue;


            const YAML::Node &propertiesNode = typeIt->second["properties"];

            if (!propertiesNode.IsDefined() || propertiesNode.IsNull() || propertiesNode.begin()==propertiesNode.end())
                continue;


            if (skipNoTypeOrRefOrArray( typeName, typeIt->second["properties"] ))
                continue;

            fout << endl << "//----------------------------------------------------------------------------" << endl;
            fout << "//! Converts " << typeName << " to QVector of QString's " << endl;
            fout<< "inline QVector<QString> modelToStrings( const " << typeName << " &v )" << endl
                << "{" << endl
                << "    QVector<QString> resVec;" 
                << endl
                ;

            //!!! Third iteration - 
            YAML::Node::const_iterator propIt = propertiesNode.begin();
            for (; propIt!=propertiesNode.end(); ++propIt)
            {
                std::string propName = propIt->first.as<std::string>();

                cerr<<"Processing property: " << propName << endl << flush;

                const YAML::Node &propProperties = propIt->second;


                std::string propType;
                if (!getProperyType( propProperties, propType ))
                {
                    cerr<<"Property: " << propName << "has no 'type' or '$ref' attribute" << endl << flush;
                    return false;
                }

                std::string propTypeFormat;
                getProperyTypeFormat( propProperties, propTypeFormat );
                usedPropTypes[propType] = propTypeFormat;

                cerr << "  Type: " << cpp::expandAtBack(propType, 20);
                if (!propTypeFormat.empty())
                    cerr << " : " << propTypeFormat << endl;
                else
                    cerr << endl;

                if (isRefTypeName(propType))
                {
                    propType = extractTypeNameFromRef(propType);
                }

                //------------------------------

                fout << endl
                     << "    //------------------------------" << endl
                     << "    if ( !v.is_" << cpp::formatName(propName,cpp::NameStyle::cppStyle) << "_Set() || !v.is_" << cpp::formatName(propName,cpp::NameStyle::cppStyle) << "_Valid() ) // type: " << propType << endl
                     << "        appendToStringVector(resVec, QString());" << endl
                     << "    else" << endl
                     << "        appendToStringVector(resVec, modelToStrings( v.get" << cpp::formatName(propName,cpp::NameStyle::pascalStyle) << "() ) );" << endl
                     ;
            }

            fout << endl
                 << "    //------------------------------" << endl
                 << "    return resVec;" << endl
                 ;

            fout<< "}" << endl ;

        }

        fout << endl << "//----------------------------------------------------------------------------" << endl;

        fout << endl << endl << endl << endl << endl;

        const std::string appendToSchemaVecStart = "    appendToStringVector( schemaVec, ";

        //!!! Fourth iteration - generating 'modelMakeSqlSchema' specializations

        static const std::size_t sqlFieldWidth     = 23;
        static const std::size_t sqlFieldSpecWidth = 16;


        typeIt = schemasNode.begin();
        for (; typeIt!=schemasNode.end(); ++typeIt)
        {
            //fout << "Type: " << it->first << endl;
            std::string typeName = typeIt->first.as<std::string>();

            if (skippingSet.find(typeName)!=skippingSet.end())
                continue;


            const YAML::Node &propertiesNode = typeIt->second["properties"];

            if (!propertiesNode.IsDefined() || propertiesNode.IsNull() || propertiesNode.begin()==propertiesNode.end())
                continue;


            if (skipNoTypeOrRefOrArray( typeName, typeIt->second["properties"] ))
                continue;

            fout << endl << "//----------------------------------------------------------------------------" << endl;
            fout << "//! Creates SQL schema format for '" << typeName << "' model " << endl;
            fout<<"template <> inline QVector<QString> modelMakeSqlSchemaStringVector< " << typeName << " >( const QString &nameOrPrefix )" << endl;
            fout<< "{" << endl
                << "    QVector<QString> schemaVec;" << endl
                << "    QString p = nameOrPrefix.isEmpty() ? QString() : nameOrPrefix + QString(\"_\");" << endl
                << endl
                ;


            using cpp::formatName;
            using cpp::detectNameStyle;
            using cpp::NameStyle;
            using cpp::expandAtBack;
            using cpp::expandAtFront;


            auto idSpec = getSqlModelIdSpec( sqlSchemaMap, typeName );
            if (!idSpec.empty())
            {
                fout << appendToSchemaVecStart << "p + " << q( expandAtBack("ID",sqlFieldWidth), expandAtBack(idSpec,sqlFieldSpecWidth) ) << " );" << " // ID spec" << endl;
            }


            YAML::Node::const_iterator propIt = propertiesNode.begin();
            for (; propIt!=propertiesNode.end(); ++propIt)
            {
                std::string propName = propIt->first.as<std::string>();

                const YAML::Node &propProperties = propIt->second;

                std::string propType;
                if (!getProperyType( propProperties, propType ))
                    return false;

                std::string propTypeFormat;
                getProperyTypeFormat( propProperties, propTypeFormat );
                //usedPropTypes[propType] = propTypeFormat;

                if (isRefTypeName(propType))
                    propType = extractTypeNameFromRef(propType);

                std::string sqlFieldSpecPropBefore = getSqlModelFieldExtraSpec( sqlSchemaMap, typeName, propName, "before" );
                if (!sqlFieldSpecPropBefore.empty())
                {
                    cerr<<"!!! Found 'before' spec for "<<typeName<<"."<<propName<<endl;

                    std::string fieldName, fieldSqlSpec;
                    if (!splitSqlFieldSpec(sqlFieldSpecPropBefore, fieldName, fieldSqlSpec))
                    {
                        cerr<<"Failed to parse sql spec for " << typeName << "." << propName << endl;
                    }
                    else
                    {
                        fout << appendToSchemaVecStart << "p + " << q( expandAtBack(fieldName,sqlFieldWidth), expandAtBack(fieldSqlSpec,sqlFieldSpecWidth) ) << " );" << " // Spec ::schema::" << typeName << "::before::" << propName << endl;
                    }
                }

                //auto propNameUpper = cpp::toUpper(propName);
                auto propNameSql = formatName( propName, cpp::NameStyle::defineStyle );
                auto propTypeSql = formatName( propType, cpp::NameStyle::defineStyle );

                std::string lookupFor;
                auto sqlSpec = getSqlSpec( sqlSchemaMap, typeName, propNameSql , propType, propTypeFormat, &lookupFor );
                std::string specLookupComment = std::string(" // Spec lookup order: ") + lookupFor ;

                auto propTypeStyle = detectNameStyle(propType);

                if (propTypeStyle == NameStyle::pascalStyle)
                {
                    std::string typeSqlGenerationPrefix = propNameSql + "_" + propTypeSql;
                    if (propTypeSql.find(propNameSql)!=propTypeSql.npos)
                        typeSqlGenerationPrefix = propTypeSql;

                    fout << appendToSchemaVecStart << "modelMakeSqlSchemaStringVector<" << propType << ">( " << "p + " << q(typeSqlGenerationPrefix) << " ) ); // " << propName << endl;
                    // nameOrPrefix
                }
                else
                {
                    fout << appendToSchemaVecStart << "p + " << q( expandAtBack(propNameSql,sqlFieldWidth), expandAtBack(sqlSpec,sqlFieldSpecWidth) ) << " );" << specLookupComment << endl;
                }

                std::string sqlFieldSpecPropAfter = getSqlModelFieldExtraSpec( sqlSchemaMap, typeName, propName, "after" );
                if (!sqlFieldSpecPropAfter.empty())
                {
                    cerr<<"!!! Found 'before' spec for "<<typeName<<"."<<propName<<endl;

                    std::string fieldName, fieldSqlSpec;
                    if (!splitSqlFieldSpec(sqlFieldSpecPropAfter, fieldName, fieldSqlSpec))
                    {
                        cerr<<"Failed to parse sql spec for " << typeName << "." << propName << endl;
                    }
                    else
                    {
                        fout << appendToSchemaVecStart << "p + " << q( expandAtBack(fieldName,sqlFieldWidth), expandAtBack(fieldSqlSpec,sqlFieldSpecWidth) ) << " );" << " // Spec ::schema::" << typeName << "::after::" << propName << endl;
                    }
                }
                
            }

            fout << endl
                 << "    return schemaVec;" << endl
                 ;

            fout<< "}" << endl ;

        } //!!! Fourth iteration - generating 'modelMakeSqlSchema' specializations


        fout << endl << endl
             << "inline QMap<QString,QString> modelMakeAllSqlShemas()" << endl
             << "{" << endl
             << "    QMap<QString,QString> resMap;" << endl << endl
             ;

        typeIt = schemasNode.begin();
        for (; typeIt!=schemasNode.end(); ++typeIt)
        {
            //fout << "Type: " << it->first << endl;
            std::string typeName = typeIt->first.as<std::string>();

            if (skippingSet.find(typeName)!=skippingSet.end())
                continue;


            const YAML::Node &propertiesNode = typeIt->second["properties"];

            if (!propertiesNode.IsDefined() || propertiesNode.IsNull() || propertiesNode.begin()==propertiesNode.end())
                continue;


            if (skipNoTypeOrRefOrArray( typeName, typeIt->second["properties"] ))
                continue;


            auto tableNameSql  = formatName( typeName, cpp::NameStyle::defineStyle );
            auto expandSqlStr  = cpp::makeExpandString( tableNameSql, 24 );
            auto expandTypeStr = cpp::makeExpandString( typeName    , 22 );

            fout << "    resMap[ \"" << tableNameSql << "\"" << expandSqlStr << " ] " << " = modelMakeSqlCreateTableSchema( modelMakeSqlSchemaStringVector< " << typeName << expandTypeStr << " >( QString() ) " << "); " << endl;

            //fout<<"template <> inline QVector<QString> modelMakeSqlSchemaStringVector< " << typeName << " >( const QString &nameOrPrefix )" << endl;
        }

        fout << endl << "    return resMap;" << endl
             << "}" << endl << endl << endl;


        fout << endl << endl
             << "inline QSet<QString> modelMakeAllSqlTablesSet()" << endl
             << "{" << endl
             << "    QSet<QString> resSet;" << endl << endl
             ;

        typeIt = schemasNode.begin();
        for (; typeIt!=schemasNode.end(); ++typeIt)
        {
            //fout << "Type: " << it->first << endl;
            std::string typeName = typeIt->first.as<std::string>();

            if (skippingSet.find(typeName)!=skippingSet.end())
                continue;


            const YAML::Node &propertiesNode = typeIt->second["properties"];

            if (!propertiesNode.IsDefined() || propertiesNode.IsNull() || propertiesNode.begin()==propertiesNode.end())
                continue;


            if (skipNoTypeOrRefOrArray( typeName, typeIt->second["properties"] ))
                continue;


            auto tableNameSql  = formatName( typeName, cpp::NameStyle::defineStyle );
            auto expandSqlStr  = cpp::makeExpandString( tableNameSql, 24 );
            auto expandTypeStr = cpp::makeExpandString( typeName    , 22 );

            fout << "    resSet.insert( \"" << tableNameSql << "\"" << expandSqlStr << " );" << endl;

            //fout<<"template <> inline QVector<QString> modelMakeSqlSchemaStringVector< " << typeName << " >( const QString &nameOrPrefix )" << endl;
        }

        fout << endl << "    return resSet;" << endl
             << "}" << endl << endl << endl;


        fout<<"} // namespace invest_openapi"           << endl << endl << endl;


        cerr << endl;
        cerr << "Used types:" << endl;

        std::map< std::string, std::string >::const_iterator usedIt = usedPropTypes.begin();
        for(; usedIt != usedPropTypes.end(); ++usedIt)
        {
            std::string typeName = usedIt->first;
            if (isRefTypeName(typeName))
                typeName = extractTypeNameFromRef(typeName);
            
            if (usedIt->second.empty())
                cerr << typeName << endl;
            else
                cerr << cpp::expandAtBack(typeName, 20) << ": " << usedIt->second << endl;
        }

        // std::cout << "Root node type: " << toString( rootNode.Type() ) << endl;

        //printNode( rootNode );

    }
    catch (const YAML::Exception& e)
    {
        std::cerr << "YAML read failed: " << e.what() << endl;
        return -1;
    }

    
    return 0;
}



