#ifndef DB_ARITHMETICMODULE_H
#define DB_ARITHMETICMODULE_H

#include "llvm/ADT/APInt.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

using namespace llvm;

class Node
{
public:

    enum class Type {
        ADDITION,
        SUBTRACTION,
        MULTIPLICATION,
        DIVISION,
        CONSTANT,
        VARIABLE
    };

    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

private:

    Type type_;
    int64_t constant;
    std::string variable;

public:

    Node(Type type) : type_(type) {}
    Node(int64_t constant) : type_(Type::CONSTANT), constant(constant) {}
    Node(std::string variable) : type_(Type::VARIABLE), variable(variable) {}

    size_t paramsCount()
    {
        switch (type_) {
            case Type::CONSTANT:
                return 0;
            case Type::VARIABLE:
                return 1;
            default:
                return left->paramsCount() + right->paramsCount();
        }
    }

    Value* eval(LLVMContext& context, BasicBlock* block, Function::arg_iterator& iterator)
    {
        switch (type_) {
            case Type::ADDITION:
                return BinaryOperator::CreateAdd(left->eval(context, block, iterator),
                                                 right->eval(context, block, iterator),
                                                 "Addition", block);
            case Type::SUBTRACTION:
                return BinaryOperator::CreateSub(left->eval(context, block, iterator),
                                                 right->eval(context, block, iterator),
                                                 "Subtraction", block);
            case Type::MULTIPLICATION:
                return BinaryOperator::CreateMul(left->eval(context, block, iterator),
                                                 right->eval(context, block, iterator),
                                                 "Multiplication", block);
            case Type::DIVISION:
                return BinaryOperator::CreateSDiv(left->eval(context, block, iterator),
                                                  right->eval(context, block, iterator),
                                                  "Division", block);
            case Type::CONSTANT:
                return ConstantInt::get(::Type::getInt64Ty(context), constant, true);
            case Type::VARIABLE:
                Value* value = &(*iterator);
                iterator++;
                value->setName(variable);
                return value;
        }
    }
};

class ArithmeticModule {
private:

    LLVMContext context_;
    std::string err_;
    Module* module_;
    ExecutionEngine* ee_;

public:

    ArithmeticModule()
    {
        InitializeNativeTarget();
        std::unique_ptr<Module> Owner(new Module("arithmetic", context_));
        module_ = Owner.get();

        ee_ = EngineBuilder(std::move(Owner))
                .setEngineKind(EngineKind::Interpreter)
                .setErrorStr(&err_)
                .create();
    }

    // this function return int and takes root.paramsCount() of int parameters
    Function* addFunction(std::string name, Node& root);

    bool runFunction(Function* func, std::vector<int64_t> _params, int64_t& _ret);
};


#endif //DB_ARITHMETICMODULE_H