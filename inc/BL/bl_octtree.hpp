#ifndef _BOUNDLESS_OCTTREE_CXX_HPP_
#define _BOUNDLESS_OCTTREE_CXX_HPP_
#include <concepts>
#include <cstdint>
#include <functional>
#include <queue>
#include <stack>
#include <vector>
#include "bl_collision.hpp"
#include "bl_log.hpp"
namespace BL::Math {
enum struct OctPos {
    I = 0x1,
    II = 0x2,
    III = 0x4,
    IV = 0x8,
    V = 0x10,
    VI = 0x20,
    VII = 0x40,
    VIII = 0x80,

    XPositive = 0x99,
    XNegative = 0x66,

    YPositive = 0x33,
    YNegative = 0xCC,

    ZPositive = 0xF,
    ZNegative = 0xF0
};
constexpr uint32_t to_parent_list_index(uint32_t a) {
    return (a - 1) / 8 + 1;
}
constexpr uint32_t to_first_index_of_group(uint32_t a) {
    return (a - 1) & (~0b111) + 1;
}
template <typename T, std::floating_point Scalar>
class OctTree {
   public:
    using Box = AABB<Scalar>;
    using Vec3 = vec3<Scalar>;
    using IterateFunction = std::function<void(const Box&, T&)>;
    const uint32_t NULL_NEXT = (~0);
    const Scalar K = static_cast<Scalar>(1.5);
    struct DataBlock {
        DataBlock* next;
        Box objectBox;
        T data;
    };
    struct OctNode {
        Box sizeBox;
        Box extendBox;
        uint32_t next = NULL_NEXT;
        DataBlock* dataHead = nullptr;
        uint32_t count = 0;
        void insert(DataBlock* n) {
            n->next = dataHead;
            dataHead = n;
            count++;
        }
        // 找到数据节点n的前一个节点,为HEAD时返回空指针
        DataBlock* find_pre(DataBlock* n) const {
            DataBlock *p = dataHead, *q = nullptr;
            while (p != n && p != nullptr) {
                q = p;
                p = p->next;
            }
            return q;
        }
        void drop(DataBlock* n) {
            DataBlock* pre = find_pre(n);
            if (pre == nullptr) {
                dataHead = dataHead->next;
            } else {
                pre->next = n->next;
            }
            count--;
        }
        void iterateData(const IterateFunction& fn) {
            DataBlock* p = dataHead;
            while (p != nullptr) {
                fn(p->objectBox, p->data);
                p = p->next;
            }
        }
    };

   private:
    std::stack<uint32_t> freeList;
    std::vector<uint32_t> parentList;
    std::vector<OctNode> nodeList;
    uint32_t nodeMaxData = 12, nodeMaxLayer = 8;

    enum struct FitPosition {
        Self = -1,
        I = 0,
        II = 1,
        III = 2,
        IV = 3,
        V = 4,
        VI = 5,
        VII = 6,
        VIII = 7,
        None = -2
    };
    FitPosition calculateOctNodeFit(uint32_t node, const Box& b) {
        CollisionResult res = intersectTest(nodeList[node].sizeBox, b);
        if (res < CollisionResult::inner) {
            return FitPosition::None;
        } else {
            uint32_t base = nodeList[node].next;
            for (uint32_t i = 0; i < 8; i++) {
                if (intersectTest(nodeList[base + i].sizeBox, b) ==
                    CollisionResult::inner) {
                    return static_cast<FitPosition>(i);
                }
            }
            return FitPosition::Self;
        }
    }
    uint32_t insertOctNodeNext(uint32_t p) {
        uint32_t r;
        if (freeList.empty()) {
            r = nodeList.size();
            nodeList.resize(r + 8);
            parentList.push_back(p);
        } else {
            r = freeList.top();
            freeList.pop();
            parentList[to_parent_list_index(r)] = p;
        }
        nodeList[p].next = r;
        Vec3 max = nodeList[p].sizeBox.max();
        Vec3 min = nodeList[p].sizeBox.min();
        Vec3 p2 = Vec3(min.x(), max.y(), max.z());
        Vec3 p3 = Vec3(min.x(), min.y(), max.z());
        Vec3 p4 = Vec3(max.x(), min.y(), max.z());
        Vec3 p5 = Vec3(max.x(), max.y(), min.z());
        Vec3 p6 = Vec3(min.x(), max.y(), min.z());
        Vec3 p8 = Vec3(max.x(), min.y(), min.z());
        Vec3 center = (max + min) / static_cast<Scalar>(2.0);
        nodeList[r + 0].sizeBox = {max, center};
        nodeList[r + 0].extendBox = {max, max + K * (center - max)};
        Vec3 c = p2 + (center - p2) * K;
        nodeList[r + 1].sizeBox = {Vec3(center.x(), p2.y(), p2.z()),
                                   Vec3(p2.x(), center.y(), center.z())};
        nodeList[r + 1].extendBox = {Vec3(c.x(), p2.y(), p2.z()),
                                     Vec3(p2.x(), c.y(), c.z())};
        c = p3 + (center - p3) * K;
        nodeList[r + 2].sizeBox = {Vec3(center.x(), center.y(), p3.z()),
                                   Vec3(p3.x(), p3.y(), center.z())};
        nodeList[r + 2].extendBox = {Vec3(c.x(), c.y(), p3.z()),
                                     Vec3(p3.x(), p3.y(), c.z())};
        c = p4 + (center - p4) * K;
        nodeList[r + 3].sizeBox = {Vec3(p4.x(), center.y(), p4.z()),
                                   Vec3(center.x(), p4.y(), center.z())};
        nodeList[r + 3].extendBox = {Vec3(p4.x(), c.y(), p4.z()),
                                     Vec3(c.x(), p4.y(), c.z())};
        c = p5 + (center - p5) * K;
        nodeList[r + 4].sizeBox = {Vec3(max.x(), max.y(), center.z()),
                                   Vec3(center.x(), center.y(), min.z())};
        nodeList[r + 4].extendBox = {Vec3(p5.x(), p5.y(), c.z()),
                                     Vec3(c.x(), c.y(), p5.z())};
        c = p6 + (center - p6) * K;
        nodeList[r + 5].sizeBox = {Vec3(center.x(), p6.y(), center.z()),
                                   Vec3(p6.x(), center.y(), p6.z())};
        nodeList[r + 5].extendBox = {Vec3(c.x(), p6.y(), c.z()),
                                     Vec3(p6.x(), c.y(), p6.z())};
        nodeList[r + 6].sizeBox = {center, min};
        nodeList[r + 6].extendBox = {min + K * (center - min), min};
        c = p8 + (center - p8) * K;
        nodeList[r + 7].sizeBox = {Vec3(max.x(), center.y(), center.z()),
                                   Vec3(center.x(), min.y(), min.z())};
        nodeList[r + 7].extendBox = {Vec3(p8.x(), c.y(), c.z()),
                                     Vec3(c.x(), p8.y(), p8.z())};
        return r;
    }
    // 节点p必须保证没有下层节点, 将node插入到p或p的下层节点中
    FitPosition expendOctNodeNext(uint32_t p, DataBlock* node) {
        if (nodeList[p].count < nodeMaxData) {
            nodeList[p].insert(node);
            return FitPosition::Self;
        } else {
            uint32_t next = insertOctNodeNext(p);
            DataBlock *cur = nodeList[p].dataHead, *pre = nullptr;
            while (cur != nullptr) {
                FitPosition fpt = calculateOctNodeFit(p, cur->objectBox);
                if (fpt == FitPosition::Self)
                    pre = cur, cur = cur->next;
                else {
                    if (pre == nullptr)
                        nodeList[p].dataHead = cur->next;
                    else
                        pre->next = cur->next;
                    nodeList[nodeList[p].next + static_cast<uint32_t>(fpt)]
                        .insert(cur);
                    cur = pre->next;
                }
            }
            FitPosition fpt = calculateOctNodeFit(p, node->objectBox);
            if (fpt == FitPosition::Self) {
                nodeList[p].insert(node);
            } else {
                nodeList[nodeList[p].next + static_cast<uint32_t>(fpt)].insert(
                    node);
            }
            return fpt;
        }
    }
    uint32_t deleteNode(uint32_t del) {
        if (nodeList[del].count > 0 || nodeList[del].next != NULL_NEXT)
            return del;
        uint32_t p = parentList[to_parent_list_index(del)];
        for (uint32_t i = 0; i < 8; i++)
            if (nodeList[nodeList[p].next + i].next != NULL_NEXT)
                return del;
        nodeList[p].next = NULL_NEXT;
        freeList.push(to_first_index_of_group(del));
        if (p != 0)
            return deleteNode(p);
        return 0;
    }
    void drop_internal(DataBlock* block, uint32_t belongTo) {
        DataBlock* pre = nodeList[belongTo].find_pre(block);
        if (pre->next != block)
            return;
        if (pre == nullptr)
            nodeList[belongTo].dataHead = cur->next;
        else
            pre->next = cur->next;
        nodeList[belongTo].count--;
    }
    uint32_t insertBeginAt(DataBlock* block, uint32_t beginAt) {
        uint32_t c = 1, p = beginAt;
        while (true) {
            if (c >= nodeMaxLayer) {
                nodeList[p].insert(block);
                nodeList[p].count++;
                return p;
            } else if (nodeList[p].next != NULL_NEXT) {
                FitPosition fp = calculateOctNodeFit(p, block->objectBox);
                if (fp == FitPosition::Self) {
                    nodeList[p].insert(block);
                    nodeList[p].count++;
                    return p;
                } else {
                    p = nodeList[p].next + static_cast<uint32_t>(fp), c++;
                }
            } else {
                FitPosition fp = expendOctNodeNext(p, block);
                uint32_t inc_pos =
                    FitPosition::Self
                        ? p
                        : nodeList[p].next + static_cast<uint32_t>(fp);
                nodeList[inc_pos].count++;
                return inc_pos;
            }
        }
    }

   public:
    void create(const Box& maxSize) {
        nodeList.resize(1);
        nodeList[0] = {.sizeBox = maxSize,
                       .extendBox = maxSize,
                       .next = NULL_NEXT,
                       .dataHead = nullptr,
                       .count = 0};
    }
    void find(const Box& size, const IterateFunction& fn) {
        std::queue<uint32_t> Q;
        Q.push(0);
        while (!Q.empty()) {
            uint32_t h = Q.front();
            nodeList[h].iterateData([&size, &fn](const Box& b, T& t) {
                if (intersectTest(size, b) >= CollisionResult::intersect) {
                    fn(b, t);
                }
            });
            uint32_t base = nodeList[h].next;
            for (uint32_t i = 0; i < 8; i++) {
                uint32_t curInd = base + i;
                if (intersectTest(size, nodeList[curInd].extendBox) >=
                    CollisionResult::intersect) {
                    Q.push(curInd);
                }
            }
        }
    }
    DataBlock* insert(const Box& size, uint32_t* ret_octnode) {
        uint32_t c = 1, p = 0;
        if (intersectTest(nodeList[0].sizeBox, size) !=
            CollisionResult::inner) {
            *ret_octnode = NULL_NEXT;
            return nullptr;
        }
        DataBlock* node = new DataBlock();
        node->objectBox = size;
        if (nodeList[0].next == NULL_NEXT) {
            FitPosition fp = expendOctNodeNext(0, node);
            *ret_octnode = (fp == FitPosition::Self)
                               ? 0
                               : nodeList[0].next + static_cast<uint32_t>(fp);
            nodeList[*ret_octnode].count++;
            return node;
        } else {
            FitPosition fp = calculateOctNodeFit(0, size);
            if (fp == FitPosition::Self)
                nodeList[0].insert(node);
            else
                p = nodeList[0].next + static_cast<uint32_t>(fp), c++;
            while (true) {
                if (c >= nodeMaxLayer) {
                    nodeList[p].insert(node);
                    *ret_octnode = p;
                    nodeList[p].count++;
                    return node;
                } else if (nodeList[p].next != NULL_NEXT) {
                    fp = calculateOctNodeFit(p, size);
                    if (fp == FitPosition::Self) {
                        nodeList[p].insert(node);
                        *ret_octnode = p;
                        nodeList[p].count++;
                        return node;
                    } else {
                        p = nodeList[p].next + static_cast<uint32_t>(fp), c++;
                    }
                } else {
                    fp = expendOctNodeNext(p, node);
                    *ret_octnode =
                        fp == FitPosition::Self
                            ? p
                            : nodeList[p].next + static_cast<uint32_t>(fp);
                    nodeList[*ret_octnode].count++;
                    return node;
                }
            }
        }
    }
    uint32_t drop(DataBlock* block, uint32_t belongTo) {
        drop_internal(block, belongTo);
        delete block;
        if (nodeList[belongTo].count == 0 && belongTo != 0)
            return deleteNode(belongTo);
        return belongTo;
    }
    uint32_t drop_nodelete(DataBlock* block, uint32_t belongTo) {
        drop_internal(block, belongTo);
        if (nodeList[belongTo].count == 0 && belongTo != 0)
            return deleteNode(belongTo);
        return belongTo;
    }
    uint32_t move(DataBlock* block, uint32_t belongTo, Vec3 dir) {
        block->objectBox.min() += dir;
        block->objectBox.max() += dir;
        uint32_t startNode = drop_nodelete(block, belongTo);
        while (intersectTest(nodeList[startNode].extendBox, block->objectBox) !=
               CollisionResult::inner) {
            uint32_t qt = to_parent_list_index(startNode);
            if (qt == to_parent_list_index(0))
                throw std::out_of_range();
            startNode = parentList[qt];
        }
        return insertBeginAt(block, startNode);
    }
};
}  // namespace BL::Math
#endif  //!_BOUNDLESS_OCTTREE_CXX_HPP_