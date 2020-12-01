#pragma once

#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;

#include <map>
#include <vector>

namespace oft {

    struct scale_node {
        Value* value;
        bool could_overflow = false;
        std::vector<scale_node*> children; 
        std::vector<scale_node*> parents; 
        scale_node(Value* v) : value(v) {}
    };

    class scale_graph {
    public:
        typedef std::map<Value*, scale_node*> sgmap;
        sgmap graph;
        std::vector<scale_node*> scale_vars;
        void addvertex(Value*, bool);
        void addvertex(Value*, bool, bool);
        void addedge(Value* from, Value* to);
        scale_node* getvertex(Value*);
        void text_print();
        unsigned int get_size();
    };

}
