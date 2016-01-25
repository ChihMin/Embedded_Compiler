1. 把此目錄的CMakeLists.txt複製到 ${LLVM_HOME_DIR}/lib/Transforms

2. 把ChihMin資料夾複製到 ${LLVM_HOME_DIR}/lib/Transforms

3. 把llvm 編起來

4. 在testcase資料夾底下有不同狀況的testcase，先把他編成bitcode

5. opt -load ${ABSOLUTE_PATH}/LLVMChihMin.so -chihmin ${bitcode}

6. Standard Output 有統計Dependence的數量以及Statement

