//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Use.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Analysis/LoopPass.h"
#include <vector>
#include <string>
#include <map>

using namespace llvm;

#define DEBUG_TYPE "hello"

// STATISTIC(HelloCounter, "Counts number of functions greeted");

// STATISTIC(Flow_dependence, "Counts number of flow dependece");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct StateStruct {
    Instruction *LHS, *RHS;
    StringRef LHS_name, RHS_name;

    int64_t LHSIndex, RHSIndex;
    
    StateStruct() {}
    StateStruct(Instruction *_LHS, Instruction *_RHS, 
                StringRef _LHS_name, StringRef _RHS_name, 
                        int64_t _LHSIndex, int64_t _RHSIndex) {
      LHS = _LHS;
      RHS = _RHS; 
      LHS_name = _LHS_name;
      RHS_name = _RHS_name;
      LHSIndex = _LHSIndex;
      RHSIndex = _RHSIndex;
    }
  };
  
  struct Hello : public LoopPass {
    
    Hello() : LoopPass(ID) {}
    bool isTargetInst(Instruction *inst); 
    BinaryOperator* getBinaryOp(Value *inst, int step);
    Instruction* getLHSArray(Instruction *inst); 
    Instruction* getRHSArray(Instruction *inst);
    Instruction* getElement(Instruction *inst);
    StringRef getElementName(Instruction *inst);
    int64_t getIndex(Value *op);
    void detectDependence(); 
    void printState(StateStruct *state); 
    bool isFlowDependence(StateStruct *stateA, StateStruct *stateB);
    bool isAntiDependence(StateStruct *stateA, StateStruct *stateB);
    bool isOutputDependence(StateStruct *stateA, StateStruct *stateB);
    void reportDependence();
    virtual  bool runOnLoop(Loop *, LPPassManager &LPM) ;
    
    static char ID; // Pass identification, replacement for typeid  
    std::vector <Instruction*> Inst;
    std::vector <StoreInst*> beginValue;
    std::vector <AllocaInst*> allocArray; 
    std::vector <StateStruct*> stateList;
    std::vector <std::pair<StateStruct*, StateStruct*>> flowDependence;
    std::vector <std::pair<StateStruct*, StateStruct*>> outputDependence;
    std::vector <std::pair<StateStruct*, StateStruct*>> antiDependence;
    std::map <AllocaInst*, int64_t> symbolTable; 
  };

  bool Hello::runOnLoop(Loop *loop, LPPassManager &LPM ){
/*    
    LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
    for (LoopInfo::iterator I = LI->begin(), E = LI->end(); I != E; ++I) {
      for(const Loop::block_iterator block = I->block_begin(), y = I->block_end(); block!= y; ++block); 
      //DEBUG(errs() << *I << "\n");
    }
*/ 
    auto BB = loop->block_begin() + 1;
    auto entryBlock = (*BB)->getParent()->begin();

    for (auto &I : (*entryBlock)) {
      if (StoreInst *stInst = dyn_cast<StoreInst>(&I)) {
        beginValue.push_back(stInst);
        Value *op = stInst->getOperand(1);
        AllocaInst *allocInst = dyn_cast<AllocaInst>(op);
        ConstantInt *constInt = 
          dyn_cast<ConstantInt>(stInst->getOperand(0));
        
        if (constInt != NULL) { 
          uint64_t constValue = constInt->getSExtValue();
          symbolTable[allocInst] = constValue;
        }
      }

      else if(AllocaInst *aInst = dyn_cast<AllocaInst>(&I)) { 
        allocArray.push_back(aInst);
        symbolTable[aInst] = 0;
      }
    }
/* 
    std::map<AllocaInst*, int64_t>::iterator lastElement = --symbolTable.end();
    for (auto par : symbolTable) {
      errs() << *(par.first) << " | " << par.second << "\n";
      
    }
*/   
    for (auto I = (*BB)->begin(), E = (*BB)->end(); I != E; ++I) {
      if (isa<StoreInst>(I)) {
        // errs() << *I << "\n"; 
        Inst.push_back(I);
      }  
    }
    
    for (auto I : beginValue) {
      // errs() << *I << "\n";
      getBinaryOp(I, 0);
    }
  
    for (auto &I : Inst) {
      getBinaryOp(dyn_cast<Value>(I), 0);
      DEBUG(errs() << "--------------------------------\n");

      Instruction *LHS = getLHSArray(I);
      Instruction *RHS = getRHSArray(I);
      
      StringRef LHS_name = getElementName(LHS);
      StringRef RHS_name = getElementName(RHS);
      
      Instruction *LHSArray = getElement(LHS); 
      Instruction *RHSArray = getElement(RHS);
      
      int64_t LHSIndex = getIndex(LHS->getOperand(2));
      int64_t RHSIndex = getIndex(RHS->getOperand(2));
      
      StateStruct *state = 
          new StateStruct(LHS, RHS, LHS_name, RHS_name, LHSIndex, RHSIndex);
      stateList.push_back(state); 
       
      DEBUG(errs() << *LHSArray << " || " << *RHSArray << "\n");
      DEBUG(errs() << "Index --> " << LHSIndex << ", " << RHSIndex << "\n");
      DEBUG(errs() << "===============================\n");
    }
/*    
    for (auto &state : stateList) {
      printState(state);
    }
*/
    detectDependence();
    reportDependence();
     
    // getBinaryOp((*(BB+1))->begin(), 0);  
    // getBinaryOp((*(BB-1))->begin(), 0);  

    return false;
  }
  
  BinaryOperator* Hello::getBinaryOp(Value *inst, int step) {
    int numOperands = 0; 
    if (Instruction *cur_inst = dyn_cast<Instruction>(inst))
      numOperands = cur_inst->getNumOperands();
    for (int j = 0; j < step; ++j)
      DEBUG(errs() << "\t");
    DEBUG(errs() << numOperands << " (" << *inst << ", " << inst << ")");
    DEBUG(errs() << "\n");
 /*   
    if (BinaryOperator *BI = dyn_cast<BinaryOperator>(inst)) {
      return BI;
    }
*/
    for (int i = 0; i < numOperands; ++i) {
      Value *next_inst = dyn_cast<Instruction>(inst)->getOperand(i);
      getBinaryOp(next_inst, step+1);
    }
    return NULL;
  }

  Instruction* Hello::getLHSArray(Instruction *inst) {
    return dyn_cast<Instruction>(inst->getOperand(1)); 
  }

  Instruction* Hello::getRHSArray(Instruction *inst) {
    Instruction *first = dyn_cast<Instruction>(inst->getOperand(0)); 
    return dyn_cast<Instruction>(first->getOperand(0));
  }
  
  StringRef Hello::getElementName(Instruction *inst) {
    return inst->getOperand(0)->getName();  
  }


  Instruction* Hello::getElement(Instruction *inst) {
    return dyn_cast<Instruction>(inst->getOperand(0));
  }
  
  int64_t Hello::getIndex(Value *param) {
    DEBUG(errs() << "GET_INDEX => " << *param << "\n");    
    
    if (CastInst *cast = dyn_cast<CastInst>(param)) {
      return getIndex(cast->getOperand(0));
    }
    else if (BinaryOperator *BI = dyn_cast<BinaryOperator>(param)) {
      int64_t constA = getIndex(BI->getOperand(0));
      int64_t constB = getIndex(BI->getOperand(1));
      int64_t ret = 0;
      unsigned opcode = BI->getOpcode();
      
      switch(opcode) {
      case Instruction::Add :
        ret = constA + constB;
        break;
      
      case Instruction::Sub :
        ret = constA - constB;
        break;

      case Instruction::Mul :
        ret = constA * constB;
        break; 
        
      case Instruction::SDiv :
        ret = constA / constB;
        break;
      }

      return ret;
    }  
    else if (ConstantInt *constInt = dyn_cast<ConstantInt>(param)){
      return constInt->getSExtValue();
    }  
    else if (LoadInst *LI = dyn_cast<LoadInst>(param)){
      AllocaInst *AI = dyn_cast<AllocaInst>(LI->getOperand(0));
      int64_t prevConst = symbolTable[AI];
      return prevConst;
    } 
    
    return 0;  
  }
  
  bool Hello::isFlowDependence(StateStruct *stateA, StateStruct *stateB) {
      if (stateA->LHS_name == stateB->RHS_name) {
        if (stateA->LHSIndex >= stateB->RHSIndex) {
          return true;    
        }
      }
      
      if(stateA->RHS_name == stateB->LHS_name) {
        if (stateA->RHSIndex < stateB->LHSIndex) {
          return true;     
        }
      }
      return false;
  }

  bool Hello::isAntiDependence(StateStruct *stateA, StateStruct *stateB) {
      if (stateA->LHS_name == stateB->RHS_name) {
        if (stateA->LHSIndex < stateB->RHSIndex) {
          return true;    
        }
      }
      
      if(stateA->RHS_name == stateB->LHS_name) {
        if (stateA->RHSIndex >= stateB->LHSIndex) {
          return true;     
        }
      }
      return false;
  }

  bool Hello::isOutputDependence(StateStruct *stateA, StateStruct *stateB) {
    if (stateA->LHS_name == stateB->LHS_name)
      return true;
    return false;
  }

  void Hello::detectDependence() {
    for (int i = 1; i < (int)stateList.size(); ++i) {
      for (int j = i - 1; j >= 0; j--) {
        StateStruct *stateB = stateList[i];
        StateStruct *stateA = stateList[j];
        
        // Flow dependence detect
        DEBUG(errs() << "(\n");
        // printState(stateA);
        // printState(stateB);
        DEBUG(errs() << ")\n");
       
        if (isFlowDependence(stateA, stateB)) { 
          flowDependence.push_back(
              std::pair<StateStruct*, StateStruct*>(stateA, stateB)
          );
        }
        
        if (isAntiDependence(stateA, stateB)) { 
          antiDependence.push_back(
              std::pair<StateStruct*, StateStruct*>(stateA, stateB)
          );
        }
        if (isOutputDependence(stateA, stateB)) { 
          outputDependence.push_back(
              std::pair<StateStruct*, StateStruct*>(stateA, stateB)
          );
        }
      }
    }
  }
 
  void Hello::reportDependence() {
    errs() << "Number of FlowDependence : " << flowDependence.size() << "\n";
    for (auto &dep : flowDependence) {
      StateStruct *stateA = dep.first;
      StateStruct *stateB = dep.second;
      printState(stateA);
      printState(stateB);
      errs() << "\n";
    }

    
    errs() << "Number of AntiDependence : " << antiDependence.size() << "\n";
    for (auto &dep : antiDependence) {
      StateStruct *stateA = dep.first;
      StateStruct *stateB = dep.second;
      printState(stateA);
      printState(stateB);
      errs() << "\n";
    }

    errs() << "Number of OutputDependence : " << outputDependence.size() << "\n";
    for (auto &dep : outputDependence) {
      StateStruct *stateA = dep.first;
      StateStruct *stateB = dep.second;
      printState(stateA);
      printState(stateB);
      errs() << "\n";
    }
  }
  
  void Hello::printState(StateStruct *state) {
      errs() << state->LHS_name << "[" 
              << state->LHSIndex << "]" 
              << " = " << state->RHS_name << "["
              << state->RHSIndex << "]\n"; 
  }

  bool Hello::isTargetInst(Instruction *inst) {
    return isa<LoadInst>(inst) || isa<StoreInst>(inst) || 
            isa<BinaryOperator>(inst) || isa<SExtInst>(inst);
  }
}

char Hello::ID = 0;
static RegisterPass<Hello> X("chihmin", "ChihMin Pass");
