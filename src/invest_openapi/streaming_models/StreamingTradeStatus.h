/**
 */

/*
 * StreamingTradeStatus.h
 *
 * 
 */

#ifndef StreamingTradeStatus_H
#define StreamingTradeStatus_H

#include <QJsonObject>


#include "../models/client/Enum.h"
#include "../models/client/Object.h"

namespace OpenAPI {

class StreamingTradeStatus : public Enum {
public:
    StreamingTradeStatus();
    StreamingTradeStatus(QString json);
    ~StreamingTradeStatus() override;

    QString asJson() const override;
    QJsonValue asJsonValue() const override;
    void fromJsonValue(QJsonValue json) override;
    void fromJson(QString jsonString) override;

    enum class eStreamingTradeStatus {
        INVALID_VALUE_OPENAPI_GENERATED = 0,
        NORMALTRADING, 
        NOTAVAILABLEFORTRADING
    };
    StreamingTradeStatus::eStreamingTradeStatus getValue() const;
    void setValue(const StreamingTradeStatus::eStreamingTradeStatus& value);
    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    eStreamingTradeStatus m_value;
    bool m_value_isSet;
    bool m_value_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::StreamingTradeStatus)

#endif // StreamingTradeStatus_H
