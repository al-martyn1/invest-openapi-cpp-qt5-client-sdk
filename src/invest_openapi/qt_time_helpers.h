#pragma once

#include <QTimeZone>
#include <QDate>
#include <QTime>
#include <QString>
#include <QByteArray>
#include <QDateTime>

#include <vector>
#include <string>

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
namespace qt_helpers
{



//----------------------------------------------------------------------------
inline
const std::map<QString, QByteArray>&
getIanaTimezoneAliases()
{
    static std::map<QString, QByteArray> shortcuts;

    if (!shortcuts.empty())
       return shortcuts;

    // https://en.wikipedia.org/wiki/Time_in_Russia
    shortcuts["RTZ1" ] = QByteArray::fromStdString("Europe/Kaliningrad");
    shortcuts["RTZ2" ] = QByteArray::fromStdString("Europe/Moscow");
    shortcuts["RTZ3" ] = QByteArray::fromStdString("Europe/Samara");
    shortcuts["RTZ4" ] = QByteArray::fromStdString("Asia/Yekaterinburg");
    shortcuts["RTZ5" ] = QByteArray::fromStdString("Asia/Omsk");
    shortcuts["RTZ6" ] = QByteArray::fromStdString("Asia/Krasnoyarsk");
    shortcuts["RTZ7" ] = QByteArray::fromStdString("Asia/Irkutsk");
    shortcuts["RTZ8" ] = QByteArray::fromStdString("Asia/Yakutsk");
    shortcuts["RTZ9" ] = QByteArray::fromStdString("Asia/Vladivostok");
    shortcuts["RTZ10"] = QByteArray::fromStdString("Asia/Srednekolymsk"); // Magadan
    shortcuts["RTZ11"] = QByteArray::fromStdString("Asia/Kamchatka"); // Chukotka and Kamchatka Krai

    shortcuts["KALT" ] = QByteArray::fromStdString("Europe/Kaliningrad");
    shortcuts["MSK"  ] = QByteArray::fromStdString("Europe/Moscow");
    shortcuts["SAMT" ] = QByteArray::fromStdString("Europe/Samara");
    shortcuts["YEKT" ] = QByteArray::fromStdString("Asia/Yekaterinburg");
    shortcuts["OMST" ] = QByteArray::fromStdString("Asia/Omsk");
    shortcuts["KRAT" ] = QByteArray::fromStdString("Asia/Krasnoyarsk");
    shortcuts["IRKT" ] = QByteArray::fromStdString("Asia/Irkutsk");
    shortcuts["YAKT" ] = QByteArray::fromStdString("Asia/Yakutsk");
    shortcuts["VLAT" ] = QByteArray::fromStdString("Asia/Vladivostok");
    shortcuts["MAGT" ] = QByteArray::fromStdString("Asia/Srednekolymsk"); // Magadan
    shortcuts["PETT" ] = QByteArray::fromStdString("Asia/Kamchatka"); // Chukotka and Kamchatka Krai

    shortcuts["MSK-1"] = QByteArray::fromStdString("Europe/Kaliningrad");
    shortcuts["MSK+0"] = QByteArray::fromStdString("Europe/Moscow");
    shortcuts["MSK+1"] = QByteArray::fromStdString("Europe/Samara");
    shortcuts["MSK+2"] = QByteArray::fromStdString("Asia/Yekaterinburg");
    shortcuts["MSK+3"] = QByteArray::fromStdString("Asia/Omsk");
    shortcuts["MSK+4"] = QByteArray::fromStdString("Asia/Krasnoyarsk");
    shortcuts["MSK+5"] = QByteArray::fromStdString("Asia/Irkutsk");
    shortcuts["MSK+6"] = QByteArray::fromStdString("Asia/Yakutsk");
    shortcuts["MSK+7"] = QByteArray::fromStdString("Asia/Vladivostok");
    shortcuts["MSK+8"] = QByteArray::fromStdString("Asia/Srednekolymsk"); // Magadan
    shortcuts["MSK+9"] = QByteArray::fromStdString("Asia/Kamchatka"); // Chukotka and Kamchatka Krai


    // List of time zones by country
    // https://en.wikipedia.org/wiki/List_of_time_zones_by_country

    shortcuts["GMT"  ] = QByteArray::fromStdString("Europe/London"); // GMT - время по Гринвичу - (UTC+00:00) Дублин, Эдинбург, Лиссабон, Лондон
    shortcuts["CET"  ] = QByteArray::fromStdString("Europe/Berlin"     ); // Западная Европа - (UTC+01:00) Амстердам, Берлин, Берн, Вена, Рим, Стокгольм
    shortcuts["WET"  ] = QByteArray::fromStdString("Europe/Lisbon"); // Same as GMT, Western European Summer Time (WEST, UTC+01:00) is a summer daylight saving time scheme
    shortcuts["EET"  ] = QByteArray::fromStdString("Europe/Chisinau"); // Eastern European Time - Восточная Европа - (UTC+02:00) Кишинев
    //shortcuts["EET"  ] = QByteArray::fromStdString("Europe/Athens"); // EET Eastern European Time, Греция, Турция - (UTC+02:00) Афины, Бухарест - (UTC+02:00)

    // https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
    // Можно будет попробовать таймзону по коду страны определять
    // Currently used in Qt but deprecated
    // EST5EDT - deprecated - Use America/New_York - Восточное время США (зима) - (UTC-05:00) Восточное время (США и Канада) - (UTC-05:00) Восточное время (США и Канада) -  Has DST: Yes
    // EST     - deprecated - Use America/Cancun - Восточн. поясн. время (Мексика) - (UTC-05:00) Четумаль - (UTC-05:00) Четумаль -  Has DST: Yes

    shortcuts["PST"  ] = QByteArray::fromStdString("PST8PDT"); // Тихоокеанское время США
    shortcuts["MST"  ] = QByteArray::fromStdString("MST7MDT"); // Горное время США
    shortcuts["CST"  ] = QByteArray::fromStdString("CST6CDT"); // Центральное время США
    shortcuts["EST"  ] = QByteArray::fromStdString("EST5EDT"); // Восточное время США

    shortcuts["PT"   ] = QByteArray::fromStdString("PST8PDT"); // Pacific Time zone: the Pacific coast states and most of Nevada
    shortcuts["MT"   ] = QByteArray::fromStdString("MST7MDT"); // Mountain Time zone: the Mountain states plus western parts of some adjacent states
    shortcuts["CT"   ] = QByteArray::fromStdString("CST6CDT"); // Central Time zone: a large area spanning from the Gulf Coast to the Great Lakes
    shortcuts["ET"   ] = QByteArray::fromStdString("EST5EDT"); // Eastern Time zone: roughly a triangle covering all the states from the Great Lakes down to Florida and east to the Atlantic coast
    shortcuts["AST"  ] = QByteArray::fromStdString("America/Puerto_Rico"); // Puerto Rico, the U.S. Virgin Islands, Ю-Ам. западное время - (UTC-04:00) Джорджтаун, Ла-Пас, Манаус, Сан-Хуан

    shortcuts["AT"   ] = QByteArray::fromStdString("America/Goose_Bay"); // (UTC-04:00) Атлантическое время (Канада), Labrador (all but southeastern tip), New Brunswick, Nova Scotia, Prince Edward Island, eastern part of Quebec
    shortcuts["NT"   ] = QByteArray::fromStdString("America/St_Johns"); // (UTC-03:30) Ньюфаундленд, Labrador (southeastern), Newfoundland
    shortcuts["AKT"  ] = QByteArray::fromStdString("America/Anchorage" ); // most of the state of Alaska, Аляскинское время (UTC-09:00) Аляска
    //shortcuts[""  ] = QByteArray::fromStdString(""); // 


    shortcuts["AST"  ] = QByteArray::fromStdString("America/Guadeloupe"); // Guadeloupe, Martinique, Saint Barthélemy, Saint Martin, (UTC-04:00) Джорджтаун, Ла-Пас, Манаус, Сан-Хуан
    shortcuts["PMST" ] = QByteArray::fromStdString("America/Miquelon"  ); // French Guiana, Saint Pierre and Miquelon, (UTC-03:00) Сен-Пьер и Микелон
    shortcuts["ST"   ] = QByteArray::fromStdString("Pacific/Midway"    ); // American Samoa, Jarvis Island, Kingman Reef, Midway Atoll and Palmyra Atoll
    shortcuts["HT"   ] = QByteArray::fromStdString("Pacific/Johnston"  ); // Hawaii, most of the Aleutian Islands, and Johnston Atoll, Гавайское время - (UTC-10:00)

    shortcuts["GALT" ] = QByteArray::fromStdString("America/Guatemala"); // Центральная Америка - (UTC-06:00), Galápagos Province, Galápagos Time (GALT) is the time observed since 1986 in the province of Galápagos, which is coterminous with the Región Insular (Insular Region) of western Ecuador. Galápagos Time is at UTC-06:00. The rest of Ecuador, the mainland, observes Ecuador Time.

    shortcuts["ART"  ] = QByteArray::fromStdString("America/Argentina/Salta"); // Аргентина - (UTC-03:00) Буэнос-Айрес

    shortcuts["PET"  ] = QByteArray::fromStdString("America/Lima"); // Ю-Ам. тихоокеанское вр. (зима) - (UTC-05:00) Богота, Кито, Лима, Рио-Бранко. Peru Time (PET) is the official time in Peru. I

    shortcuts["ART"  ] = QByteArray::fromStdString("Antarctica/Palmer"); // Antarctica/Rothera, Palmer Station, Rothera Station, Antarctica, Ю-Ам. восточное время - (UTC-03:00) Кайенна, Форталеза

    shortcuts["CXT"  ] = QByteArray::fromStdString("Indian/Christmas"); // UTC+07:00 (CXT) — Christmas Island, Юго-Восточная Азия (зима) - (UTC+07:00) Бангкок, Джакарта, Ханой

    shortcuts["AWST" ] = QByteArray::fromStdString("Australia/Perth"); // UTC+08:00 (AWST) — Western Australia, Indian Pacific railway when travelling between Port Augusta, South Australia and Kalgoorlie, Western Australia, Западная Австралия - (UTC+08:00) Перт
    shortcuts["CWT"  ] = QByteArray::fromStdString("Australia/Eucla"); // Ц.-зап. Австралия (зима) - (UTC+08:45) Юкла, South Australia (Border Village), Western Australia (Caiguna, Cocklebiddy, Eucla, Madura, Mundrabilla)
    shortcuts["ACST" ] = QByteArray::fromStdString("Australia/Darwin"); // South Australia, Northern Territory, New South Wales (Yancowinna County), Центральная Австралия - (UTC+09:30) Дарвин
    shortcuts["AEST" ] = QByteArray::fromStdString("Australia/Brisbane"); // Queensland, New South Wales, Australian Capital Territory, Victoria, Tasmania, Восточная Австралия - (UTC+10:00) Брисбен
    shortcuts["NFT"  ] = QByteArray::fromStdString("Pacific/Norfolk"); // Norfolk Island, Норфолк - (UTC+11:00) Остров Норфолк

    // Шутка. Бг-г-г ;)
    // В бытность студентом, был возле ВУЗа кабак "Бугенвилия". Бывало, прогуливал там лекции. Иногда даже в ВУЗане заходил, сразу туда шел :)
    shortcuts["BG"   ] = QByteArray::fromStdString("Pacific/Bougainville"); // Бугенвиль - (UTC+11:00) Остров Бугенвиль
    shortcuts["BGG"  ] = QByteArray::fromStdString("Pacific/Bougainville"); // Бугенвиль - (UTC+11:00) Остров Бугенвиль
    shortcuts["BGGG" ] = QByteArray::fromStdString("Pacific/Bougainville");

    shortcuts["WAT"  ] = QByteArray::fromStdString("Africa/Algiers"); // West Africa Time, Западная Центр. Африка - (UTC+01:00), West Africa Time, or WAT, is a time zone used in west-central Africa.[1] West Africa Time is one hour ahead of Coordinated Universal Time (UTC+01:00), which makes it the same as Central European Time (CET) during winter, or Western European Summer Time (WEST) / British Summer Time (BST) during the summer.
    shortcuts["CAT"  ] = QByteArray::fromStdString("Africa/Cairo"); // Africa/Cairo - Египетское время - (UTC+02:00), Central Africa Time, or CAT, is a time zone used in central and southern Africa. Central Africa Time is two hours ahead of Coordinated Universal Time (UTC+02:00), which is the same as the adjacent South Africa Standard Time, Egypt Standard Time, Eastern European Time, Kaliningrad Time and Central European Summer Time.
    shortcuts["SAST" ] = QByteArray::fromStdString("Africa/Harare"); // Южная Африка - (UTC+02:00) Хараре, Претория, South African Standard Time (SAST) is the time zone used by all of South Africa as well as Eswatini and Lesotho. 
    shortcuts["EAT"  ] = QByteArray::fromStdString("Africa/Djibouti"); // Восточная Африка - (UTC+03:00) Найроби, Comoros, Djibouti, Eritrea, Ethiopia, Kenya, Madagascar, Somalia, Somaliland (disputed territory), Tanzania, Uganda, East Africa Time, or EAT, is a time zone used in eastern Africa. The time zone is three hours ahead of UTC (UTC+03:00), which is the same as Moscow Time, Arabia Standard Time, Further-eastern European Time and Eastern European Summer Time.[1]

    shortcuts["TRT"  ] = QByteArray::fromStdString("Europe/Istanbul"); // Турция (UTC+03:00) - Стамбул. Time in Turkey is given by UTC+03:00 year-round. This time is also called Turkey Time (TRT). The time is currently same as in the Arabia Standard Time and Moscow Time zone. 

    /// Same as India. Hm-m-m...
    // shortcuts["IST"  ] = QByteArray::fromStdString("Asia/Jerusalem"); // Иерусалимское время - (UTC+02:00) Иерусалим. Israel Standard Time (IST) is the standard time zone in Israel. It is two hours ahead of UTC (UTC+02:00).

    shortcuts["IRST" ] = QByteArray::fromStdString("Asia/Tehran"); // Иранское время - (UTC+03:30) Тегеран. Iran Standard Time (IRST) or Iran Time (IT) is the time zone used in Iran. Iran uses a UTC offset UTC+03:30. IRST is defined by the 52.5 degrees east meridian, the same meridian which defines the Iranian calendar and is the official meridian of Iran.
    shortcuts["IT"   ] = QByteArray::fromStdString("Asia/Tehran"); // 

    shortcuts["PKT"  ] = QByteArray::fromStdString("Asia/Karachi"); // Пакистан - (UTC+05:00) Исламабад, Карачи. Pakistan Standard Time (abbreviated as PKT) is UTC+05:00 hours ahead of Coordinated Universal Time. The time zone is in use during standard time in Asia.

    shortcuts["BTT"  ] = QByteArray::fromStdString("Asia/Thimphu"); // Бангладеш - (UTC+06:00) Дакка. Bhutan Time (BTT) is the time zone of Bhutan

    // Same as Israel. Hm-m-m...
    shortcuts["IST"  ] = QByteArray::fromStdString("Asia/Calcutta"); // Индийское время - (UTC+05:30) Колката, Мумбаи, Нью-Дели, Ченнай. Indian Standard time (IST) is the time zone observed throughout India, with a time offset of UTC+05:30. India does not observe daylight saving time or other seasonal adjustments. In military and aviation time IST is designated E* ("Echo-Star").[1] It is indicated as Asia/Kolkata in the IANA time zone database.
    shortcuts["Asia/Kolkata"  ] = QByteArray::fromStdString("Asia/Calcutta"); // 

    shortcuts["SLST" ] = QByteArray::fromStdString("Asia/Colombo"); // Шри-Ланка - (UTC+05:30) Шри-Джаявардене-пура-Котте. Sri Lanka Standard Time (SLST) is the time zone for Sri Lanka. It is 5 hours and 30 minutes ahead of GMT/UTC (UTC+05:30).[1]
    shortcuts["CST"  ] = QByteArray::fromStdString("Asia/Shanghai"); // Китайское время - (UTC+08:00) Гонконг, Пекин, Урумчи, Чунцин. The time in China follows a single standard time offset of UTC+08:00 (eight hours ahead of Coordinated Universal Time), despite China spanning five geographical time zones. The official national standard time is called Beijing Time (Chinese: 北京时间) domestically[1] and China Standard Time (CST) internationally.[2] Daylight saving time has not been observed since 1991
    shortcuts["HKT"  ] = QByteArray::fromStdString("Asia/Hong_Kong"); // Китайское время - (UTC+08:00) Гонконг, Пекин, Урумчи, Чунцин. Hong Kong Time (abbreviation: HKT; Chinese: 香港時間; Jyutping: hoeng1 gong2 si4 gaan3) is the time in Hong Kong, observed at UTC+08:00 all year round.[1] The Hong Kong Observatory is the official timekeeper of the Hong Kong Time.
    shortcuts["KST"  ] = QByteArray::fromStdString("Asia/Seoul"); // Корейское время - (UTC+09:00) Сеул. South Korea has one time zone, Korea Standard Time (UTC+09:00), which is abbreviated KST
    shortcuts["PYT"  ] = QByteArray::fromStdString("Asia/Pyongyang"); // Северокорейское время - (UTC+09:00) Пхеньян. Time in North Korea, called Pyongyang Time[1] (PYT) or Standard Time of the Democratic People's Republic of Korea 
    shortcuts["PHT"  ] = QByteArray::fromStdString("Asia/Manila"); // Малайское время - (UTC+08:00) Куала-Лумпур, Сингапур. Philippine Standard Time (PST[1] or PhST;[2] Filipino: Pamantayang Oras ng Pilipinas), also known as Philippine Time (PHT), is the official name for the time zone used in the Philippines. The country only uses one time zone, at an offset of UTC+08:00, but has used daylight saving time for brief periods in the 20th century    
    shortcuts["SST"  ] = QByteArray::fromStdString("Asia/Singapore"); // Малайское время - (UTC+08:00) Куала-Лумпур, Сингапур. Singapore Standard Time (SST), also known as Singapore Time (SGT), is used in Singapore and is 8 hours ahead of UTC (UTC+08:00).
    shortcuts["SGT"  ] = QByteArray::fromStdString("Asia/Singapore"); // 
    shortcuts["JST"  ] = QByteArray::fromStdString("Asia/Tokyo"); // Japan Standard Time. Токийское время - (UTC+09:00) Осака, Саппоро, Токио


    // shortcuts[""  ] = QByteArray::fromStdString(""); // 

    return shortcuts;

}

//----------------------------------------------------------------------------
inline
QByteArray getTimezoneIanaIdFromAlias( const QString &idOrAlias )
{
    const std::map<QString, QByteArray>& aliasMap = getIanaTimezoneAliases();

    auto it = aliasMap.find(idOrAlias);

    if (it!=aliasMap.end())
        return it->second;

    return QByteArray::fromStdString( idOrAlias.toStdString() );
}

//----------------------------------------------------------------------------
inline
QByteArray getTimezoneIanaIdFromAlias( const std::string &idOrAlias )
{
    return getTimezoneIanaIdFromAlias( QString::fromStdString(idOrAlias) );
}

//----------------------------------------------------------------------------
inline
QByteArray getTimezoneIanaIdFromAlias( const char* idOrAlias )
{
    return getTimezoneIanaIdFromAlias( QString::fromLocal8Bit(idOrAlias) );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename StringType >
std::vector<StringType> getTimezonesAliasList()
{
    throw std::runtime_error("getTimezonesAliasList not implemented for this type");
}

//------------------------------
template< >
std::vector<QString> getTimezonesAliasList<QString>()
{
    std::vector<QString> resVec;

    const std::map<QString, QByteArray>& tzaMap = getIanaTimezoneAliases();

    std::map<QString, QByteArray>::const_iterator it = tzaMap.begin();

    for( ; it != tzaMap.end(); ++it)
       resVec.push_back(it->first);

    return resVec;
}

//------------------------------
template< >
std::vector<std::string> getTimezonesAliasList<std::string>()
{
    std::vector<std::string> resVec;

    const std::map<QString, QByteArray>& tzaMap = getIanaTimezoneAliases();

    std::map<QString, QByteArray>::const_iterator it = tzaMap.begin();

    for( ; it != tzaMap.end(); ++it)
       resVec.push_back(it->first.toStdString());

    return resVec;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
QTimeZone makeTimezoneByName( const QString &tzName )
{
    return QTimeZone( getTimezoneIanaIdFromAlias(tzName) );
}

//----------------------------------------------------------------------------
inline
QTimeZone makeTimezoneByName( const std::string &tzName )
{
    return QTimeZone( getTimezoneIanaIdFromAlias(tzName) );
}

//----------------------------------------------------------------------------
inline
QTimeZone makeTimezoneByName( const char* tzName )
{
    return QTimeZone( getTimezoneIanaIdFromAlias(tzName) );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
} // namespace qt_helpers




