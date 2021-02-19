#!/usr/bin/env bash

llvm=/home/jzarins/install/llvm-old-flang/
export PATH=$llvm/bin:$PATH

INPUT_FILE=$1

[[ -z ${INPUT_FILE} ]] && exit 1

function join_by() {
  local IFS="$1"
  shift
  echo "$*"
}

#

INPUT_FILE_STEM=${INPUT_FILE%*.*}
OUT_SUFFIX="optlvl2_custom.ll"

# comment/uncomment and/or reorder passes freely
PASSES=(
#-early-cse
#-early-cse-memssa
#-instcombine
#-instsimplify
#-ipsccp
#-sccp
-aa
-adce
-alignment-from-assumptions
-assumption-cache-tracker
-barrier
-basicaa
-basiccg
-bdce
-block-freq
-branch-prob
-called-value-propagation
-constmerge
-correlated-propagation
-deadargelim
-demanded-bits
-div-rem-pairs
-domtree
-dse
-ee-instrument
-elim-avail-extern
-float2int
-forceattrs
-functionattrs
-globaldce
-globalopt
-globals-aa
#-gvn
-indvars
-inferattrs
#-inline
-jump-threading
-lazy-block-freq
-lazy-branch-prob
-lazy-value-info
-lcssa
-lcssa-verification
-libcalls-shrinkwrap
-licm
-loop-accesses
-loop-deletion
-loop-distribute
-loop-idiom
-loop-load-elim
-loop-rotate
-loop-simplify
-loop-sink
-loop-unroll
-loop-unswitch
-loop-vectorize
-loops
-lower-expect
-mem2reg
-memcpyopt
-memdep
-memoryssa
-mldst-motion
-opt-remark-emitter
-pgo-memop-opt
-phi-values
-postdomtree
-profile-summary-info
-prune-eh
-reassociate
-rpo-functionattrs
-scalar-evolution
-scoped-noalias
-simplifycfg
-slp-vectorizer
-speculative-execution
-sroa
-strip-dead-prototypes
-tailcallelim
-targetlibinfo
-tbaa
-tti
-verify
)

# convert array of passes to single passable command-line argument string
PASSES_STR=$(join_by ' ' "${PASSES[@]}")

opt \
  -S \
  ${PASSES_STR} \
  ${INPUT_FILE} -o "${INPUT_FILE_STEM}.${OUT_SUFFIX}"
