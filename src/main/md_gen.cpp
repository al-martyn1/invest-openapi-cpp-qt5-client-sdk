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
#include "invest_openapi/utility.h"
#include "invest_openapi/yaml_helpers.h"

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
std::string q( const std::string &str, std::size_t expandSize )
{
    std::string quoted = q(str);
    auto expandSqlStr  = cpp::makeExpandString( quoted, expandSize );
    return quoted + expandSqlStr;
}

//----------------------------------------------------------------------------
std::string q( const std::string &str1, const std::string &str2 )
{
    return q( str1+std::string(" ")+str2 );
}

//------------------------------
std::string q( const std::string &str1, const std::string &str2, std::size_t expandSize )
{
    std::string quoted = q( str1+std::string(" ")+str2 );
    auto expandSqlStr  = cpp::makeExpandString( quoted, expandSize );
    return quoted + expandSqlStr;
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

    lookupFor.push_back( std::string("::") + formatName( modelTypeName, cpp::NameStyle::pascalStyle ) + std::string("::") + formatName( fieldName, cpp::NameStyle::defineStyle ) );
    lookupFor.push_back( std::string("::") + formatName( modelTypeName, cpp::NameStyle::defineStyle ) + std::string("::") + formatName( fieldName, cpp::NameStyle::defineStyle ) );

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
                             , const std::string &specType // inline/schema
                             )
{
    std::vector< std::string > lookupFor;

    lookupFor.push_back( std::string("::schema::") + formatName( modelTypeName, cpp::NameStyle::pascalStyle ) + std::string("::ID::") + specType );
    lookupFor.push_back( std::string("::schema::") + formatName( modelTypeName, cpp::NameStyle::defineStyle ) + std::string("::ID::") + specType );

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
                                     , const std::string &orderKeyword // before/after/inline_break
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
    if (invest_openapi::ends_with(str, std::string("Response")))
        return true;
    return false;
}

//----------------------------------------------------------------------------
bool isRequestType( const std::string &str )
{
    if (invest_openapi::ends_with(str, std::string("Request")))
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
std::ostream& printListDelimiterComma( std::ostream &fout, bool &firstProp )
{
    if (!firstProp)
        fout<<",";
    firstProp = false;
    return fout;
}


//----------------------------------------------------------------------------
// prologHanler( fout, DBTYPE, typeName );
// epilogHanler( fout, DBTYPE, typeName );
// fieldHanler( firstProp, fout, DBTYPE, typeName, propName, propType, propTypeFormat );
template< typename PrologHanler
        , typename EpilogHanler
        , typename FieldHanler
        >
bool iterateYamlTypes( const YAML::Node             &schemasNode
                     , const std::set<std::string>  &skippingSet
                     , std::ostream                 &fout
                     , const std::string            &DBTYPE
                     , const PrologHanler           &prologHanler
                     , const EpilogHanler           &epilogHanler
                     , const FieldHanler            &fieldHanler
                     )
{
    YAML::Node::const_iterator typeIt = schemasNode.begin();
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


        prologHanler( fout, DBTYPE, typeName );

        bool firstProp = true;

        YAML::Node::const_iterator propIt = propertiesNode.begin();
        for (; propIt!=propertiesNode.end(); ++propIt)
        {
            std::string propName = propIt->first.as<std::string>();

            std::cerr<<"Processing property: " << propName << " of " << typeName << std::endl << flush;


            const YAML::Node &propProperties = propIt->second;

            std::string propType;
            if (!getProperyType( propProperties, propType ))
            {
                std::cerr<<"Property: " << propName << "has no 'type' or '$ref' attribute" << std::endl << flush;
                return false;
            }

            std::string propTypeFormat;
            getProperyTypeFormat( propProperties, propTypeFormat );
            //usedPropTypes[propType] = propTypeFormat;

            std::cerr << "  Type: " << cpp::expandAtBack(propType, 20);
            if (!propTypeFormat.empty())
                std::cerr << " : " << propTypeFormat << std::endl;
            else
                std::cerr << std::endl;

            if (isRefTypeName(propType))
            {
                propType = extractTypeNameFromRef(propType);
            }

            fieldHanler( fout, DBTYPE, typeName, firstProp, propName, propType, propTypeFormat );

            firstProp = false;

        }

        epilogHanler( fout, DBTYPE, typeName );

    }


    return true;

}






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


    if (argc<5)
    {
        std::cerr<<"No input files taken";
        return -1;
    }

    try
    {
        std::string arg1 = argv[1];
        std::string arg2 = argv[2];
        std::string arg3 = argv[3];
        std::string arg4 = argv[4];

        YAML::Node rootNode = parse(argv[1]);
        std::map<std::string, std::string> sqlSchemaMap = parseSchemaIni(argv[2]);

        // Schema file name - argv[2]
        // Output file name - argv[3]

        if (argc>5)
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


        const std::string DBTYPE = arg4;
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
            <<"//----------------------------------------------------------------------------" << endl << endl << endl << endl << endl;
            ;


        const YAML::Node &schemasNode = rootNode["components"]["schemas"];

        if (schemasNode.Type()!=YAML::NodeType::Map)
        {
            cout << "Expected type 'Map' for node '/components/schemas', but got the '" << toString(schemasNode.Type()) << "'\n";
            return -1;
        
        }

        std::set<std::string>  skippingSet;

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

            /*
            if (hasPayloadProperty(typeName, typeIt->second))
            {
                cerr<<"Skipping type with payload: " << typeName << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }
            */

            //getPayloadOrPropertiesItself

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

        }





        fout 
             << "//----------------------------------------------------------------------------" << endl
             << "#ifndef INVEST_OPENAPI_GENERATED_MODEL_TO_STRINGS_DECLARED" << endl
             << "#define INVEST_OPENAPI_GENERATED_MODEL_TO_STRINGS_DECLARED" << endl
             << endl
             //<< "//----------------------------------------------------------------------------" << endl
             ;

        iterateYamlTypes( schemasNode, skippingSet, fout, DBTYPE
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {
                              using std::endl;
                              using std::flush;
                              fout<<"QVector<QString> modelToStrings( const " << cpp::expandAtBack(typeName, 24) << " &v );" << endl;
                          }
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {}
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName, bool firstProp, const std::string &propName, const std::string &propType, const std::string &propTypeFormat )
                          {}
                        );

        fout << endl << "#endif /* INVEST_OPENAPI_GENERATED_MODEL_TO_STRINGS_DECLARED */" << endl;

        fout << "//----------------------------------------------------------------------------" << endl;
        fout << endl 
             << endl 
             << endl
             << endl
             ;




        #if 0
        fout << "//----------------------------------------------------------------------------" << endl
             << "#ifndef INVEST_OPENAPI_GENERATED_MODEL_TABLE_GET_COLUMN_NAMES_DECLARED" << endl
             << "#define INVEST_OPENAPI_GENERATED_MODEL_TABLE_GET_COLUMN_NAMES_DECLARED" << endl
             << endl
             //<< "//----------------------------------------------------------------------------" << endl
             ;

        iterateYamlTypes( schemasNode, skippingSet, fout, DBTYPE
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {
                              using std::endl;
                              using std::flush;
                              fout<<"template< > inline QVector<QString> modelTableGetColumnNames< " << cpp::expandAtBack(typeName, 24) << " >( const QString &prefix );" << endl;
                          }
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {}
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName, bool firstProp, const std::string &propName, const std::string &propType, const std::string &propTypeFormat )
                          {}
                        );

        fout << endl << "#endif /* INVEST_OPENAPI_GENERATED_MODEL_TABLE_GET_COLUMN_NAMES_DECLARED */" << endl;

        fout << "//----------------------------------------------------------------------------" << endl;
        fout << endl 
             << endl 
             << endl
             ;
        #endif




        fout << "//----------------------------------------------------------------------------" << endl
             << endl
             //<< "//----------------------------------------------------------------------------" << endl
             ;

        iterateYamlTypes( schemasNode, skippingSet, fout, DBTYPE
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {
                              using std::endl;
                              using std::flush;
                              fout<<"template <> QVector<QString> modelMakeSqlSchemaStringVector_"<<DBTYPE<<"< " << cpp::expandAtBack(typeName, 24) << " >( const QString &nameOrPrefix, bool forInlining );" << endl;
                          }
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {}
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName, bool firstProp, const std::string &propName, const std::string &propType, const std::string &propTypeFormat )
                          {}
                        );

        fout << "//----------------------------------------------------------------------------" << endl;
        fout << endl 
             << endl 
             << endl
             ;





        fout 
             << "//----------------------------------------------------------------------------" << endl
             << "#ifndef INVEST_OPENAPI_GENERATED_MODEL_TO_STRINGS_IMPLEMENTED" << endl
             << "#define INVEST_OPENAPI_GENERATED_MODEL_TO_STRINGS_IMPLEMENTED" << endl
             << endl
             //<< "//----------------------------------------------------------------------------" << endl
             ;

        iterateYamlTypes( schemasNode, skippingSet, fout, DBTYPE
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {
                              using std::endl;
                              using std::flush;
                              //fout<<"QVector<QString> modelToStrings( const " << cpp::expandAtBack(typeName, 24) << " &v )" << endl;
                              fout << endl << "//----------------------------------------------------------------------------" << endl;
                              fout << "//! Converts " << typeName << " to QVector of QString's " << endl;
                              fout << "inline QVector<QString> modelToStrings( const " << typeName << " &v )" << endl
                                   << "{" << endl
                                   << "    QVector<QString> resVec;" 
                                   << endl
                                   ;

                          }
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {
                              using std::endl;
                              using std::flush;

                              fout << endl
                                   << "    //------------------------------" << endl
                                   << "    return resVec;" << endl
                                   ;
                             
                              fout<< "}" << endl ;
                          }
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName, bool firstProp, const std::string &propName, const std::string &propType, const std::string &propTypeFormat )
                          {
                              using std::endl;
                              using std::flush;

                              auto cppPropName = cpp::formatName(propName,cpp::NameStyle::cppStyle);
                             
                              fout << endl
                                   << "    //------------------------------" << endl
                                   << "    if ( !v.is_" << cpp::formatName(propName,cpp::NameStyle::cppStyle) << "_Set() || !v.is_" << cppPropName << "_Valid() ) // type: " << propType << endl
                                   << "        appendToStringVector(resVec, QString());" << endl
                                   << "    else" << endl
                                   ;
                              fout
                                   << "        appendToStringVector(resVec, modelToStrings( v.get" << cpp::formatName(propName,cpp::NameStyle::pascalStyle) << "() ) );" << endl
                                   ;
                          }
                        );

        fout << endl << "#endif /* INVEST_OPENAPI_GENERATED_MODEL_TO_STRINGS_IMPLEMENTED */" << endl;

        fout << "//----------------------------------------------------------------------------" << endl;
        fout << endl 
             << endl 
             << endl
             << endl
             ;




        #if 0
        fout 
             << "//----------------------------------------------------------------------------" << endl
             << "#ifndef INVEST_OPENAPI_GENERATED_MODEL_TABLE_GET_COLUMN_NAMES_IMPLEMENTED" << endl
             << "#define INVEST_OPENAPI_GENERATED_MODEL_TABLE_GET_COLUMN_NAMES_IMPLEMENTED" << endl
             << endl
             //<< "//----------------------------------------------------------------------------" << endl
             ;

        iterateYamlTypes( schemasNode, skippingSet, fout, DBTYPE
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {
                              using std::endl;
                              using std::flush;
                              //fout<<"QVector<QString> modelToStrings( const " << cpp::expandAtBack(typeName, 24) << " &v )" << endl;
                              fout << endl << "//----------------------------------------------------------------------------" << endl;
                              fout << "//! Get column names for " << typeName << endl;
                              fout << "template< > inline QVector<QString> modelTableGetColumnNames< " << typeName << " >( const QString &prefix )" << endl
                                   << "{" << endl
                                   << "    QString p = prefix; // prefix.isEmpty() ? QString() : prefix + QString(\"_\");" << endl
                                   << "    QVector<QString> resVec;" << endl;
                                   //<< endl
                                   ;

                          }
                        , []( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName )
                          {
                              using std::endl;
                              using std::flush;

                              fout // << endl
                                   << "    return resVec;" << endl
                                   ;
                              fout<< "}" << endl ;
                          }
                        , [sqlSchemaMap]( std::ostream& fout, const std::string &DBTYPE, const std::string &typeName, bool firstProp, const std::string &propName, const std::string &propType, const std::string &propTypeFormat )
                          {
                              using std::endl;
                              using std::flush;

                              static const std::size_t sqlFieldNameWidth = 34;
                              static const std::size_t sqlFieldSpecWidth = 34;

                              if (firstProp)
                              {
                                  auto idSpecInline = getSqlModelIdSpec( sqlSchemaMap, typeName, "inline" );
                                  auto idSpecSchema = getSqlModelIdSpec( sqlSchemaMap, typeName, "schema" );
                                 
                                  if (!idSpecInline.empty() || !idSpecSchema.empty())
                                  {
                                      fout<<"    appendToStringVector( resVec, generateFieldNameFromPrefixAndName(p, "<<q("ID")<<") );"
                                          <<endl;
                                      firstProp = false;
                                  }
                              }

                              std::string sqlFieldSpecPropBefore = getSqlModelFieldExtraSpec( sqlSchemaMap, typeName, propName, "before" );
                              if (!sqlFieldSpecPropBefore.empty())
                              {
                                  cerr<<"!!! Found 'before' spec for "<<typeName<<"."<<propName<<endl;
                             
                                  std::string fieldName, fieldSqlSpec;
                                  if (!splitSqlFieldSpec(sqlFieldSpecPropBefore, fieldName, fieldSqlSpec))
                                  {
                                      cerr<<"Failed to parse sql spec 'before' for " << typeName << "." << propName << endl;
                                  }
                                  else
                                  {
                                      fout<<"    appendToStringVector( resVec, generateFieldNameFromPrefixAndName(p, "<<q(fieldName)<<") );"
                                          <<endl;
                                      firstProp = false;
                                  }
                              }

                              auto propNameSql = cpp::formatName( propName, cpp::NameStyle::defineStyle );
                              auto propTypeSql = cpp::formatName( propType, cpp::NameStyle::defineStyle );

                              //std::string lookupFor;
                              //auto sqlSpec = getSqlSpec( sqlSchemaMap, typeName, propNameSql , propType, propTypeFormat, &lookupFor );
                              //std::string specLookupComment = std::string(" // Spec lookup order: ") + lookupFor ;

                              auto propTypeStyle = cpp::detectNameStyle(propType);

                              if (propTypeStyle == cpp::NameStyle::pascalStyle)
                              {
                                  std::string typeSqlGenerationPrefix = propNameSql + "_" + propTypeSql;
                                  if (propTypeSql.find(propNameSql)!=propTypeSql.npos)
                                      typeSqlGenerationPrefix = propTypeSql;
                             
                                  fout << "    "
                                       << "appendToStringVector( resVec, "
                                       << "modelTableGetColumnNames<" << propType << ">( " << "generateFieldNameFromPrefixAndName( p, " << q(typeSqlGenerationPrefix) << " ) ) ); // " << propName << endl;
                                  // nameOrPrefix
                              }
                              else
                              {
                                  //auto expandSpecStr  = cpp::makeExpandString( sqlSpec, sqlFieldSpecWidth );
                                  fout<<"    appendToStringVector( resVec, generateFieldNameFromPrefixAndName( p, "<<q(cpp::formatName(propName,cpp::NameStyle::sqlStyle))<<" ) );"
                                      <<endl;
                              }

                              std::string sqlFieldSpecPropAfter = getSqlModelFieldExtraSpec( sqlSchemaMap, typeName, propName, "after" );
                              if (!sqlFieldSpecPropAfter.empty())
                              {
                                  cerr<<"!!! Found 'after' spec for "<<typeName<<"."<<propName<<endl;
                             
                                  std::string fieldName, fieldSqlSpec;
                                  if (!splitSqlFieldSpec(sqlFieldSpecPropAfter, fieldName, fieldSqlSpec))
                                  {
                                      cerr<<"Failed to parse sql spec 'after' for " << typeName << "." << propName << endl;
                                  }
                                  else
                                  {
                                      fout<<"    appendToStringVector( resVec, generateFieldNameFromPrefixAndName(p, "<<q(fieldName)<<") );"
                                          <<endl;
                                      //auto expandSpecStr  = makeExpandString( fieldSqlSpec, sqlFieldSpecWidth );
                                      //fout << appendToSchemaVecStart << "p + " << q( expandAtBack(fieldName,sqlFieldNameWidth), fieldSqlSpec ) << expandSpecStr << " );" << " // Spec ::schema::" << typeName << "::after::" << propName << endl;
                                  }
                              }



                              //auto cppPropName = cpp::formatName(propName,cpp::NameStyle::cppStyle);
                             
                          }
                        );

        fout << endl << "#endif /* INVEST_OPENAPI_GENERATED_MODEL_TABLE_GET_COLUMN_NAMES_IMPLEMENTED */" << endl;

        fout << "//----------------------------------------------------------------------------" << endl;
        fout << endl 
             << endl 
             << endl
             << endl
             ;
        #endif



        fout << endl << endl << endl << endl << endl;

        const std::string appendToSchemaVecStart = "    appendToStringVector( resVec, ";

        //!!! Fourth iteration - generating 'modelMakeSqlSchema' specializations

        static const std::size_t sqlFieldNameWidth = 34;
        static const std::size_t sqlFieldSpecWidth = 34;


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
            fout<<"template <> inline QVector<QString> modelMakeSqlSchemaStringVector_"<<DBTYPE<<"< " << typeName << " >( const QString &nameOrPrefix, bool forInlining )" << endl;
            fout<< "{" << endl
                //<< "    QVector<QString> resVec;" << endl
                //<< "    QString p = forInlining ? (nameOrPrefix.isEmpty() ? QString() : nameOrPrefix + QString(\"_\")) : QString();" << endl
                << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_PROLOG();" << endl
                << endl
                ;

            // QString generateFieldNameFromPrefixAndName( const QString &prefix, const QString &fieldName )

            using cpp::formatName;
            using cpp::detectNameStyle;
            using cpp::NameStyle;
            using cpp::expandAtBack;
            using cpp::expandAtFront;
            using cpp::makeExpandString;


            auto idSpecInline = getSqlModelIdSpec( sqlSchemaMap, typeName, "inline" );
            auto idSpecSchema = getSqlModelIdSpec( sqlSchemaMap, typeName, "schema" );

            if (!idSpecInline.empty() || !idSpecSchema.empty())
            {
                fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_INLINING_BEGIN() /* if (forInlining) */" << endl
                     // << "    {"
                     ;

                fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_RES_APPEND2( "<< q("ID", sqlFieldNameWidth) << " , " << q(idSpecInline,sqlFieldSpecWidth) << " );" << " // ID spec inline" << endl;

                if (!idSpecInline.empty())
                {
                    fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_INLINING_ELSE()" << endl
                         ;
                }

                if (!idSpecSchema.empty())
                {
                    fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_RES_APPEND2( "<< q("ID", sqlFieldNameWidth) << " , " << q(idSpecSchema,sqlFieldSpecWidth) << " );" << " // ID spec schema" << endl;
                }

                fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_INLINING_END()" << endl;
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
                        cerr<<"Failed to parse sql spec 'before' for " << typeName << "." << propName << endl;
                    }
                    else
                    {
                        // auto expandSpecStr  = makeExpandString( fieldSqlSpec, sqlFieldSpecWidth );
                        // fout << appendToSchemaVecStart << "p + " << q( expandAtBack(fieldName,sqlFieldNameWidth), fieldSqlSpec ) << expandSpecStr << " );" << " // Spec ::schema::" << typeName << "::before::" << propName << endl;
                        // fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_RES_APPEND2( " << fieldName << " , " << fieldSqlSpec << " )" << " // Spec ::schema::" << typeName << "::before::" << propName << endl;
                        fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_RES_APPEND2( "<< q(fieldName, sqlFieldNameWidth) << " , " << q(idSpecSchema,fieldSqlSpec) << " );" << " // Spec ::schema::" << typeName << "::before::" << propName << endl;
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

                    fout << appendToSchemaVecStart << "modelMakeSqlSchemaStringVector_"<<DBTYPE<<"<" << propType << ">( " << "generateFieldNameFromPrefixAndName( p , " << q(typeSqlGenerationPrefix) << " ), true ) ); // " << propName << endl;
                    // nameOrPrefix
                }
                else
                {
                    auto expandSpecStr  = makeExpandString( sqlSpec, sqlFieldSpecWidth );
                    // fout << appendToSchemaVecStart << "p + " << q( expandAtBack(propNameSql,sqlFieldNameWidth), sqlSpec ) << expandSpecStr << " );" << specLookupComment << endl;
                    // fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_RES_APPEND2(" << propNameSql << ", " << sqlSpec << " )" << " // Spec ::schema::" << typeName << "::before::" << propName << endl;
                    fout << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_RES_APPEND2( "<< q(propNameSql, sqlFieldNameWidth) << " , " << q(sqlSpec,sqlFieldSpecWidth) << " );" << specLookupComment << endl;
                }

                std::string sqlFieldSpecPropAfter = getSqlModelFieldExtraSpec( sqlSchemaMap, typeName, propName, "after" );
                if (!sqlFieldSpecPropAfter.empty())
                {
                    cerr<<"!!! Found 'after' spec for "<<typeName<<"."<<propName<<endl;

                    std::string fieldName, fieldSqlSpec;
                    if (!splitSqlFieldSpec(sqlFieldSpecPropAfter, fieldName, fieldSqlSpec))
                    {
                        cerr<<"Failed to parse sql spec 'after' for " << typeName << "." << propName << endl;
                    }
                    else
                    {
                        auto expandSpecStr  = makeExpandString( fieldSqlSpec, sqlFieldSpecWidth );
                        fout << appendToSchemaVecStart << "generateFieldNameFromPrefixAndName( p , " << q( expandAtBack(fieldName,sqlFieldNameWidth), fieldSqlSpec ) << expandSpecStr << " ) );" << " // Spec ::schema::" << typeName << "::after::" << propName << endl;
                    }
                }
                
            }

            fout << endl
                 << "    IOA_MODEL_TO_STRINGS_MODEL_MAKE_SQL_SCHEMA_STRING_VECTOR_EPILOG();" << endl
                 ;

            fout<< "}" << endl ;

        } //!!! Fourth iteration - generating 'modelMakeSqlSchema' specializations


        fout << endl << endl
             << "inline QMap<QString,QString> modelMakeAllSqlSchemas_"<<DBTYPE<<"()" << endl
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

            fout << "    resMap[ \"" << tableNameSql << "\"" << expandSqlStr << " ] " << " = modelMakeSqlCreateTableSchema_"<<DBTYPE<<"( modelMakeSqlSchemaStringVector_"<<DBTYPE<<"< " << typeName << expandTypeStr << " >( QString(), false ) " << "); " << endl;

            //fout<<"template <> inline QVector<QString> modelMakeSqlSchemaStringVector< " << typeName << " >( const QString &nameOrPrefix )" << endl;
        }

        fout << endl << "    return resMap;" << endl
             << "}" << endl << endl << endl;


        fout << endl << endl
             << "inline QSet<QString> modelMakeAllSqlTablesSet_"<<DBTYPE<<"()" << endl
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

        /*
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
        */
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



