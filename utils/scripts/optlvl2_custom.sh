#!/usr/bin/env bash

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
  -tti
  -verify
  -ee-instrument
  -targetlibinfo -assumption-cache-tracker
  -profile-summary-info
  -forceattrs
  -basiccg
  -always-inline
  -barrier
  -targetlibinfo
  -tti
  -tbaa
  -scoped-noalias
  -assumption-cache-tracker
  -profile-summary-info
  -forceattrs
  -inferattrs
  -ipsccp
  -called-value-propagation
  -globalopt
  -domtree
  -mem2reg
  -deadargelim
  -basicaa
  -cfl-steens-aa
  -aa
  -loops
  -lazy-branch-prob
  -lazy-block-freq
  -opt-remark-emitter
  -instcombine
  -simplifycfg
  -basiccg
  -globals-aa
  -prune-eh
  -always-inline
  -functionattrs
  -sroa
  -memoryssa
  -early-cse-memssa
  -speculative-execution
  -lazy-value-info
  -jump-threading
  -correlated-propagation
  -libcalls-shrinkwrap
  -branch-prob
  -block-freq
  -pgo-memop-opt
  -tailcallelim
  -reassociate
  -loop-simplify
  -lcssa-verification
  -lcssa
  -scalar-evolution
  -licm
  -loop-unswitch
  -indvars
  -loop-idiom
  -loop-deletion
  -memdep
  -memcpyopt
  -sccp
  -demanded-bits
  -bdce
  -dse
  -postdomtree
  -adce
  -barrier
  -rpo-functionattrs
  -globaldce
  -float2int
  -loop-accesses
  -loop-distribute
  -loop-load-elim
  -alignment-from-assumptions
  -strip-dead-prototypes
  -loop-sink
  -instsimplify
  -div-rem-pairs
  -verify
  -ee-instrument
  -early-cse
  -lower-expect
)

# convert array of passes to single passable command-line argument string
PASSES_STR=$(join_by ' ' "${PASSES[@]}")

opt \
  -S \
  ${PASSES_STR} \
  ${INPUT_FILE} -o "${INPUT_FILE_STEM}.${OUT_SUFFIX}"
