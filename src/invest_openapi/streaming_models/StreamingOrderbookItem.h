#ifndef StreamingOrderbookItem_H
#define StreamingOrderbookItem_H

#include <QJsonObject>
#include <QList>

#include <exception>
#include <stdexcept>

#include "../marty_decimal.h"

#include "StreamingOrderResponse.h"


namespace OpenAPI {


struct StreamingOrderbookItem
{
    marty::Decimal   price;
    marty::Decimal   quantity;

    static StreamingOrderbookItem fromList( const QList< marty::Decimal > &list )
    {
        if (list.size()<2)
            throw std::runtime_error("StreamingOrderbookItem::fromList: Invalind list size");

        StreamingOrderbookItem res;

        res.price     = list[0];
        res.quantity  = list[1];

        return res;

    }

    QList< marty::Decimal > toList() const
    {
        QList< marty::Decimal > res;

        res.push_back( price );
        res.push_back( quantity );

        return res;
    }

};


} // namespace OpenAPI


#endif // StreamingOrderbookItem_H
