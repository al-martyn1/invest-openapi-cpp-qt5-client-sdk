/*! \file
    \brief Configs lookup test

 */

#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <iomanip>
#include <vector>
#include <cstring>
#include <string>

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



template <typename StringType> inline bool ends_with( const StringType &str, const StringType &postfix )
{
    if (str.size()<postfix.size())
        return false;

    return str.compare( str.size()-postfix.size(), postfix.size(), postfix )==0;
}


bool isResponseType( const std::string &str )
{
    if (ends_with(str, std::string("Response")))
        return true;
    return false;
}

bool isRequestType( const std::string &str )
{
    if (ends_with(str, std::string("Request")))
        return true;
    return false;
}



YAML::Node parse(std::istream& input)
{
    return YAML::Load(input);
    //std::cout << doc << "\n";
  //} catch (const YAML::Exception& e) {
  //  std::cerr << e.what() << "\n";
  //}
}

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

inline
bool isRefTypeName( const std::string &typeName )
{
    if (typeName.empty())
        return false;

    return typeName[0] == '#';
}

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


bool isProperyTypeArray( const std::string &propType )
{
    return propType=="array";
}

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



bool isSequenceOrMap(YAML::NodeType::value v)
{
    switch(v)
    {
        case YAML::NodeType::Sequence : 
        case YAML::NodeType::Map      : return true;
        default                       : return false;
    }
}


bool isTagMeanfull( const std::string &tagName )
{
    if (tagName=="?" || tagName=="!")
        return false;
    return true;
}

bool hasPayloadPropery( const std::string &typeName, const YAML::Node &typeInfoNode )
{
    using std::cout;
    using std::cerr;
    using std::endl;

    try
    {
        cerr << typeName << ".hasPayloadPropery (1)" << endl;
        const YAML::Node &propsNode   = typeInfoNode["properties"];
        if (propsNode.Type()!=YAML::NodeType::Map) // if (!propsNode.IsDefined() || !propsNode.IsNull())
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

        if (propsNode.Type()!=YAML::NodeType::Map) // if (!payloadNode.IsDefined() || !payloadNode.IsNull())
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

    if (argc<2)
    {
        std::cerr<<"No input file taken";
        return -1;
    }

    try
    {
        YAML::Node rootNode = parse(argv[1]);

        if (argc>3)
        {
            QVariant qvDepth = 1;

            if (argc>4)
            {
                QLatin1String arg4latin1 = QLatin1String( argv[4] );
                qvDepth = arg4latin1;
            }

            int depth = qvDepth.toInt();
            depth++;

            std::string subNodeName = argv[3];

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
        if (std::strcmp(argv[2],"-")!=0)
        {
            foutStream.open(argv[2]);
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

        YAML::Node::const_iterator typeIt = schemasNode.begin();
        for (; typeIt!=schemasNode.end(); ++typeIt)
        {
            //fout << "Type: " << it->first << endl;
            std::string typeName = typeIt->first.as<std::string>();

            if (isRequestType(typeName))
            {
                cerr<<"Skip request type: " << typeName << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }

            if (isResponseType(typeName))
            {
                cerr<<"Skip response type: " << typeName << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }


            if (hasPayloadPropery(typeName, typeIt->second))
            {
                cerr<<"Skip type with payload: " << typeName << endl << endl << endl;
                skippingSet.insert(typeName);
                continue;
            }

            if (skipNoTypeOrRefOrArray( typeName, typeIt->second["properties"] ))
            {
                skippingSet.insert(typeName);
                continue;
            }

            fout<<"QVector<QString> modelToStringVector( const " << typeName << " &v );" << endl;
        }

        fout << "//----------------------------------------------------------------------------" << endl;
        fout << endl << endl << endl; // << endl;
        //fout << "//----------------------------------------------------------------------------" << endl;


        std::map< std::string, std::string > usedPropTypes;

        typeIt = schemasNode.begin();
        for (; typeIt!=schemasNode.end(); ++typeIt)
        {
            //fout << "Type: " << it->first << endl;
            std::string typeName = typeIt->first.as<std::string>();

            if (skippingSet.find(typeName)!=skippingSet.end())
                continue;


            const YAML::Node &propertiesNode = typeIt->second["properties"];

            if (skipNoTypeOrRefOrArray( typeName, typeIt->second["properties"] ))
            {
                continue;
            }

            fout << endl << "//----------------------------------------------------------------------------" << endl;
            fout<< "QVector<QString> modelToStringVector( const " << typeName << " &v )" << endl
                << "{" << endl
                << "    QVector<QString> resVec;" 
                << endl
                ;

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
                     << "    if ( !v.is_" << cpp::formatName(propName,cpp::NameStyle::cppStyle) << "_Set() " << endl
                     << "      || !v.is_" << cpp::formatName(propName,cpp::NameStyle::cppStyle) << "_Valid()" << endl
                     << "       )" << endl
                     << "    {"    << endl
                     << "        appendToStringVector(resVec, QString());" << endl
                     << "    }"    << endl
                     << "    else" << endl
                     << "    {"    << endl
                     << "        appendToStringVector(resVec, v.get" << cpp::formatName(propName,cpp::NameStyle::pascalStyle) << "());" << endl
                     << "    }"    << endl
                     ;

                //fout << "appendToStringVector( resVec,  )";

                // 
                /*
                    bool isRefTypeName( const std::string &typeName )
                    std::string extractTypeNameFromRef( const std::string &typeName )

                    qint32

                    marty::Decimal getMinPriceIncrement() const;
                    void setMinPriceIncrement(const marty::Decimal &min_price_increment);
                    bool is_min_price_increment_Set() const;
                    bool is_min_price_increment_Valid() const;
                
                */


                // usedPropTypes
                // bool getProperyTypeFormat( const YAML::Node &propProperties, std::string &propTypeFormat )

                //fout << "    " << propName << "    " << propType << endl << flush;

            }

            fout << endl
                 << "    //------------------------------" << endl
                 << "    return resVec;" << endl
                 ;

            fout<< "}" << endl ;

        }


        fout<<"} // namespace invest_openapi"           <<endl;


        cerr << endl;
        cerr << "Used types:" << endl;

        std::map< std::string, std::string >::const_iterator usedIt = usedPropTypes.begin();
        for(; usedIt != usedPropTypes.end(); ++usedIt)
        {
            std::string typeName = usedIt->first;
            if (isRefTypeName(typeName))
                typeName = extractTypeNameFromRef(typeName);
            
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



