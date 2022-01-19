#include "ActiveVar.h"

#include <algorithm>

void ActiveVar::execute() {
  for (auto &func : this->module->get_functions()) {
    if (func->get_basic_blocks().empty()) {
      continue;
    } else {
      func_ = func;
      def.clear();
      use.clear();
      live_in.clear();
      live_out.clear();

      get_in_out();
      for (auto bb : func_->get_basic_blocks()) {
        //bb->set_def_var(def[bb]);
        //bb->set_use_var(use[bb]);

        bb->set_live_in(live_in[bb]);
        bb->set_live_out(live_out[bb]);
      }
      /*you need to finish this function*/
    }
  }
  return;
}

void ActiveVar::get_def() {
  for (auto bb : func_->get_basic_blocks()) {
    def[bb] = {};
    for (auto instr : bb->get_instructions()) {
      if (!instr->is_void()) {
        def[bb].insert(instr);
      }
    }
  }
}

void ActiveVar::get_use() {
  for (auto bb : func_->get_basic_blocks()) {
    use[bb] = {};
    phi[bb] = {};
    for (auto instr : bb->get_instructions()) {
      if (instr->is_phi()) {
        for (auto i = 0; i < instr->get_num_operand(); i += 2) {
          phi[bb].insert(instr->get_operand(i));
        }
      }
      for (auto op : instr->get_operands()) {
        if (dynamic_cast<ConstantInt *>(op))
          continue;
        if (dynamic_cast<BasicBlock *>(op))
          continue;
        if (dynamic_cast<Function *>(op))
          continue;
        use[bb].insert(op);
      }
    }
    for (auto var : def[bb]) {
      if (use[bb].find(var) != use[bb].end()) {
        use[bb].erase(var);
      }
    }
  }
}

void ActiveVar::get_in_out() {
  get_def();
  get_use();
  for (auto bb : func_->get_basic_blocks()) {
    live_in[bb] = {};
    live_out[bb] = {};
  }
  bool flag = 1;
  while (flag) {
    for (auto bb : func_->get_basic_blocks()) {
      std::set<Value *> tmp_out = {};
      for (auto suc : bb->get_succ_basic_blocks()) {
        auto suc_phi = phi[suc];
        std::set<Value *> phi_delete_part = {};
        for (auto inst : suc->get_instructions()) {
          if (inst->is_phi()) {
            for (auto i = 0; i < inst->get_num_operand(); i += 2) {
              // values not coming from bb
              if (inst->get_operand(i + 1) != bb) {
                phi_delete_part.insert(inst->get_operand(i));
              }
            }
          }
          std::set<Value *> not_to_delete = {};
          // values used in context other than phi should stay active
          auto suc_out = live_out[suc];
          auto suc_def = def[suc];
          std::set<Value *> inherit = {};
          std::set_difference(suc_out.begin(), suc_out.end(), suc_def.begin(),
                              suc_def.end(),
                              std::inserter(inherit, inherit.begin()));
          for (auto phi_op : phi_delete_part) {
            if (inherit.find(phi_op) != inherit.end()) {
              not_to_delete.insert(phi_op);
              continue;
            }
            bool inherited = 0;
            for (auto com : phi_op->get_use_list()) {
              auto use_inst = dynamic_cast<Instruction *>(com.val_);
              auto use_no = com.arg_no_;
              if (use_inst->get_parent() != suc) {
                continue; // find all the usage of phi_op IN SUC!
              }
              if (use_inst->is_phi()) {
                auto from = use_inst->get_operand(use_no + 1);
                if (from == bb) {
                  inherited = 1;
                  break; // this op is inherited from bb;
                }
              } else {
                inherited = 1; // this op is inherited from bb;
                break;
              }
            }
            if (inherited) {
              not_to_delete.insert(phi_op);
            }
          }
          for (auto phi_op : not_to_delete) {
            phi_delete_part.erase(phi_op);
          }
          auto suc_in = live_in[suc];
          std::set<Value *> suc_in_without_phi;
          std::set_difference(
              suc_in.begin(), suc_in.end(), phi_delete_part.begin(),
              phi_delete_part.end(),
              std::inserter(suc_in_without_phi, suc_in_without_phi.begin()));
          std::set<Value *> tmp;
          std::set_union(tmp_out.begin(), tmp_out.end(),
                         suc_in_without_phi.begin(), suc_in_without_phi.end(),
                         std::inserter(tmp, tmp.begin()));
          tmp_out = tmp;
        }

        live_out[bb] = tmp_out;
        auto tmp_in = tmp_out;
        std::set<Value *> tmp1, tmp2, tmp3;
        std::set_difference(tmp_in.begin(), tmp_in.end(), def[bb].begin(),
                            def[bb].end(), std::inserter(tmp1, tmp1.begin()));
        // tmp1 = OUT-def
        std::set_union(tmp1.begin(), tmp1.end(), use[bb].begin(), use[bb].end(),
                       std::inserter(tmp2, tmp2.begin()));
        // tmp2 = use plus tmp1
        std::set_union(tmp2.begin(), tmp2.end(), phi[bb].begin(), phi[bb].end(),
                       std::inserter(tmp3, tmp3.begin()));
        // tmp3 = use plus tmp3
        if (tmp3.size() > live_in[bb].size()) {
          live_in[bb] = tmp3;
          flag = 1;
        } else {
          flag = 0;
        }
      }
    }
  }
}
