/**
 */

/*
 * GenericStreamingResponse.h
 *
 * 
 */

#ifndef GenericStreamingResponse_H
#define GenericStreamingResponse_H

#include <QJsonObject>

#include <QString>
#include <QDateTime>

#include "..\models\client\Enum.h"
#include "..\models\client\Object.h"

namespace OpenAPI {

class GenericStreamingResponse : public Object {
public:
    GenericStreamingResponse();
    GenericStreamingResponse(QString json);
    ~GenericStreamingResponse() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    //------------------------------

    QString getEvent() const;
    void setEvent(const QString &event);
    bool is_event_Set() const;
    bool is_event_Valid() const;

    QDateTime getTime() const;
    void setTime(const QDateTime &time);
    bool is_time_Set() const;
    bool is_time_Valid() const;

    //------------------------------

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    //------------------------------

    QString event;
    bool m_event_isSet;
    bool m_event_isValid;

    QDateTime time;
    bool m_time_isSet;
    bool m_time_isValid;

    //------------------------------

};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::GenericStreamingResponse)

#endif // GenericStreamingResponse_H
