/**
 */

/*
 * StreamingError_payload.h
 *
 * 
 */

#ifndef StreamingError_payload_H
#define StreamingError_payload_H

#include <QJsonObject>

#include <QString>

#include "../models/client/Enum.h"
#include "../models/client/Object.h"

namespace OpenAPI {

class StreamingError_payload : public Object {
public:
    StreamingError_payload();
    StreamingError_payload(QString json);
    ~StreamingError_payload() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getMessage() const;
    void setMessage(const QString &message);
    bool is_message_Set() const;
    bool is_message_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString message;
    bool m_message_isSet;
    bool m_message_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::StreamingError_payload)

#endif // StreamingError_payload_H
