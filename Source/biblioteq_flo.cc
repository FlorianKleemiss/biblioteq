#include "biblioteq.h"

QRect biblioteq::get_Screensize()
{
    return m_screensize;
}

void biblioteq::set_Screensize(const QRect &size)
{
    m_screensize = size;
}