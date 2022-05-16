#include "OverflowTool/ScaleGraph.hpp"

#include "OverflowTool/Debug.hpp"

#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "oft-scale-graph-construct"

namespace oft {

void scale_graph::addvertex(Value *val, bool is_root) {
    addvertex(val, is_root, false);
}

void scale_graph::addvertex(Value *val, bool is_root, bool could_overflow) {
    OFT_DEBUG(dbgs() << "Adding vertex: " << *val << "\n";);
    sgmap::iterator itr = graph.find(val);
    if (itr == graph.end()) {
        scale_node *v = new scale_node(val);
        v->could_overflow = could_overflow;
        if (is_root)
            scale_vars.push_back(v);
        graph[val] = v;
        return;
    }
    OFT_DEBUG(dbgs() << "\nVertex already exists!\n";);
}

void scale_graph::addedge(Value *from, Value *to) {
    OFT_DEBUG(dbgs() << "Adding edge: " << *from << " -> " << *to << "\n";);
    sgmap::iterator itr_f = graph.find(from);
    sgmap::iterator itr_t = graph.find(to);
    if (itr_f != graph.end() && itr_t != graph.end()) {
        scale_node *f = (*itr_f).second;
        scale_node *t = (*itr_t).second;
        if (std::find(f->children.begin(), f->children.end(), t) ==
            f->children.end()) {
            f->children.push_back(t);
            t->parents.push_back(f);
            return;
        } else {
            OFT_DEBUG(dbgs() << "\nEdge already exists!\n";);
        }
    } else {
        OFT_DEBUG(dbgs() << "\nOne or both of the vertices are missing!\n";);
    }
}

scale_node *scale_graph::getvertex(Value *val) {
    sgmap::iterator itr = graph.find(val);
    if (itr != graph.end()) {
        return itr->second;
    }
    return NULL;
}

void scale_graph::print(llvm::raw_ostream &OS) const {
    OS << "Scale variables:\n";
    for (scale_node *v : scale_vars)
        OS << *(v->value) << "\n";
    OS << "\nScale graph nodes:\n";
    for (auto &it : graph) {
        OS << "val: " << *(it.second->value) << "\n";
        OS << "owf: " << it.second->could_overflow << "\n";
        OS << "chi: ";
        for (scale_node *c : it.second->children)
            OS << *(c->value) << "=+=";
        OS << "\npar: ";
        for (scale_node *c : it.second->parents)
            OS << *(c->value) << "=+=";
        OS << "\n";
    }
}

unsigned int scale_graph::get_size() { return graph.size(); }

} // namespace oft
