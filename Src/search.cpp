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
    if (options.searchtype == CN_SP_ST_ARASTAR && sresult.time != 0) {
        hweight = std::max(1.0, hweight - options.heuristicstep);
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
        auto node = Node(map.getStartPosition());
        node.H = getHeuristics(node.getPosition(), map, options);
        open.insert(node);
        addToPos(node);
    }

    bool found = sresult.pathfound;
    sresult = SearchResult();
    sresult.numberofsteps = 0;

    while (!open.empty()) {
        auto node = *open.begin();
        open.erase(open.begin());

        if (close.find(node) != close.end()) {
            continue;
        }
        close.insert(node);
        if (options.searchtype != CN_SP_ST_ARASTAR && node.getPosition() == map.getGoalPosition()) {
            found = true;
            break;
        }
        if (options.searchtype == CN_SP_ST_ARASTAR && node.F() >= goalFValue) {
            found = pos.find(node.getPosition()) != pos.end();
            break;
        }

        for (const auto &move : getAdjacent(node.getPosition(), map, options)) {
            auto new_g = move.delta + node.g;
            auto it = pos.find(move.position);
            if (it != pos.end() && it->g <= new_g) {
                continue;
            }
            Node new_node(move.position);
            new_node.g = new_g;
            new_node.H = getHeuristics(new_node.getPosition(), map, options);
            new_node.parent = node.getPosition();
            if (close.find(move.position) != close.end()) {
                if (options.searchtype == CN_SP_ST_ARASTAR) {
                    incons.insert(new_node);
                }
                continue;
            }
            if (move.position == map.getGoalPosition()) {
                goalFValue = std::min(goalFValue, new_node.F());
            }
            addToPos(new_node);
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
        auto current_node = *lookupCloseNode(map.getGoalPosition());
        sresult.pathlength = goalFValue;
        sresult.lppath.push_back(current_node);
        while (current_node.getPosition() != map.getStartPosition()) {
            current_node = *lookupCloseNode(current_node.parent);
            sresult.lppath.push_back(current_node);
        }
        std::reverse(sresult.lppath.begin(), sresult.lppath.end());
        sresult.hppath.push_back(sresult.lppath.front());
        bool direction_set = false;
        std::pair<int, int> direction = {0, 0};
        for (auto it = next(sresult.lppath.begin()); it != sresult.lppath.end(); ++it) {
            auto prv = prev(it);
            std::pair<int, int> new_direction = {it->i - prv->i, it->j - prv->j};
            if (!direction_set) {
                direction = new_direction;
                direction_set = true;
            } else if (direction != new_direction) {
                sresult.hppath.push_back(*prv);
                direction_set = false;
            }
        }
        sresult.hppath.push_back(sresult.lppath.back());
    }
    epsilon = hweight;
    if (close.find(map.getGoalPosition()) != close.end()) {
        auto closeNode = *close.find(map.getGoalPosition());
        auto div = std::numeric_limits<double>::infinity();
        for (const auto &node : open) {
            div = std::min(div, node.g + node.H / hweight);
        }
        for (const auto &node : incons) {
            div = std::min(div, node.g + node.H / hweight);
        }
        epsilon = std::min(epsilon, closeNode.g / div);
        std::cout << "eps: " << epsilon << std::endl;
        epsilon = std::max(1.0, epsilon);
    }
    std::chrono::duration<double> d = std::chrono::high_resolution_clock::now() - t;
    sresult.time = d.count();
    if (options.searchtype == CN_SP_ST_ARASTAR) {
        return {epsilon != 1.0, sresult};
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

decltype(Search::pos.cbegin()) Search::lookupCloseNode(const std::pair<int, int> &position) {
    return pos.find(position);
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

void Search::addToPos(const Node &node) {
    auto it = pos.find(node);
    if (it != pos.end()) {
        if (node.g <= it->g) {
            pos.erase(it);
            pos.insert(node);
        }
    } else {
        pos.insert(node);
    }
}

/*void Search::makePrimaryPath(Node curNode)
{
    //need to implement
}*/

/*void Search::makeSecondaryPath()
{
    //need to implement
}*/
