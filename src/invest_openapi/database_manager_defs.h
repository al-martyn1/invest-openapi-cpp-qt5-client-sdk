#pragma once

#define RETURN_IOA_SQL_EXEC_QUERY( queru, querySqlText )                                         \
            do                                                                                   \
            {                                                                                    \
                 /* qDebug().nospace().noquote() << "Executing query: " << querySqlText; */      \
                bool res = (query).exec(querySqlText);                                           \
                if (!res)                                                                        \
                {                                                                                \
                    qDebug().nospace().noquote() << "Error      : " << (query).lastError();      \
                    qDebug().nospace().noquote() << "Query text : " << querySqlText;             \
                    return false;                                                                \
                }                                                                                \
                else                                                                             \
                {                                                                                \
                     /* qDebug().nospace().noquote() << "Done"; */                               \
                    return true;                                                                 \
                }                                                                                \
                                                                                                 \
            } while(0)
