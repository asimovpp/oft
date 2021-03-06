#pragma once

#include <map>
#include <vector>

namespace llvm {
class Value;
class raw_ostream;
} // namespace llvm

using namespace llvm;

namespace oft {

struct scale_node {
    Value *value;
    bool could_overflow = false;
    std::vector<scale_node *> children;
    std::vector<scale_node *> parents;
    scale_node(Value *v) : value(v) {}
};

class scale_graph {
  public:
    typedef std::map<Value *, scale_node *> sgmap;
    sgmap graph;
    std::vector<scale_node *> scale_vars;
    void addvertex(Value *, bool);
    void addvertex(Value *, bool, bool);
    void addedge(Value *from, Value *to);
    scale_node *getvertex(Value *);
    void print(raw_ostream &OS) const;
    unsigned int get_size();
};

} // namespace oft
