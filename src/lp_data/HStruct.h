/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2023 by Julian Hall, Ivet Galabova,    */
/*    Leona Gottwald and Michael Feldmeier                               */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file lp_data/HStruct.h
 * @brief Structs for HiGHS
 */
#ifndef LP_DATA_HSTRUCT_H_
#define LP_DATA_HSTRUCT_H_

#include <unordered_map>
#include <vector>

#include "lp_data/HConst.h"

struct HighsIterationCounts {
  HighsInt simplex = 0;
  HighsInt ipm = 0;
  HighsInt crossover = 0;
  HighsInt qp = 0;
};

struct HighsSolution {
  bool value_valid = false;
  bool dual_valid = false;
  std::vector<double> col_value;
  std::vector<double> col_dual;
  std::vector<double> row_value;
  std::vector<double> row_dual;
  void invalidate();
  void clear();
};

struct HighsObjectiveSolution {
  double objective;
  std::vector<double> col_value;
  void clear();
};

struct HighsCallbackDataOut {
  HighsLogType log_type;
  HighsInt simplex_iteration_count;
  HighsObjectiveSolution objective_solution;
  void clear();
};

struct HighsCallbackDataIn {
  bool user_interrupt;
  void clear();
};

struct HighsCallback {
  void (*highs_user_callback)(const int, const char*, void*,
			      const HighsCallbackDataOut&,
			      HighsCallbackDataIn&) = nullptr;
  void* highs_user_callback_data = nullptr;
  const int num_type = int(NewHighsCallbackType::kMipImprovingSolution) + 1;
  std::vector<bool> active;
  HighsCallbackDataOut highs_callback_data_out;
  HighsCallbackDataIn highs_callback_data_in;
  bool callbackAction(const NewHighsCallbackType type);
  void clear();
};

struct RefactorInfo {
  bool use = false;
  std::vector<HighsInt> pivot_row;
  std::vector<HighsInt> pivot_var;
  std::vector<int8_t> pivot_type;
  double build_synthetic_tick;
  void clear();
};

struct HotStart {
  bool valid = false;
  RefactorInfo refactor_info;
  std::vector<int8_t> nonbasicMove;
  void clear();
};

struct HighsBasis {
  bool valid = false;
  bool alien = true;
  bool was_alien = true;
  HighsInt debug_id = -1;
  HighsInt debug_update_count = -1;
  std::string debug_origin_name = "None";
  std::vector<HighsBasisStatus> col_status;
  std::vector<HighsBasisStatus> row_status;
  void invalidate();
  void clear();
};

struct HighsScale {
  HighsInt strategy;
  bool has_scaling;
  HighsInt num_col;
  HighsInt num_row;
  double cost;
  std::vector<double> col;
  std::vector<double> row;
};

struct HighsLpMods {
  std::vector<HighsInt> save_non_semi_variable_index;
  std::vector<HighsInt> save_inconsistent_semi_variable_index;
  std::vector<double> save_inconsistent_semi_variable_lower_bound_value;
  std::vector<double> save_inconsistent_semi_variable_upper_bound_value;
  std::vector<HighsVarType> save_inconsistent_semi_variable_type;

  std::vector<HighsInt> save_relaxed_semi_variable_lower_bound_index;
  std::vector<double> save_relaxed_semi_variable_lower_bound_value;
  std::vector<HighsInt> save_tightened_semi_variable_upper_bound_index;
  std::vector<double> save_tightened_semi_variable_upper_bound_value;
  void clear();
  bool isClear();
};

struct HighsNameHash {
  std::unordered_map<std::string, int> name2index;
  void form(const std::vector<std::string>& name);
  bool hasDuplicate(const std::vector<std::string>& name);
  void clear();
};

struct HighsPresolveRuleLog {
  HighsInt call;
  HighsInt col_removed;
  HighsInt row_removed;
};

struct HighsPresolveLog {
  std::vector<HighsPresolveRuleLog> rule;
  void clear();
};

#endif /* LP_DATA_HSTRUCT_H_ */
