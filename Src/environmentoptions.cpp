#include "environmentoptions.h"

EnvironmentOptions::EnvironmentOptions()
{
    searchtype = CN_SP_ST_DIJK;
    metrictype = CN_SP_MT_EUCL;
    allowsqueeze = false;
    allowdiagonal = true;
    cutcorners = false;
    heuristicweight = 1.0;
    heuristicstep = 0.5;
    breakingties = CN_SP_BT_GMAX;
}