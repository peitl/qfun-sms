/*
 * File:  ipasir_wrap.h
 * Author:  mikolas
 * Created on:  Wed Jul 10 16:42:31 DST 2019
 * Copyright (C) 2019, Mikolas Janota
 */
#pragma once
#include "minisat/core/SolverTypes.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "cadicalSMS.hpp"

#define SATSPC Minisat
namespace SATSPC {
//std::ostream &operator<<(std::ostream &outs, Minisat::Lit lit);
class SMSWrap {
  public:
    inline const Minisat::LSet &get_conflict() { return _conflict; }
    inline const Minisat::vec<Minisat::lbool> &get_model() { return _model; }

    SMSWrap(int vertices = 2, int cutoff = 20000) :_nvars(0), sms(SolverConfig(vertices, cutoff)) {
        sms.config.printStats = true;
    }

    //virtual ~IPASIRWrap() { ipasir_release(_s); }

    inline void setFrozen(Minisat::Var, bool) {}
    inline void setPolarity(Minisat::Var, Minisat::lbool) {}
    inline void bump(Minisat::Var) {}
    inline void releaseVar(Minisat::Lit l) { addClause(l); }
    inline bool simplify() { return true; }

    //bool addClause(Minisat::vec<Minisat::Lit> &cl) {
    //    std::vector<int> native_cl(cl.size());
    //    for (int i = 0; i < cl.size(); ++i)
    //        native_cl[i] = lit2val(cl[i]);
    //    sms.addClause(native_cl, false);
    //    return true;
    //}

    vector<int> lit2val(const Minisat::vec<Minisat::Lit> &cl) {
        vector<int> mapped_clause(cl.size());
        for (int i = 0; i < cl.size(); ++i)
            mapped_clause[i] = lit2val(cl[i]);
        return mapped_clause;
    }

    vector<int> lit2val(const vector<Minisat::Lit> &cl) {
        vector<int> mapped_clause(cl.size());
        for (size_t i = 0; i < cl.size(); ++i)
            mapped_clause[i] = lit2val(cl[i]);
        return mapped_clause;
    }

    inline bool addClause(const Minisat::vec<Minisat::Lit> &cl) {
        sms.addClause(lit2val(cl), false);
        return true;
    }

    inline bool addClause(const vector<Minisat::Lit> &cl) {
        sms.addClause(lit2val(cl), false);
        return true;
    }

    inline bool addClause_(const Minisat::vec<Minisat::Lit> &cl) {
        return addClause(cl);
    }

    bool addClause(Minisat::Lit p) {
        sms.addClause({
            lit2val(p),
            }, false);
        return true;
    }

    bool addClause(Minisat::Lit p, Minisat::Lit q) {
        sms.addClause({
            lit2val(p),
            lit2val(q),
            }, false);
        return true;
    }

    bool addClause(Minisat::Lit p, Minisat::Lit q, Minisat::Lit r) {
        sms.addClause({
            lit2val(p),
            lit2val(q),
            lit2val(r),
            }, false);
        return true;
    }

    bool addClause(Minisat::Lit p, Minisat::Lit q, Minisat::Lit r,
                   Minisat::Lit s) {
        sms.addClause({
            lit2val(p),
            lit2val(q),
            lit2val(r),
            lit2val(s),
            }, false);
        return true;
    }

    int nVars() const { return _nvars; }

    void new_variables(Minisat::Var v) {
        if (_nvars < v)
            _nvars = v + 1;
    }

    inline Minisat::Var newVar() { return ++_nvars; }

    inline Minisat::lbool get_model_value(Minisat::Var v) const {
        return v < _model.size() ? _model[v] : Minisat::l_Undef;
    }

    bool solve(const Minisat::vec<Minisat::Lit> &assumps);
    bool solve();

  public:
    /* const int           _verb = 1; */
    int _nvars;
    CadicalSolver sms;
    Minisat::vec<Minisat::Lit> _assumps;
    Minisat::LSet _conflict;
    Minisat::vec<Minisat::lbool> _model;
    inline int lit2val(const Minisat::Lit &p) {
        return Minisat::sign(p) ? -Minisat::var(p) : Minisat::var(p);
    }

    //inline void add(const Minisat::Lit &p) { /*std::cout << lit2val(p) << " ";*/ sms.solver->add(lit2val(p)); }

    /*inline bool end_clause() {
        sms.solver->add(0);
        //std::cout << "0 learned" << std::endl;
        return true;
    }*/
};

inline bool SMSWrap::solve(const Minisat::vec<Minisat::Lit> &assumps) {

  const auto rv = sms.solve(lit2val(assumps));

  _model.clear();
  if (rv) {
      _model.growTo(_nvars + 1, Minisat::l_Undef);
      for (int v = _nvars; v; v--) {
          const int vval = sms.solver->val(v);
          _model[v] = (vval == 0)
                          ? Minisat::l_Undef
                          : (vval < 0 ? Minisat::l_False : Minisat::l_True);
      }
  } else {
      _conflict.clear();
      for (int i = 0; i < assumps.size(); ++i) {
          const auto val = lit2val(assumps[i]);
          if (sms.solver->failed(val))
              _conflict.insert(~assumps[i]);
      }
  }
  return rv;
}

inline bool SMSWrap::solve() {
    const bool r = sms.GraphSolver::solve();
    _model.clear();
    if (r) {
        _model.growTo(_nvars + 1, Minisat::l_Undef);
        for (int v = _nvars; v; v--) {
            const int vval = sms.solver->val(v);
            _model[v] = (vval == 0)
                            ? Minisat::l_Undef
                            : (vval < 0 ? Minisat::l_False : Minisat::l_True);
        }
    }
    return r;
}
} // namespace SATSPC
