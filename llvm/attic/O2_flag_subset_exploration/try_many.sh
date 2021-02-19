TARGET=optlvl2_custom.sh

# lines 23 to 105 are the flags
#for i in {23..105}; do
for i in {23..104}; do
#for i in {40,53,62,84,100,104}; do
    #line=$(awk 'NR=='$i $TARGET)
    line=$(sed $i'q;d' $TARGET)
    echo $i $line
    bash comment_out_nth.sh $TARGET $i
    
    ./optlvl2_custom.sh use_module_linked.ll    

    ~/sources/llvm-flang-version/build/bin/opt -load ~/sources/llvm-flang-version/build/lib/HPCAnalysis.so -analyse_scale -S -o new_ir.ll < ~/sources/llvm_hpc_static_analysis/F/use_module_linked.optlvl2_custom.ll 2> output.ll 
    grep 4294967277 output.ll
    
    bash uncomment_nth.sh $TARGET $i
    echo ""
done
