#ifndef NODE_H
#define NODE_H

//That's the data structure for storing a single search node.
//You MUST store all the intermediate computations occuring during the search
//incapsulated to Nodes (so NO separate arrays of g-values etc.)

struct Node
{
    int     i = -1, j = -1; //grid cell coordinates
    double  g = 0.0, H = 0.0; //f-, g- and h-values of the search node
    std::pair<int, int> parent = {-1, -1}; //backpointer to the predecessor node (e.g. the node which g-value was used to set the g-velue of the current node)

public:
    Node(int i, int j) : i(i), j(j) {}
    Node(const std::pair<int, int> &p) : i(p.first), j(p.second) {}
    std::pair<int, int> getPosition() const { return std::make_pair(i, j); }
    double F() const { return g + H; }
    bool operator==(const Node &other) const {
        return i == other.i && j == other.j && g == other.g && H == other.H && parent == other.parent;
    }
    bool operator!=(const Node &other) const {
        return !(*this == other);
    }
    bool operator<(const Node &other) const {
        if (F() < other.F()) {
            return true;
        }
        if (F() > other.F()) {
            return false;
        }
        return i < other.i || (i == other.i && j < other.j);
    }
    struct CoordCompare {
        bool operator()(const Node &a, const Node &b) const {
            return a.i < b.i || (a.i == b.i && a.j < b.j);
        }
    };
};
#endif
