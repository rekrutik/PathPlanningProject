#ifndef ENVIRONMENTOPTIONS_H
#define ENVIRONMENTOPTIONS_H
#include "gl_const.h"

class EnvironmentOptions
{
public:
    EnvironmentOptions(bool AS, bool AD, bool CC, int MT = CN_SP_MT_EUCL, int ST = CN_SP_ST_DIJK, double HW = 1.0, int BT = CN_SP_BT_GMAX);
    EnvironmentOptions();
    int     searchtype;
    int     metrictype;     //Can be chosen Euclidean, Manhattan, Chebyshev and Diagonal distance
    bool    allowsqueeze;   //Option that allows to move throught "bottleneck"
    bool    allowdiagonal;  //Option that allows to make diagonal moves
    bool    cutcorners;     //Option that allows to make diagonal moves, when one adjacent cell is untraversable
    double  heuristicweight;
    int     breakingties;
};

#endif // ENVIRONMENTOPTIONS_H
