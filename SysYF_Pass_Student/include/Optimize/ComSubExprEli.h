#ifndef SYSYF_COMSUBEXPRELI_H
#define SYSYF_COMSUBEXPRELI_H

#include "Pass.h"
#include <map>
#include <set>
#include "Instruction.h"

/*****************************CommonSubExprElimination**************************************/
/***************************This class is based on SSA form*********************************/
/***************************you need to finish this class***********************************/
struct cmp_expr{  //compare expressions: number of operands, name/value...
    bool operator()(Instruction* a,Instruction* b)const{
        auto a_oprs = a->get_operands();
        auto b_oprs = b->get_operands();
        auto a_size = a_oprs.size();
        auto b_size = b_oprs.size();
        if (a_size < b_size) return true;
        if (a_size > b_size) return false;

        auto a_iter = a_oprs.begin();
        auto b_iter = b_oprs.begin();
        while (a_iter != a_oprs.end()){
            auto a_operand = *a_iter;
            auto b_operand = *b_iter;
            if (a_operand != b_operand){
                auto const_a_opr = dynamic_cast<ConstantInt*>(a_operand);
                auto const_b_opr = dynamic_cast<ConstantInt*>(b_operand);
                if (const_a_opr && const_b_opr){
                    if (const_a_opr->get_value() != const_b_opr->get_value())
                        return (const_a_opr->get_value() < const_b_opr->get_value());
                }
                else{
                    return (a_operand->get_name() < b_operand->get_name());
                }
            }
            a_iter++; b_iter++;
        }
        if (a->get_instr_op_name() != b->get_instr_op_name())
            return (a->get_instr_op_name() < b->get_instr_op_name());
        else{
            auto a_br = dynamic_cast<CmpInst*>(a);
            auto b_br = dynamic_cast<CmpInst*>(b);
            if (a_br && b_br)
                return (a_br->get_cmp_op() < b_br->get_cmp_op());
        }
        return false;
    }
};


class ComSubExprEli : public Pass {
public:
    explicit ComSubExprEli(Module* module):Pass(module){}
    const std::string get_name() const override {return name;}
    void execute() override;
    static bool is_valid_expr(Instruction* inst);

    void compute_local_gen(Function* f);
    void compute_local_kill(Function* f);
    void compute_global_in_out(Function* f);
    void compute_global_common_expr(Function* f);
    void initial_map(Function* f);

    static void remove_relevant_instr(Value* val,std::set<Instruction*,cmp_expr>& bb_set);
    static void insert_relevant_instr(Value* val,std::set<Instruction*,cmp_expr>& bb_set);
private:
    const std::string name = "ComSubExprEli";
    std::set<Instruction*,cmp_expr> U;
    std::map<BasicBlock*,std::set<Instruction*,cmp_expr>> bb_in, bb_out, bb_gen, bb_kill;
};

#endif // SYSYF_COMSUBEXPRELI_H