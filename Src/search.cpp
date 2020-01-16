#include "search.h"

Search::Search()
{
//set defaults here
}

Search::~Search() {}


SearchResult Search::startSearch(ILogger *Logger, const Map &map, const EnvironmentOptions &options)
{
    auto t = std::chrono::high_resolution_clock::now();
    bool found = false;
    open.insert(Node(map.getStartPosition().first, map.getStartPosition().second));

    sresult.numberofsteps = 0;

    while (!open.empty()) {
        sresult.numberofsteps++;
        auto node = *open.begin();
        open.erase(open.begin());

        if (close.find(node) != close.end()) {
            continue;
        }

        auto close_it = close.insert(node).first;

        if (node.getPosition() == map.getGoalPosition()) {
            found = true;
            break;
        }

        for (const auto &move : getAdjacent(node.getPosition(), map, options)) {
            if (lookupCloseNode(move.position) != close.end()) {
                continue;
            }
            auto new_g = move.delta + node.g;

            Node new_node(move.position.first, move.position.second);
            new_node.g = new_g;
            new_node.H = getHeuristics(new_node.getPosition(), map, options);
            new_node.parent = &(*close_it);
            open.insert(new_node);
        }
    }

    sresult.pathfound = found;
    sresult.nodescreated = open.size() + close.size();
    if (sresult.pathfound) {
        auto current_node = &(*lookupCloseNode(map.getGoalPosition()));
        sresult.pathlength = current_node->g;
        const auto finish = *lookupCloseNode(map.getStartPosition());
        hppath.push_back(*current_node);
        while (*current_node != finish) {
            current_node = current_node->parent;
            hppath.push_back(*current_node);
        }

        sresult.hppath = sresult.lppath = &hppath;
    }
    std::chrono::duration<double> d = std::chrono::high_resolution_clock::now() - t;
    sresult.time = d.count();
    return sresult;
}

double Search::getHeuristics(const std::pair<int, int> &position, const Map &Map, const EnvironmentOptions &options) {
    auto finish = Map.getGoalPosition();
    auto dx = abs(position.first - finish.first);
    auto dy = abs(position.second - finish.second);
    switch (options.metrictype) {
        case CN_SP_MT_MANH:
            return dx + dy;
        case CN_SP_MT_CHEB:
            return std::max(dx, dy);
        case CN_SP_MT_EUCL:
            return sqrt(dx * 1LL * dx + dy * 1LL * dy);
        case CN_SP_MT_DIAG:
            return abs(dx - dy) + sqrt(2.0) * (std::max(dx, dy) - abs(dx - dy));
    }
}

decltype(Search::close.cbegin()) Search::lookupCloseNode(const std::pair<int, int> &position) {
    auto it = close.lower_bound(Node(position.first, position.second));
    if (it != close.end() && it->i == position.first && it->j == position.second) {
        return it;
    }
    return close.end();
}

decltype(Search::open.cbegin()) Search::lookupOpenNode(const std::pair<int, int> &position) {
    auto it = open.lower_bound(Node(position.first, position.second));
    if (it != open.end() && it->i == position.first && it->j == position.second) {
        return it;
    }
    return open.end();
}

std::vector<Search::Adjacent>
Search::getAdjacent(const std::pair<int, int> &position, const Map &map, const EnvironmentOptions &options) {
    std::vector<Adjacent> res;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            bool diagonal = abs(i) + abs(j) == 2;
            if (i == 0 && j == 0) continue;
            if (!map.CellOnGrid(position.first + i, position.second + j) || !map.CellIsTraversable(position.first + i, position.second + j)) continue;
            if (!options.allowdiagonal && diagonal) continue;
            if (!options.cutcorners && diagonal && (map.CellIsObstacle(position.first, position.second + j) || map.CellIsObstacle(position.first + i, position.second))) continue;
            if (!options.allowsqueeze && diagonal && map.CellIsObstacle(position.first, position.second + j) && map.CellIsObstacle(position.first + i, position.second)) continue;
            Adjacent adj;
            adj.position = std::make_pair(position.first + i, position.second + j);
            adj.delta = diagonal ? sqrt(2.0) : 1.0;
            res.push_back(adj);
        }
    }
    return res;
}

/*void Search::makePrimaryPath(Node curNode)
{
    //need to implement
}*/

/*void Search::makeSecondaryPath()
{
    //need to implement
}*/
