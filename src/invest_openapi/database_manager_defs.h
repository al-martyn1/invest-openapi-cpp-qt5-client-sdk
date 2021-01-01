#pragma once

#define RETURN_IOA_SQL_EXEC_QUERY( queru, querySqlText )                                         \
            do                                                                                   \
            {                                                                                    \
                if (m_pLoggingConfig->debugSqlQueries)                                           \
                {                                                                                \
                    qDebug().nospace().noquote() << "Executing query:";                          \
                    qDebug().nospace().noquote() << querySqlText;                                \
                }                                                                                \
                                                                                                 \
                bool res = (query).exec(querySqlText);                                           \
                if (!res)                                                                        \
                {                                                                                \
                    qDebug().nospace().noquote() << "Error      : " << (query).lastError();      \
                    if (!m_pLoggingConfig->debugSqlQueries) /* Do not duplicate logging if it logged above */ \
                    {                                                                            \
                        qDebug().nospace().noquote() << "Query text :";                          \
                        qDebug().nospace().noquote() << querySqlText  ;                          \
                    }                                                                            \
                    return false;                                                                \
                }                                                                                \
                else                                                                             \
                {                                                                                \
                    return true;                                                                 \
                }                                                                                \
                                                                                                 \
            } while(0)


/* Mostly same as RETURN_IOA_SQL_EXEC_QUERY */
#define IOA_SQL_EXEC_QUERY( queru, querySqlText )                                                \
            do                                                                                   \
            {                                                                                    \
                if (m_pLoggingConfig->debugSqlQueries)                                           \
                {                                                                                \
                    qDebug().nospace().noquote() << "Executing query:";                          \
                    qDebug().nospace().noquote() << querySqlText;                                \
                }                                                                                \
                                                                                                 \
                bool res = (query).exec(querySqlText);                                           \
                if (!res)                                                                        \
                {                                                                                \
                    qDebug().nospace().noquote() << "Error      : " << (query).lastError();      \
                    if (!m_pLoggingConfig->debugSqlQueries) /* Do not duplicate logging if it logged above */ \
                    {                                                                            \
                        qDebug().nospace().noquote() << "Query text :";                          \
                        qDebug().nospace().noquote() << querySqlText  ;                          \
                    }                                                                            \
                     /* return false; */                                                         \
                }                                                                                \
                else                                                                             \
                {                                                                                \
                     /* return true; */                                                          \
                }                                                                                \
                                                                                                 \
            } while(0)

