#ifndef _BOUNDLESS_GRAPH_CXX_FILE_
#define _BOUNDLESS_GRAPH_CXX_FILE_
#include <cstdint>
#include <vector>
namespace BL::Graph {
template<typename T>
union NoDestructor {
    T value;
    ~NoDestructor() {}
};
template <typename T, typename U>
class DirectedGraphLinked {
   public:
    using Index = uint32_t;
    const Index NULL_INDEX = ~0u;
    struct Node {
        Index inEdge, outEdge;
        uint32_t inDegree, outDegree;
        bool null;
        NoDestructor<T> data;
    };
    struct Edge {
        Index beginNode, endNode;
        Index nextIn, nextOut;
        NoDestructor<U> data;
    };

   private:
    std::vector<Node> V;
    std::vector<Edge> E;
    size_t nodeSize = 0, edgeSize = 0;
    Index vfree = NULL_INDEX, efree = NULL_INDEX;

   public:
    Index insert_vertex(T&& data) {
        Node* node;
        Index pos;
        if (vfree == NULL_INDEX) {
            V.resize(V.size() + 1);
            node = &V.back();
            pos = V.size() - 1;
        } else {
            node = &V[vfree];
            pos = vfree;
            vfree = node->outEdge;
        }
        node->inDegree = node->outDegree = 0;
        node->inEdge = node->outEdge = NULL_INDEX;
        node->null = false;
        node->data.value = std::forward(data);
        return pos;
    }
    void erase_vertex(Index index) {
        V[index].data.~T();
        V[index].outEdge = vfree;
        V[index].null = true;
        vfree = index;
    }
    Index insert_edge(U&& data, Index begin, Index end) {
        Edge* edge;
        Index pos;
        if (efree == NULL_INDEX) {
            E.resize(E.size() + 1);
            edge = &E.back();
            pos = E.size() - 1;
        } else {
            edge = &E[efree];
            pos = efree;
            efree = edge->nextOut;
        }
        edge->beginNode = begin;
        edge->endNode = end;
        edge->nextIn = V[end].inEdge;
        V[end].inEdge = pos;
        V[end].inDegree++;
        edge->nextOut = V[begin].outEdge;
        V[begin].outEdge = pos;
        V[begin].outDegree++;
        edge->data = std::forward(data);
        return pos;
    }
    void erase_edge(Index index) {
        Edge& edge = E[index];
        edge.data.~U();
        Index p = V[edge.beginNode].outEdge, q = NULL_INDEX;
        while (p != index)
            q = p, p = E[p].nextOut;
        if (q == NULL_INDEX) {
            V[edge.beginNode].outEdge = edge.nextOut;
        } else {
            E[q].nextOut = edge.nextOut;
        }
        p = V[edge.endNode].inEdge, q = NULL_INDEX;
        while (p != index)
            q = p, p = E[p].nextIn;
        if (q == NULL_INDEX) {
            V[edge.beginNode].inEdge = edge.nextIn;
        } else {
            E[q].nextIn = edge.nextIn;
        }
        edge.beginNode = NULL_INDEX;
        edge.endNode = NULL_INDEX;
        edge.nextOut = efree;
        efree = index;
    }
    struct VertexIterator {
        using Iterator = VertexIterator;
        std::vector<Node>& ref_node;
        Index ptr;
        void move_right() {
            do {
                ptr++;
            } while (ptr < ref_node.size() && ref_node[ptr].null);
        }
        void move_left() {
            do {
                ptr--;
            } while (ptr != NULL_INDEX && ref_node[ptr].null);
        }
        void move_right(size_t n) {
            for (; n > 0; n--)
                move_right();
        }
        void move_left(size_t n) {
            for (; n > 0; n--)
                move_left();
        }
        void set_begin() {
            ptr = 0;
            if (ref_node[ptr].null)
                move_right();
        }
        void set_end() { ptr = ref_node.size(); }
        Index get_inhead() { return ref_node[ptr].inEdge; }
        Index get_outhead() { return ref_node[ptr].outEdge; }
        bool is_begin() { return ptr == 0; }
        bool is_end() { return ptr == ref_node.size(); }
        bool is_null() {
            return 0 <= ptr && ptr < ref_node.size() && !ref_node[ptr].null;
        }
        Iterator& operator++() {
            move_right();
            return *this;
        }
        Iterator& operator--() {
            move_left();
            return *this;
        }
        Iterator& operator+=(size_t n) {
            move_right(n);
            return *this;
        }
        Iterator& operator-=(size_t n) {
            move_left(n);
            return *this;
        }
        bool operator==(const Iterator& it) {
            return it.ref_node.data() == ref_node.data() && ptr == it.ptr;
        }
        bool operator!=(const Iterator& it) {
            return it.ref_node.data() != ref_node.data() || ptr != it.ptr;
        }
    };
    VertexIterator vertex_iterator(Index index) { return {V, index}; }
    struct EdgeIterator {
        using Iterator = EdgeIterator;
        std::vector<Edge>& ref_edge;
        Index ptr;
        void move_next_inedge() { ptr = ref_edge[ptr].nextIn; }
        void move_next_outedge() { ptr = ref_edge[ptr].nextOut; }
        Index get_beginvertex() { return ref_edge[ptr].beginNode; }
        Index get_endvertex() { return ref_edge[ptr].endNode; }
        void move_right() {
            do {
                ptr++;
            } while (ptr < ref_edge.size() &&
                     ref_edge[ptr].begin == NULL_INDEX);
        }
        void move_left() {
            do {
                ptr--;
            } while (ptr != NULL_INDEX && ref_edge[ptr].begin == NULL_INDEX);
        }
        void move_right(size_t n) {
            for (; n > 0; n--)
                move_right();
        }
        void move_left(size_t n) {
            for (; n > 0; n--)
                move_left();
        }
        void set_begin() {
            ptr = 0;
            if (ref_edge[ptr].null)
                move_right();
        }
        void set_end() { ptr = ref_edge.size(); }
        bool is_begin() { return ptr == 0; }
        bool is_end() { return ptr == ref_edge.size(); }
        bool is_null() {
            return 0 <= ptr && ptr < ref_edge.size() &&
                   ref_edge[ptr].begin != NULL_INDEX;
        }
        Iterator& operator++() {
            move_right();
            return *this;
        }
        Iterator& operator--() {
            move_left();
            return *this;
        }
        Iterator& operator+=(size_t n) {
            move_right(n);
            return *this;
        }
        Iterator& operator-=(size_t n) {
            move_left(n);
            return *this;
        }
        bool operator==(const Iterator& it) {
            return it.ref_edge.data() == ref_edge.data() && ptr == it.ptr;
        }
        bool operator!=(const Iterator& it) {
            return it.ref_edge.data() != ref_edge.data() || ptr != it.ptr;
        }
    };
    EdgeIterator edge_iterator(Index index) { return {E, index}; }
};
}  // namespace BL::Graph
#endif  //!_BOUNDLESS_GRAPH_CXX_FILE_