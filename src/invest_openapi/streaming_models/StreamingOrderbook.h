/**
 */

/*
 * StreamingOrderbook.h
 *
 * 
 */

#ifndef StreamingOrderbook_H
#define StreamingOrderbook_H

#include <QJsonObject>

#include "..\models\client\OrderResponse.h"
#include <QList>
#include <QString>

#include "..\models\client\Enum.h"
#include "..\models\client\Object.h"

#include "../marty_decimal.h"


namespace OpenAPI {

class StreamingOrderbook : public Object {
public:
    StreamingOrderbook();
    StreamingOrderbook(QString json);
    ~StreamingOrderbook() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getFigi() const;
    void setFigi(const QString &figi);
    bool is_figi_Set() const;
    bool is_figi_Valid() const;

    qint32 getDepth() const;
    void setDepth(const qint32 &depth);
    bool is_depth_Set() const;
    bool is_depth_Valid() const;

    QList<OrderResponse> getBids() const;
    void setBids(const QList<OrderResponse> &bids);
    bool is_bids_Set() const;
    bool is_bids_Valid() const;

    QList<OrderResponse> getAsks() const;
    void setAsks(const QList<OrderResponse> &asks);
    bool is_asks_Set() const;
    bool is_asks_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString figi;
    bool m_figi_isSet;
    bool m_figi_isValid;

    qint32 depth;
    bool m_depth_isSet;
    bool m_depth_isValid;

    QList<OrderResponse> bids;
    bool m_bids_isSet;
    bool m_bids_isValid;

    QList<OrderResponse> asks;
    bool m_asks_isSet;
    bool m_asks_isValid;

};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::StreamingOrderbook)

#endif // StreamingOrderbook_H
