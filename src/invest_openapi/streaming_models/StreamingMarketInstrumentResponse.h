/**
 */

/*
 * StreamingMarketInstrumentResponse.h
 *
 * 
 */

#ifndef StreamingMarketInstrumentResponse_H
#define StreamingMarketInstrumentResponse_H

#include <QJsonObject>

#include "StreamingMarketInstrument.h"
#include <QString>
#include <QDateTime>

#include "../models/client/Enum.h"
#include "../models/client/Object.h"

#include "../models/client/Candle.h"

namespace OpenAPI {

class StreamingMarketInstrumentResponse : public Object {
public:
    StreamingMarketInstrumentResponse();
    StreamingMarketInstrumentResponse(QString json);
    ~StreamingMarketInstrumentResponse() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getEvent() const;
    void setEvent(const QString &event);
    bool is_event_Set() const;
    bool is_event_Valid() const;

    QDateTime getTime() const;
    void setTime(const QDateTime &time);
    bool is_time_Set() const;
    bool is_time_Valid() const;

    StreamingMarketInstrument getPayload() const;
    void setPayload(const StreamingMarketInstrument &payload);
    bool is_payload_Set() const;
    bool is_payload_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString event;
    bool m_event_isSet;
    bool m_event_isValid;

    QDateTime time;
    bool m_time_isSet;
    bool m_time_isValid;

    StreamingMarketInstrument payload;
    bool m_payload_isSet;
    bool m_payload_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::StreamingMarketInstrumentResponse)

#endif // StreamingMarketInstrumentResponse_H
