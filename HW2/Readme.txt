1. Put files & folders  to ${LLVM_HOME}/lib/Transforms

2. Compile LLVM project, and you will get dataflow analysis library "LLVMDataFlow.so" under ${BUILD_FOLDER}/lib/ 

3. There are several testcases under the testcase folder.

4. First, compile *.c file to bitcode(*.bc files)

5. Use LLVM_OPT to execute and analysis testcase program, e.g. ${OPT} -load ${PATH}/LLVMDataFlow.so -dataflow ${BITCODE}

6. You will get analysis result on your screen, there're sample outputs(*.out files) in testcase* folder.

7. Now only support two element expression, e.g. z = x + y, z = x + 2 --> R.H.S. Only has two variable.

8. Of course support if/else and simple for-loop.