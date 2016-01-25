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
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Casting.h"
#include <vector>
#include <string>
#include <map>

using namespace llvm;

#define DEBUG_TYPE "hello"

// STATISTIC(HelloCounter, "Counts number of functions greeted");

// STATISTIC(Flow_dependence, "Counts number of flow dependece");

namespace {
  
  struct Expression {
    Value *left, *right;
    Value *tar;
    unsigned int opcode;

    Expression(Value *_left, Value *_right) {
      left = _left, right = _right;
    }
       
    Expression(Value *_left, Value *_right, unsigned int _opcode) {
      left = _left, right = _right, opcode = _opcode;
    }
    
    void setLeft(Value *v) {
      this->left = v;
    }

    void setRight(Value *v) {
      this->right = v;
    }
    
    void setOpcode(unsigned opcode) {
      this->opcode = opcode;
    }

    Value* getLeft() {
      return this->left;
    }

    Value* getRight() {
      return this->right;
    }

    unsigned int getOpcode() {
      return this->opcode;
    }

    bool operator==(const Expression &expr) const {
      if (this->left != expr.left) return false;
      if (this->right != expr.right) return false;
      if (this->opcode != expr.opcode) return false;
      return true;
    }
  };

  struct DataFlow : public FunctionPass {
    static char ID;
    typedef std::vector<Expression> ExprVec;
    std::map <Instruction*, ExprVec* > IN, OUT, GEN, KILL; 
    std::map <Instruction*, bool > isVisited; 
     
    DataFlow() : FunctionPass(ID){}
     
    void pushSet(ExprVec* v, Expression expr) {
      bool hasElement = false;
      for (auto element : *v) {
        if (element == expr) {
          hasElement = true;
          break;
        }
      }
      if (!hasElement)  
        v->push_back(expr);   
    } 

    void pushSetGroup(ExprVec *tarSet, ExprVec *srcSet) {
      for (auto src : *srcSet) {
        pushSet(tarSet, src);
      }
    }

    void andSetGroup(ExprVec *tar, ExprVec *src) {
      for (unsigned int i = 0; i < tar->size(); ++i) {
        Expression tarElement = *(tar->begin() + i);
        bool hasCommonElement = false;
        for (auto srcElement : *src) {
          if (tarElement == srcElement) {
            hasCommonElement = true;
            break;
          }
        }
        
        if (!hasCommonElement) {
          tar->erase(tar->begin() + i);
          i--;
        }
      }
    }

    void getKillSet(ExprVec *killSet, ExprVec *inSet, Value *killedInst) {
      for (auto inExpr : *inSet) {
        if (inExpr.getLeft() == killedInst || inExpr.getRight() == killedInst) {
          pushSet(killSet, inExpr);
        }
      }
    }
    
    void complementSet(ExprVec *mainSet, ExprVec *compSet) {
      for (unsigned int i = 0; i < mainSet->size(); ++i) {
       // DEBUG(errs() << i << " : " << mainSet->size() << "\n");
        for (auto kill : *compSet) {
          Expression in = *(mainSet->begin() + i);
          if (in == kill) {
            mainSet->erase(mainSet->begin() + i);
            i--;
            break;
          }
        }
      }
    }
    
    bool isSetEqual(ExprVec *setA, ExprVec *setB) {
      if (setA->size() != setB->size())  
        return false; 
      else if (setA->size() == 0)
        return true;

      for (auto setAElement : *setA) {
        bool findElement = false;
        for (auto setBElement : *setB) 
          if (setAElement == setBElement) 
            findElement = true;
        
        if (!findElement) 
          return false;
      }
      return true;
    }
     
    StringRef getOperatorChar(unsigned int opcode) {
      StringRef op;
      switch(opcode) {
      case Instruction::Add :
        op = " + ";
        break;

      case Instruction::Sub :
        op = " - ";
        break;

      case Instruction::Mul :
        op = " * ";  
        break;

      case Instruction::SDiv :
        op = " / ";
        break;
      }
      
      return op;
    }
    
    void print_expression(Value *left, Value *right, StringRef op) {
      if (ConstantInt *constRight = dyn_cast<ConstantInt>(right)) 
        errs() << left->getName() << op << constRight->getSExtValue() << ", ";
      else
        errs() <<  left->getName() << op << right->getName() << ", ";
    }

    void print_set(ExprVec *v) {
      if (v->size() == 0)
        errs() << "[EMPTY]";

      for (auto element : *v) {
        unsigned int opcode = element.getOpcode();
        StringRef op = getOperatorChar(opcode);
        Value *left = element.left;
        Value *right = element.right;
        print_expression(left, right, op);  
      }
      errs() << "\n";
    }

    void printStatus(BasicBlock *block) {
      errs() << "[ " << block->getName() << " ]\n";
      for (auto it = block->begin(); it != block->end(); ++it) {
        Instruction *inst = it;
        if (StoreInst *strInst = dyn_cast<StoreInst>(inst)) {
          // errs() << ">>>> " << *inst << "\n"; 
          Value *left = strInst->getOperand(1);
          Value *right = strInst->getOperand(0);
          StringRef op;
          
          errs() << "\t>>>> " << left->getName() << " = ";
          if (BinaryOperator *BI = dyn_cast<BinaryOperator>(right)) { 
            op = getOperatorChar(BI->getOpcode());
            Value *OperandA = BI->getOperand(0);
            Value *OperandB = BI->getOperand(1);
            
            if (isa<ConstantInt>(OperandA))
              std::swap(OperandA, OperandB);
             
            if (LoadInst *LI = dyn_cast<LoadInst>(OperandA))
              OperandA = LI->getOperand(0);

            if (LoadInst *LI = dyn_cast<LoadInst>(OperandB))
              OperandB = LI->getOperand(0);
            //  errs() << *OperandA << " " << *OperandB << "\n";
            print_expression(OperandA, OperandB, op);
            errs() << "\n";
          }
          else {
            ConstantInt *CI = dyn_cast<ConstantInt>(right);
            errs() << CI->getSExtValue() << "\n";
          }
          
          errs() << "\t\te_IN : "; 
            print_set(IN[inst]);
          
          errs() << "\t\te_OUT : ";
            print_set(OUT[inst]);
          
          errs() << "\t\te_GEN : ";
            print_set(GEN[inst]);
          
          errs() << "\t\te_KILL : ";
            print_set(KILL[inst]);
        }
      }
    }

    void traverse(BasicBlock *child, Instruction *lastInst) {
      // Instruction *lastInst = parent;
      for (auto bt = child->begin(); bt != child->end(); ++bt) {
        Instruction *inst = bt;
        //DEBUG(errs() << "> Current address : " << inst << "\n");
        if (IN.find(inst) == IN.end()) {
          ExprVec *in_v = new ExprVec();
          ExprVec *out_v = new ExprVec();
          ExprVec *gen_v = new ExprVec();
          ExprVec *kill_v = new ExprVec();
          
          IN[inst] = in_v;
          OUT[inst] = out_v;
          GEN[inst] = gen_v;
          KILL[inst] = kill_v;
        }

        ExprVec *in_set = IN[inst];
        ExprVec *out_set = OUT[inst];
        ExprVec *gen_set = GEN[inst];
        ExprVec *kill_set = KILL[inst];
        
        if (StoreInst *strInst = dyn_cast<StoreInst>(inst)) { 
          Value *operand = strInst->getOperand(0);
          Value *target;
          if (BinaryOperator *expr = dyn_cast<BinaryOperator>(operand)) {
            Value *instA, *instB;
            
            unsigned opcode = expr->getOpcode();
            instA = expr->getOperand(0);
            instB = expr->getOperand(1);
            target = strInst->getOperand(1);
            
            if (LoadInst *load = dyn_cast<LoadInst>(instA)) 
              instA = load->getOperand(0);
               
            if (LoadInst *load = dyn_cast<LoadInst>(instB))
              instB = load->getOperand(0);  
            
            if (instA > instB) 
              std::swap(instA, instB);
            
            if (isa<ConstantInt>(instA))
              std::swap(instA, instB);
            
            /* Here only handle only one block testcase */
            if (target != instA && target != instB)
              pushSet(gen_set, Expression(instA, instB, opcode));
            //  DEBUG(errs() << "out_set_orig : " << out_set->size()<< "\n");
            
            DEBUG(errs() << "\t(" << *target << ")" <<  " = ("  << *instA 
                          << ") operand (" << *instB << " " << instB << ") ");
            DEBUG(errs() << "\n");
          } else {
            target = bt->getOperand(1); 
            DEBUG(errs() << "\t" << *target << "\n");   
          }
          
          /***** KILL SET ******/
          if (lastInst != NULL) {
            ExprVec *prevOutSet = OUT[lastInst];
            if (isVisited.find(inst) == isVisited.end() ) {
              pushSetGroup(in_set, prevOutSet);
            } else {
              DEBUG(errs() << "constand ANDSET!!\n");
              andSetGroup(in_set, prevOutSet);    
            }
          }
          // kill  
          getKillSet(kill_set, in_set, target);      
          ExprVec compSet;
          for (auto it = in_set->begin(); it != in_set->end(); ++it) {
            Expression exprIn = *it;
            compSet.push_back(exprIn);
          }
          complementSet(&compSet, kill_set);
          
          ExprVec *tempOutSet = new ExprVec();
             
          //out_set->clear();
          pushSetGroup(tempOutSet, gen_set);
          pushSetGroup(tempOutSet, &compSet);
          
          if (isVisited.find(inst) != isVisited.end() 
                && isSetEqual(out_set, tempOutSet)) return; 
          else {
            out_set->clear();
            pushSetGroup(out_set, tempOutSet);
            delete tempOutSet;
          }
          isVisited[inst] = 1;
          lastInst = inst;  // update last instruction 
        }
      }
      // find next node 
      for (succ_iterator it = succ_begin(child); it != succ_end(child); ++it) {
        DEBUG(errs() << it->getName() << "\n");
        BasicBlock *block = *it;
        traverse(block, lastInst);
      }
    }

    bool runOnFunction(Function &F) override {
      DEBUG(errs() << "DataFlow : ");
      errs().write_escaped(F.getName()) << "\n";
      

      Function::iterator block = F.begin(); // get first basic block
      DEBUG(errs() << block->getName() << "\n");
      BasicBlock *bb = block;
      
      traverse(bb, NULL);    
      for (auto blockIt = F.begin(); blockIt != F.end(); ++blockIt) { 
        BasicBlock *block = blockIt;
        printStatus(block);
      }
      
      return false;
    }
  };
}

char DataFlow::ID = 2;
static RegisterPass<DataFlow> X("dataflow", "DataFlow Analysis Pass");
