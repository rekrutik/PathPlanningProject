#include "search.h"

Search::Search()
{
//set defaults here
}

Search::~Search() {}


std::pair<bool, SearchResult> Search::startSearch(ILogger *Logger, const Map &map, const EnvironmentOptions &options)
{
    switch (options.breakingties) {
        case CN_SP_BT_GMIN:
            open = std::set<Node, bool(*)(const Node &, const Node &)>([](const Node &a, const Node &b) {
                if (a.F() < b.F()) {
                    return true;
                }
                if (a.F() > b.F()) {
                    return false;
                }
                if (a.g < b.g) {
                    return true;
                }
                if (a.g > b.g) {
                    return false;
                }
                return a.i < b.i || (a.i == b.i && a.j < b.j);
            });
            break;
        case CN_SP_BT_GMAX:
            open = std::set<Node, bool(*)(const Node &, const Node &)>([](const Node &a, const Node &b) {
                if (a.F() < b.F()) {
                    return true;
                }
                if (a.F() > b.F()) {
                    return false;
                }
                if (a.g < b.g) {
                    return false;
                }
                if (a.g > b.g) {
                    return true;
                }
                return a.i < b.i || (a.i == b.i && a.j < b.j);
            });
            break;
    }

    auto t = std::chrono::high_resolution_clock::now();
    bool found = false;
    if (options.searchtype == CN_SP_ST_ARASTAR && !incons.empty()) {
        auto new_hweight = hweight - options.heuristicstep;
        if (close.find(map.getGoalPosition()) != close.end()) {
            for (const auto &node : open) {
                new_hweight = std::min(new_hweight, close.find(map.getGoalPosition())->g / (node.g + node.H / hweight));
            }
            for (const auto &node : incons) {
                new_hweight = std::min(new_hweight, close.find(map.getGoalPosition())->g / (node.g + node.H / hweight));
            }
        }
        hweight = std::max(1.0, new_hweight);
        std::vector<Node> tmp;
        std::copy(open.begin(), open.end(), std::back_inserter(tmp));
        std::copy(incons.begin(), incons.end(), std::back_inserter(tmp));
        for (auto &node : tmp) {
            node.H = getHeuristics(node.getPosition(), map, options);
        }
        open.clear();
        open.insert(tmp.begin(), tmp.end());
        close.clear();
    } else {
        hweight = options.heuristicweight;
        auto node = Node(map.getStartPosition().first, map.getStartPosition().second);
        node.H = getHeuristics(node.getPosition(), map, options);
        open.insert(node);
    }

    sresult = SearchResult();
    sresult.numberofsteps = 0;

    while (!open.empty()) {
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
            if (close.find(move.position) != close.end()) {
                if (options.searchtype == CN_SP_ST_ARASTAR) {
                    incons.insert(*close.find(move.position));
                }
                continue;
            }
            auto new_g = move.delta + node.g;

            Node new_node(move.position.first, move.position.second);
            new_node.g = new_g;
            new_node.H = getHeuristics(new_node.getPosition(), map, options);
            new_node.parent = node.getPosition();
            open.insert(new_node);
        }
        if (Logger->loglevel == CN_LP_LEVEL_FULL_WORD) {
            Logger->writeToLogOpenClose(uniqueOpen(), close, sresult.numberofsteps);
        }
        sresult.numberofsteps++;
    }

    sresult.pathfound = found;
    if (Logger->loglevel == CN_LP_LEVEL_MEDIUM_WORD) {
        Logger->writeToLogOpenClose(uniqueOpen(), close, 0);
    }
    else if (Logger->loglevel == CN_LP_LEVEL_FULL_WORD) {
        Logger->writeToLogOpenClose(uniqueOpen(), close, sresult.numberofsteps);
    }
    auto close_copy = close;
    for (const auto &node : open) {
        close_copy.insert(node);
    }
    sresult.nodescreated = close_copy.size();

    if (sresult.pathfound) {
        auto current_node = lookupCloseNode(map.getGoalPosition());
        std::cout << current_node->i << " " << current_node->j << std::endl;
        sresult.pathlength = current_node->g;
        lppath.push_back(*current_node);
        while (current_node->getPosition() != map.getStartPosition()) {
            current_node = lookupCloseNode(current_node->parent);
            lppath.push_back(*current_node);
            std::cout << current_node->i << " " << current_node->j << std::endl;
        }
        std::reverse(lppath.begin(), lppath.end());

        sresult.lppath = &lppath;
        hppath.push_back(lppath.front());
        bool direction_set = false;
        std::pair<int, int> direction = {0, 0};
        for (auto it = next(lppath.begin()); it != lppath.end(); ++it) {
            auto prv = prev(it);
            std::pair<int, int> new_direction = {it->i - prv->i, it->j - prv->j};
            if (!direction_set) {
                direction = new_direction;
                direction_set = true;
            } else if (direction != new_direction) {
                hppath.push_back(*prv);
                direction_set = false;
            }
        }
        hppath.push_back(lppath.back());
        //std::reverse(hppath.begin(), hppath.end());
        sresult.hppath = &hppath;
    }
    std::chrono::duration<double> d = std::chrono::high_resolution_clock::now() - t;
    sresult.time = d.count();
    if (options.searchtype == CN_SP_ST_ARASTAR) {
        return {hweight != 1.0, sresult};
    }
    return {false, sresult};
}

double Search::getHeuristics(
        const std::pair<int, int> &position,
        const Map &Map,
        const EnvironmentOptions &options) {
    if (options.searchtype == CN_SP_ST_DIJK) {
        return 0.0;
    } else if (options.searchtype & CN_SP_ST_ASTAR) {
        auto finish = Map.getGoalPosition();
        auto dx = abs(position.first - finish.first);
        auto dy = abs(position.second - finish.second);
        double ret = 0.0;
        switch (options.metrictype) {
            case CN_SP_MT_MANH:
                ret = dx + dy;
                break;
            case CN_SP_MT_CHEB:
                ret = std::max(dx, dy);
                break;
            case CN_SP_MT_EUCL:
                ret = sqrt(dx * 1LL * dx + dy * 1LL * dy);
                break;
            case CN_SP_MT_DIAG:
                ret = abs(dx - dy) + sqrt(2.0) * (std::max(dx, dy) - abs(dx - dy));
                break;
        }
        return ret * hweight;
    }

}

decltype(Search::close.cbegin()) Search::lookupCloseNode(const std::pair<int, int> &position) {
    auto it = close.find(position);
    if (it == close.end()) {
        it = incons.find(position);
    }
    return it;
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

std::vector<Node> Search::uniqueOpen() const {
    std::vector<Node> res;
    std::set<std::pair<int, int>> used;
    for (const auto &node : open) {
        if (used.find({node.i, node.j}) == used.end() && close.find({node.i, node.j}) == close.end()) {
            used.insert({node.i, node.j});
            res.push_back(node);
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
