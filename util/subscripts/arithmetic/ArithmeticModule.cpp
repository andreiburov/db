#include "ArithmeticModule.h"

Function* ArithmeticModule::addFunction(std::string name, Node& root)
{
    Type* return_type = Type::getInt64Ty(context_);
    std::vector<Type*> params_v(root.paramsCount(), Type::getInt64Ty(context_));
    ArrayRef<Type*> params(params_v);
    FunctionType* function_type =
            FunctionType::get(return_type, params, false);

    Function* func = cast<Function>(module_->getOrInsertFunction(name.c_str(), function_type));
    Function::arg_iterator iterator = func->arg_begin();

    BasicBlock* start_block = BasicBlock::Create(context_, "Start", func);
    Value* result = root.eval(context_, start_block, iterator);
    ReturnInst::Create(context_, result, start_block);

    return func;
}

bool ArithmeticModule::runFunction(Function* func, std::vector<int64_t> _params, int64_t& _ret)
{
    std::vector<GenericValue> params(_params.size());
    size_t i = _params.size(); // read parameters backwards (C - notation)
    for (auto& p : params) {
        p.IntVal = APInt(64, _params[--i], true);
    }

    GenericValue result;

    if (!ee_) {
        errs() << "Failed to construct ExecutionEngine: " << err_ << "\n";
        return false;
    }

    errs() << "verifying... ";
    if (verifyModule(*module_)) {
        errs() << ": Error constructing function!\n";
        return false;
    }

    errs() << "OK\n";
    errs() << "We just constructed this LLVM module:\n\n---------\n" << *module_;
    errs() << "---------\nstarting function with JIT...\n";

    _ret = ee_->runFunction(func, params).IntVal.getLimitedValue();
    return true;
}