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
    open.insert(Node(map.getStartPosition().first, map.getGoalPosition().second));

    sresult.numberofsteps = 0;

    while (!open.empty()) {
        sresult.numberofsteps++;
        auto node = *open.begin();
        open.erase(open.begin());

        if (close.find(node) != close.end()) {
            continue;
        }

        std::cout << node.i << " " << node.j << " " << " " << node.parent << std::endl;

        auto close_it = close.insert(node).first;

        if (node.getPosition() == map.getGoalPosition()) {
            std::cout << "qwe" << std::endl;
            found = true;
            break;
        }

        for (const auto &move : getAdjacent(node.getPosition(), map, options)) {
            std::cout << move.position.first << " " << move.position.second << " " << std::endl;
            if (lookupCloseNode(move.position) != close.end()) {
                continue;
            }
            auto new_g = move.delta + node.g;

            Node new_node(move.position.first, move.position.second);
            new_node.g = new_g;
            new_node.H = 0;
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
            if (i == 0 && j == 0) continue;
            if (!map.CellOnGrid(position.first + i, position.second + j) || !map.CellIsTraversable(position.first + i, position.second + j)) continue;
            Adjacent adj;
            adj.position = std::make_pair(position.first + i, position.second + j);
            adj.delta = abs(i) + abs(j) == 2 ? sqrt(2.0) : 1.0;
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
