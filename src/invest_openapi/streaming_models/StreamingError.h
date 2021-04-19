/**
 */

/*
 * StreamingError.h
 *
 * 
 */

#ifndef StreamingError_H
#define StreamingError_H

#include <QJsonObject>

#include "StreamingError_payload.h"
#include <QString>
#include <QDateTime>

#include "../models/client/Enum.h"
#include "../models/client/Object.h"

namespace OpenAPI {

class StreamingError : public Object {
public:
    StreamingError();
    StreamingError(QString json);
    ~StreamingError() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    StreamingError_payload getPayload() const;
    void setPayload(const StreamingError_payload &payload);
    bool is_payload_Set() const;
    bool is_payload_Valid() const;

    QString getEvent() const;
    void setEvent(const QString &event);
    bool is_event_Set() const;
    bool is_event_Valid() const;

    QDateTime getTime() const;
    void setTime(const QDateTime &time);
    bool is_time_Set() const;
    bool is_time_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    StreamingError_payload payload;
    bool m_payload_isSet;
    bool m_payload_isValid;

    QString event;
    bool m_event_isSet;
    bool m_event_isValid;

    QDateTime time;
    bool m_time_isSet;
    bool m_time_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::StreamingError)

#endif // StreamingError_H
