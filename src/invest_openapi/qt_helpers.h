#pragma once


inline
//QString toString( QNetworkReply::NetworkError &err)
QString toString( QNetworkReply &err)
{
    return err.errorString();
}

#if 0
inline
QString toString( YAML::NodeType::value y)
{
    switch(y)
    {
        case YAML::NodeType /* ::value */ ::Undefined  : return "Undefined";
        case YAML::NodeType /* ::value */ ::Null       : return "Null";
        case YAML::NodeType /* ::value */ ::Scalar     : return "Scalar";
        case YAML::NodeType /* ::value */ ::Sequence   : return "Sequence";
        case YAML::NodeType /* ::value */ ::Map        : return "Map";
    }
    return  QString /* std::string */ ("toString( YAML::NodeType ): node type unknown");
}
#endif


