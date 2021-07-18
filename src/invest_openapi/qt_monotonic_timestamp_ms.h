#pragma once


#include <QElapsedTimer>

#include <cstdint>



//----------------------------------------------------------------------------
inline
bool isUmbaNeedToInitializeQtMonotonicTimestampCounter()
{
    static bool initialized = false;

    if (!initialized)
    {
        initialized = true;
        return true;
    }

    return false;

}

//----------------------------------------------------------------------------
inline
std::uint64_t qtGetMonotonicTimestamp() // ms resolution
{
    static QElapsedTimer timer; // Чот не понял, как задать монотоник, но вроде по умолчанию он годен

    if (isUmbaNeedToInitializeQtMonotonicTimestampCounter())
    {
        timer.start();
    }

    return (std::uint64_t)timer.elapsed();
}



