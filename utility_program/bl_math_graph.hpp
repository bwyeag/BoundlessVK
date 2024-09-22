#ifndef _BOUNDLESS_MATH_GRAPH_FILE_HPP_
#define _BOUNDLESS_MATH_GRAPH_FILE_HPP_
#include <cstdint>
#include <vector>
namespace BL::Math {
template <typename T, typename U>
class DirectedGraphLink {
   public:
    struct Vertex {
        uint32_t inEdge, outEdge, inDegree, outDegree;
        T data;
        bool isNull;
    };
    struct Edge {
        uint32_t beginVertex, endVertex, inNext, outNext;
        U data;
    };

   private:
    const uint32_t NIL = (~0);
    std::vector<Vertex> V;
    std::vector<Edge> E;
    uint32_t freeVertex, freeEdge;

   public:
    DirectedGraphLink();
    ~DirectedGraphLink();
    T& getVData(uint32_t p) { return V[p].data; }
    const Vertex& getVertex(uint32_t p) { return V[p]; }
    uint32_t insertVertex() {
        Vertex* newv;
        uint32_t p;
        if (freeEdge == NIL) {
            V.push_back();
            p = V.size() - 1;
            newv = &V[p];
        } else {
            p = freeVertex;
            newv = &V[p];
            freeVertex = newv->outEdge;
        }
        newv->inDegree = newv->outDegree = 0;
        newv->inEdge = newv->outEdge = NIL;
        newv->isNull = false;
        return p;
    }
    void eraseEdge(uint32_t edge) {
        Edge* dele = &E[edge];
        uint32_t p = V[dele->beginVertex].outEdge, q = NIL;
        while (p != edge) {
            q = p;
            p = E[p].outNext;
        }
        if (p == edge) {
            if (q == NIL) {
                V[dele->beginVertex].outEdge = dele->outNext;
            } else {
                E[q].outNext = dele->outNext;
            }
        }
        p = V[dele->endVertex].inEdge, q = NIL;
        while (p != edge) {
            q = p;
            p = E[p].outNext;
        }
        if (p == edge) {
            if (q == NIL) {
                V[dele->endVertex].inEdge = dele->inNext;
            } else {
                E[q].inNext = dele->inNext;
            }
        }
        dele->beginVertex = dele->endVertex = NIL;
        dele->outNext = freeEdge;
        freeEdge = edge;
        dele->data.~U();
    }
    void eraseVertex(uint32_t p) {
        Vertex* delv = &V[p];
        uint32_t p = delv->inEdge;
        delv->isNull = true;
        delv->outEdge = freeVertex;
        freeVertex = p;
    }
};

}  // namespace BL::Math
#endif  //!_BOUNDLESS_MATH_GRAPH_FILE_HPP_