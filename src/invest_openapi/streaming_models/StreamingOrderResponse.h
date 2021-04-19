/**
 */

/*
 * StreamingOrderResponse.h
 *
 * 
 */

#ifndef StreamingOrderResponse_H
#define StreamingOrderResponse_H

#include <QJsonObject>
#include <QList>


#include "../models/client/Enum.h"
#include "../models/client/Object.h"

#include "../marty_decimal.h"


namespace OpenAPI {

class StreamingOrderResponse : public Object {
public:
    StreamingOrderResponse();
    StreamingOrderResponse(QString json);
    ~StreamingOrderResponse() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QList<marty::Decimal> getValues() const;
    void setValues(const QList<marty::Decimal> &values);
    bool is_values_Set() const;
    bool is_values_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QList<marty::Decimal> values;
    bool m_values_isSet;
    bool m_values_isValid;

};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::StreamingOrderResponse)

#endif // StreamingOrderResponse_H
