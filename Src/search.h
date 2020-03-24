#ifndef SEARCH_H
#define SEARCH_H
#include "ilogger.h"
#include "searchresult.h"
#include "environmentoptions.h"
#include <algorithm>
#include <list>
#include <vector>
#include <cmath>
#include <limits>
#include <chrono>
#include <set>
#include <map>

class Search
{
    public:
        Search();
        ~Search(void);
        SearchResult startSearch(ILogger *Logger, const Map &Map, const EnvironmentOptions &options);

    protected:
        struct Adjacent {
            std::pair<int, int> position;
            double delta = 0.0;
        };

        std::set<Node, bool(*)(const Node &, const Node &)> open;
        std::set<Node, Node::CoordCompare> close;

        decltype(close.cbegin()) lookupCloseNode(const std::pair<int, int> &position);
        decltype(open.cbegin()) lookupOpenNode(const std::pair<int, int> &position);
        double getHeuristics(const std::pair<int, int>&position,
                const Map &Map, const EnvironmentOptions &options);
        std::vector<Adjacent> getAdjacent(const std::pair<int, int> &position,
                                          const Map &Map, const EnvironmentOptions &options);
        std::vector<Node> uniqueOpen() const;

        SearchResult                    sresult; //This will store the search result
        std::list<Node>                 lppath, hppath;
};
#endif
