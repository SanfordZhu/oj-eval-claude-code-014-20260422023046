#pragma once
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include <cstdio>
#define EOF (-1)
#include <boost/multiprecision/cpp_int.hpp>
#undef EOF
#include "Python3ParserBaseVisitor.h"
#include "Python3Parser.h"
#include <any>
#include <map>
#include <string>
#include <vector>

using boost::multiprecision::cpp_int;

struct Value {
    enum Type {TNone, TBool, TInt, TFloat, TString};
    Type type=TNone; bool b=false; cpp_int i=0; double f=0.0; std::string s;
    static Value None(){ return Value(); }
    static Value Bool(bool v){ Value x; x.type=TBool; x.b=v; return x; }
    static Value Int(const cpp_int &v){ Value x; x.type=TInt; x.i=v; return x; }
    static Value Float(double v){ Value x; x.type=TFloat; x.f=v; return x; }
    static Value Str(std::string v){ Value x; x.type=TString; x.s=std::move(v); return x; }
};

struct FunctionDef {
    std::vector<std::string> params;
    std::vector<bool> hasDefault;
    std::vector<Python3Parser::TestContext*> defaults;
    Python3Parser::SuiteContext* body=nullptr;
};

struct Env {
    std::vector<std::map<std::string, Value>> scopes; // 0 = global
    std::map<std::string, FunctionDef> funcs;
    Env(){ scopes.emplace_back(); }
    void push(){ scopes.emplace_back(); }
    void pop(){ scopes.pop_back(); }
    Value get(const std::string &name){ for(int k=(int)scopes.size()-1;k>=0;--k){ auto it=scopes[k].find(name); if(it!=scopes[k].end()) return it->second; } auto it0=scopes[0].find(name); if(it0!=scopes[0].end()) return it0->second; return Value::None(); }
    void set(const std::string &name, const Value &v){ scopes.back()[name]=v; }
};

class EvalVisitor : public Python3ParserBaseVisitor {
public:
    Env env;
    std::any visitFile_input(Python3Parser::File_inputContext *ctx) override;
    std::any visitStmt(Python3Parser::StmtContext *ctx) override;
    std::any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override;
    std::any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override;
    std::any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override;
    std::any visitFuncdef(Python3Parser::FuncdefContext *ctx) override;
    std::any visitParameters(Python3Parser::ParametersContext *ctx) override;
    std::any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override;
    std::any visitTestlist(Python3Parser::TestlistContext *ctx) override;
    std::any visitTest(Python3Parser::TestContext *ctx) override;
    std::any visitOr_test(Python3Parser::Or_testContext *ctx) override;
    std::any visitAnd_test(Python3Parser::And_testContext *ctx) override;
    std::any visitNot_test(Python3Parser::Not_testContext *ctx) override;
    std::any visitComparison(Python3Parser::ComparisonContext *ctx) override;
    std::any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override;
    std::any visitTerm(Python3Parser::TermContext *ctx) override;
    std::any visitFactor(Python3Parser::FactorContext *ctx) override;
    std::any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override;
    std::any visitTrailer(Python3Parser::TrailerContext *ctx) override;
    std::any visitAtom(Python3Parser::AtomContext *ctx) override;
    std::any visitFormat_string(Python3Parser::Format_stringContext *ctx) override;
    std::any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override;
    std::any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override;
    std::any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override;
    std::any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override;
    std::any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override;
    std::any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override;
    std::any visitSuite(Python3Parser::SuiteContext *ctx) override;
};

#endif//PYTHON_INTERPRETER_EVALVISITOR_H
