#include "GlobalLocalProperty.h"


using namespace GraphEx;
void GlocalPropertySetBase::increasePropertyCountUsingGlobal()
{
    if (!hasGlobalPropertySet())
    {
        return;
    }

    ++mPropertyCountUsingGlobal;
}


void GlocalPropertySetBase::decreasePropertyCountUsingGlobal()
{
    if (!hasGlobalPropertySet())
    {
        return;
    }

    --mPropertyCountUsingGlobal;
}


bool GlocalPropertySetBase::hasPropertiesUsingGlobal() const
{
    return hasGlobalPropertySet() && mPropertyCountUsingGlobal > 0;
}