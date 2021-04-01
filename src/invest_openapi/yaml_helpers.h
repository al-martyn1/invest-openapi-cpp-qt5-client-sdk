#pragma once

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

#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep

#include "qt_helpers.h"




namespace invest_openapi
{

//----------------------------------------------------------------------------
bool hasPayloadProperty( const std::string &typeName, const YAML::Node &typeInfoNode )
{
    using std::cout;
    using std::cerr;
    using std::endl;

    try
    {
        cerr << typeName << ".hasPayloadProperty (1)" << endl;
        const YAML::Node &propsNode   = typeInfoNode["properties"];
        if (propsNode.Type()!=YAML::NodeType::Map)
        {
            cerr << typeName << " - 'properties' is not a map" << endl;
            //cerr << typeName << " - no 'properties'" << endl;
            cerr << typeName << ".hasPayloadProperty - false" << endl << endl;
            return false;
        }

        cerr << typeName << ".hasPayloadProperty (2)" << endl;
        const YAML::Node &payloadNode = propsNode["payload"];

        YAML::NodeType::value typeInfoNodeType = typeInfoNode.Type();
        cerr << typeName << ", typeInfoNode - type: " << toString(typeInfoNodeType) << endl;

        YAML::NodeType::value payloadNodeType = payloadNode.Type();
        cerr << typeName << ", payloadNode - type: " << toString(payloadNodeType) << endl;

        if (propsNode.Type()!=YAML::NodeType::Map)
        {
            cerr << typeName << " - no 'payload'" << endl;
            cerr << typeName << ".hasPayloadProperty - false" << endl << endl;
            return false;
        }

        cerr << typeName << ".hasPayloadProperty - true" << endl << endl;
        return true;
    }
    catch (const YAML::Exception& e)
    {
        cerr << "YAML parsing exception: " << e.what() << endl;
        cerr << typeName << ".hasPayloadProperty - false" << endl << endl;
        return false;
    }
    catch(...)
    {
        cerr << "YAML parsing unknown exception" << endl;
        cerr << typeName << ".hasPayloadProperty - false" << endl << endl;
        return false;
    }
}

//----------------------------------------------------------------------------
YAML::Node getPayloadOrPropertiesItself( const std::string &typeName, const YAML::Node &typeInfoNode )
{
    YAML::Node propsNode   = typeInfoNode["properties"];

    if (!hasPayloadProperty(typeName, typeInfoNode))
        return propsNode;

    return propsNode["payload"];
}

//----------------------------------------------------------------------------


} // namespace invest_openapi


