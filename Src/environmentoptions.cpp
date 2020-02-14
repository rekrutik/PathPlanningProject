#include "environmentoptions.h"

EnvironmentOptions::EnvironmentOptions()
{
    searchtype = CN_SP_ST_DIJK;
    metrictype = CN_SP_MT_EUCL;
    allowsqueeze = false;
    allowdiagonal = true;
    cutcorners = false;
    heuristicweight = 1.0;
    breakingties = CN_SP_BT_GMAX;
}

EnvironmentOptions::EnvironmentOptions(bool AS, bool AD, bool CC, int MT, int ST, double HW, int BT)
{
    metrictype = MT;
    allowsqueeze = AS;
    allowdiagonal = AD;
    cutcorners = CC;
    searchtype = ST;
    heuristicweight = HW;
    breakingties = BT;
}

