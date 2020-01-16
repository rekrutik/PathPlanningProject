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
        //CODE HERE

        //Hint 1. You definetely need class variables for OPEN and CLOSE

        //Hint 2. It's a good idea to define a heuristic calculation function, that will simply return 0
        //for non-heuristic search methods like Dijkstra

        //Hint 3. It's a good idea to define function that given a node (and other stuff needed)
        //will return it's sucessors, e.g. unordered list of nodes

        //Hint 4. working with OPEN and CLOSE is the core
        //so think of the data structures that needed to be used, about the wrap-up classes (if needed)
        //Start with very simple (and ineffective) structures like list or vector and make it work first
        //and only then begin enhancement!

        struct Adjacent {
            std::pair<int, int> position;
            double delta = 0.0;
        };

        std::set<Node> open;
        std::set<Node, Node::CoordCompare> close;

        decltype(close.cbegin()) lookupCloseNode(const std::pair<int, int> &position);
        decltype(open.cbegin()) lookupOpenNode(const std::pair<int, int> &position);
        double getHeuristics(const std::pair<int, int>&position,
                const Map &Map, const EnvironmentOptions &options);
        std::vector<Adjacent> getAdjacent(const std::pair<int, int> &position,
                                          const Map &Map, const EnvironmentOptions &options);

        SearchResult                    sresult; //This will store the search result
        std::list<Node>                 lppath, hppath;
};
#endif
