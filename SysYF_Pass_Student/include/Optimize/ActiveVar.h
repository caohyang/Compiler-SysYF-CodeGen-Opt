#ifndef SYSYF_ACTIVEVAR_H
#define SYSYF_ACTIVEVAR_H

#include "Pass.h"
#include "Module.h"

class ActiveVar : public Pass
{
public:
    ActiveVar(Module *module) : Pass(module) {}
    void execute() final;
    void get_def();
    void get_use();
    void get_in_out();
    const std::string get_name() const override {return name;}
private:
    Function *func_;
    const std::string name = "ActiveVar";
    std::map<BasicBlock *, std::set<Value *>> use, def, live_in, live_out, phi;
};

#endif  // SYSYF_ACTIVEVAR_H
