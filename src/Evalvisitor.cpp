#include <cstdio>
#include "Evalvisitor.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

static std::string floatToString(double x){ std::ostringstream oss; oss<<std::fixed<<std::setprecision(6)<<x; return oss.str(); }

static std::string bigIntToString(const BigInt &x){ if(x.d.empty()) return "0"; std::string s; if(x.sign==-1) s.push_back('-'); int n=x.d.size(); s += std::to_string(x.d.back()); for(int i=n-2;i>=0;--i){ std::string chunk=std::to_string(x.d[i]); s += std::string(9 - chunk.size(), '0') + chunk; } return s; }
static double bigIntToDouble(const BigInt &x){ if(x.d.empty()) return 0.0; double val=0.0; for(int i=(int)x.d.size()-1;i>=0;--i){ val = val*BigInt::base + x.d[i]; } return x.sign==-1 ? -val : val; }
static bool stringIsInteger(const std::string &s){ if(s.empty()) return false; size_t i=0; if(s[0]=='+'||s[0]=='-') i=1; if(i>=s.size()) return false; for(; i<s.size(); ++i) if(s[i]<'0'||s[i]>'9') return false; return true; }

static Value toBool(const Value &v){ switch(v.type){ case Value::TBool: return v; case Value::TInt: return Value::Bool(!v.i.d.empty()); case Value::TFloat: return Value::Bool(v.f!=0.0); case Value::TString: return Value::Bool(!v.s.empty()); default: return Value::Bool(false);} }
static Value toInt(const Value &v){ switch(v.type){ case Value::TInt: return v; case Value::TBool: return Value::Int(BigInt(v.b?1:0)); case Value::TFloat: return Value::Int(BigInt((long long)v.f)); case Value::TString: { if(stringIsInteger(v.s)) return Value::Int(BigInt(v.s)); return Value::Int(BigInt(0)); } default: return Value::Int(BigInt(0)); } }
static Value toFloat(const Value &v){ switch(v.type){ case Value::TFloat: return v; case Value::TInt: return Value::Float(bigIntToDouble(v.i)); case Value::TBool: return Value::Float(v.b?1.0:0.0); case Value::TString: return Value::Float(std::stod(v.s)); default: return Value::Float(0.0);} }
static Value toStr(const Value &v){ switch(v.type){ case Value::TString: return v; case Value::TBool: return Value::Str(v.b?"True":"False"); case Value::TInt: return Value::Str(bigIntToString(v.i)); case Value::TFloat: return Value::Str(floatToString(v.f)); default: return Value::Str("None"); } }

static Value promoteAdd(const Value &a, const Value &b, char op){ if(a.type==Value::TString && b.type==Value::TString){ if(op=='+') return Value::Str(a.s+b.s); }
    if(a.type==Value::TFloat || b.type==Value::TFloat){ double x = (a.type==Value::TFloat)? a.f : (a.type==Value::TInt? bigIntToDouble(a.i) : 0.0); double y = (b.type==Value::TFloat)? b.f : (b.type==Value::TInt? bigIntToDouble(b.i) : 0.0); if(op=='+') return Value::Float(x+y); else return Value::Float(x-y); }
    if(a.type==Value::TInt && b.type==Value::TInt){ if(op=='+') return Value::Int(a.i+b.i); else return Value::Int(a.i-b.i); }
    return Value::None(); }
static Value add(const Value &a, const Value &b){ return promoteAdd(a,b,'+'); }
static Value sub(const Value &a, const Value &b){ return promoteAdd(a,b,'-'); }
static Value mul(const Value &a, const Value &b){ if(a.type==Value::TInt && b.type==Value::TInt) return Value::Int(a.i*b.i); if(a.type==Value::TFloat || b.type==Value::TFloat){ double x = (a.type==Value::TFloat)? a.f : (a.type==Value::TInt? bigIntToDouble(a.i) : 0.0); double y = (b.type==Value::TFloat)? b.f : (b.type==Value::TInt? bigIntToDouble(b.i) : 0.0); return Value::Float(x*y); }
    if(a.type==Value::TString && b.type==Value::TInt){ long long times = (long long)(b.i.d.empty()?0:b.i.d[0]); if(times<0) times=0; std::string r; for(long long i=0;i<times;++i) r+=a.s; return Value::Str(r); }
    if(a.type==Value::TInt && b.type==Value::TString){ long long times = (long long)(a.i.d.empty()?0:a.i.d[0]); if(times<0) times=0; std::string r; for(long long i=0;i<times;++i) r+=b.s; return Value::Str(r); }
    return Value::None(); }
static Value idiv(const Value &a, const Value &b){ if(a.type==Value::TInt && b.type==Value::TInt){ return Value::Int(a.i / b.i); } if(a.type==Value::TFloat || b.type==Value::TFloat){ double x = (a.type==Value::TFloat)? a.f : (a.type==Value::TInt? bigIntToDouble(a.i):0.0); double y = (b.type==Value::TFloat)? b.f : (b.type==Value::TInt? bigIntToDouble(b.i):0.0); long long q = (long long)std::floor(x/y); return Value::Int(BigInt(q)); } return Value::None(); }
static Value fdiv(const Value &a, const Value &b){ double x = (a.type==Value::TFloat)? a.f : (a.type==Value::TInt? bigIntToDouble(a.i) : 0.0); double y = (b.type==Value::TFloat)? b.f : (b.type==Value::TInt? bigIntToDouble(b.i) : 0.0); return Value::Float(x/y); }
static Value mod(const Value &a, const Value &b){ if(a.type==Value::TInt && b.type==Value::TInt){ Value q = idiv(a,b); return Value::Int(a.i - q.i * b.i); } return Value::None(); }
static int cmp(const Value &a, const Value &b){ if(a.type==Value::TString && b.type==Value::TString){ if(a.s<b.s) return -1; if(a.s>b.s) return 1; return 0; }
    if(a.type==Value::TFloat || b.type==Value::TFloat){ double x = (a.type==Value::TFloat)? a.f : (a.type==Value::TInt? bigIntToDouble(a.i) : 0.0); double y = (b.type==Value::TFloat)? b.f : (b.type==Value::TInt? bigIntToDouble(b.i) : 0.0); if(x<y) return -1; if(x>y) return 1; return 0; }
    if(a.type==Value::TInt && b.type==Value::TInt){ if(a.i<b.i) return -1; if(a.i>b.i) return 1; return 0; }
    if(a.type==Value::TBool && b.type==Value::TBool){ if(a.b==b.b) return 0; return a.b?1:-1; }
    return 0; }

std::any EvalVisitor::visitFile_input(Python3Parser::File_inputContext *ctx){ for(auto st: ctx->stmt()) visit(st); return 0; }
std::any EvalVisitor::visitStmt(Python3Parser::StmtContext *ctx){ if(ctx->simple_stmt()) return visit(ctx->simple_stmt()); return visit(ctx->compound_stmt()); }
std::any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx){ return visit(ctx->small_stmt()); }
std::any EvalVisitor::visitSmall_stmt(Python3Parser::Small_stmtContext *ctx){ if(ctx->expr_stmt()) return visit(ctx->expr_stmt()); if(ctx->flow_stmt()) return visit(ctx->flow_stmt()); return 0; }

std::any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx){ auto tl = ctx->testlist(); if(ctx->ASSIGN().size()>0){ Value rval = std::any_cast<Value>(visit(tl.back())); for(size_t i=0;i<ctx->ASSIGN().size();++i){ auto atom = tl[i]->test(0)->or_test()->and_test(0)->not_test(0)->comparison()->arith_expr(0)->term(0)->factor(0)->atom_expr()->atom(); auto name = atom->NAME(); if(name) env.set(name->getSymbol()->getText(), rval); } return 0; }
    if(ctx->augassign()){
        auto atom = tl[0]->test(0)->or_test()->and_test(0)->not_test(0)->comparison()->arith_expr(0)->term(0)->factor(0)->atom_expr()->atom(); std::string nm = atom->NAME()->getSymbol()->getText(); Value left = env.get(nm); Value right = std::any_cast<Value>(visit(tl[1])); auto op = ctx->augassign(); Value res; if(op->ADD_ASSIGN()) res = add(left,right); else if(op->SUB_ASSIGN()) res = sub(left,right); else if(op->MULT_ASSIGN()) res = mul(left,right); else if(op->DIV_ASSIGN()) res = fdiv(left,right); else if(op->IDIV_ASSIGN()) res = idiv(left,right); else if(op->MOD_ASSIGN()) res = mod(left,right); env.set(nm, res); return 0; }
    Value v = std::any_cast<Value>(visit(tl[0])); return v; }

std::any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext *ctx){ std::string name = ctx->NAME()->getSymbol()->getText(); FunctionDef def; auto params = ctx->parameters()->typedargslist(); if(params){ for(size_t i=0;i<params->tfpdef().size();++i){ def.params.push_back(params->tfpdef(i)->NAME()->getSymbol()->getText()); if(i<params->test().size()){ def.hasDefault.push_back(true); def.defaults.push_back(params->test(i)); } else { def.hasDefault.push_back(false); def.defaults.push_back(nullptr); } } } def.body = ctx->suite(); env.funcs[name]=def; return 0; }

std::any EvalVisitor::visitParameters(Python3Parser::ParametersContext *ctx){ return 0; }
std::any EvalVisitor::visitTypedargslist(Python3Parser::TypedargslistContext *ctx){ return 0; }

std::any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx){ if(ctx->test().size()==1) return visit(ctx->test(0)); std::vector<Value> args; for(auto t: ctx->test()) args.push_back(std::any_cast<Value>(visit(t))); return args; }

std::any EvalVisitor::visitTest(Python3Parser::TestContext *ctx){ return visit(ctx->or_test()); }
std::any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx){ Value res = std::any_cast<Value>(visit(ctx->and_test(0))); for(size_t i=1;i<ctx->and_test().size();++i){ if(toBool(res).b) return Value::Bool(true); res = std::any_cast<Value>(visit(ctx->and_test(i))); } return toBool(res); }
std::any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx){ Value res = std::any_cast<Value>(visit(ctx->not_test(0))); for(size_t i=1;i<ctx->not_test().size();++i){ if(!toBool(res).b) return Value::Bool(false); res = std::any_cast<Value>(visit(ctx->not_test(i))); } return toBool(res); }
std::any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx){ if(ctx->NOT()) return Value::Bool(!toBool(std::any_cast<Value>(visit(ctx->not_test()))).b); return visit(ctx->comparison()); }

std::any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx){ Value a = std::any_cast<Value>(visit(ctx->arith_expr(0))); for(size_t i=1;i<ctx->arith_expr().size();++i){ Value b = std::any_cast<Value>(visit(ctx->arith_expr(i))); int c = cmp(a,b); auto op = ctx->comp_op(i-1); bool ok=false; if(op->LESS_THAN()) ok = c<0; else if(op->GREATER_THAN()) ok = c>0; else if(op->EQUALS()) ok=c==0; else if(op->GT_EQ()) ok=c>=0; else if(op->LT_EQ()) ok=c<=0; else if(op->NOT_EQ_2()) ok=c!=0; if(!ok) return Value::Bool(false); a=b; } return Value::Bool(true); }

std::any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx){ Value v = std::any_cast<Value>(visit(ctx->term(0))); for(size_t i=1;i<ctx->term().size();++i){ auto op = ctx->addorsub_op(i-1); Value r = std::any_cast<Value>(visit(ctx->term(i))); if(op->ADD()) v=add(v,r); else v=sub(v,r); } return v; }
std::any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx){ Value v = std::any_cast<Value>(visit(ctx->factor(0))); for(size_t i=1;i<ctx->factor().size();++i){ auto op=ctx->muldivmod_op(i-1); Value r = std::any_cast<Value>(visit(ctx->factor(i))); if(op->STAR()) v=mul(v,r); else if(op->DIV()) v=fdiv(v,r); else if(op->IDIV()) v=idiv(v,r); else v=mod(v,r); } return v; }
std::any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx){ if(ctx->atom_expr()) return visit(ctx->atom_expr()); auto v = std::any_cast<Value>(visit(ctx->factor())); if(ctx->MINUS()){
        if(v.type==Value::TInt){ return Value::Int(BigInt(0) - v.i); } else if(v.type==Value::TFloat){ return Value::Float(-v.f);} }
    return v; }

std::any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx){ Value v = std::any_cast<Value>(visit(ctx->atom())); if(ctx->trailer()){ auto tr = ctx->trailer(); auto nameNode = ctx->atom()->NAME(); if(nameNode){ std::string fn = nameNode->getSymbol()->getText(); std::vector<Value> args; if(tr->arglist()){ for(auto arg: tr->arglist()->argument()){ args.push_back(std::any_cast<Value>(visit(arg->test(0)))); } }
            if(fn=="print"){ for(size_t i=0;i<args.size();++i){ std::string out = toStr(args[i]).s; std::cout<<out; if(i+1<args.size()) std::cout<<" "; } std::cout<<"\n"; return Value::None(); }
            if(fn=="int"){ return toInt(args[0]); }
            if(fn=="float"){ return toFloat(args[0]); }
            if(fn=="str"){ return toStr(args[0]); }
            if(fn=="bool"){ return toBool(args[0]); }
            auto it = env.funcs.find(fn); if(it!=env.funcs.end()){ FunctionDef &def = it->second; env.push(); for(size_t i=0;i<def.params.size();++i){ Value val; if(i<args.size()) val=args[i]; else if(def.hasDefault[i]) val = std::any_cast<Value>(visit(def.defaults[i])); else val = Value::None(); env.set(def.params[i], val); }
                visit(def.body); env.pop(); return Value::None(); }
        }
    }
    return v; }

std::any EvalVisitor::visitTrailer(Python3Parser::TrailerContext *ctx){ return 0; }
std::any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx){ if(ctx->NAME()){ std::string nm = ctx->NAME()->getSymbol()->getText(); return env.get(nm); } if(ctx->NUMBER()){ auto t = ctx->NUMBER()->getSymbol()->getText(); if(stringIsInteger(t)) return Value::Int(BigInt(t)); else return Value::Float(std::stod(t)); } if(!ctx->STRING().empty()){ std::string s; for(auto st: ctx->STRING()){ auto raw = st->getSymbol()->getText(); if(raw.size()>=2) s += raw.substr(1, raw.size()-2); }
        return Value::Str(s); } if(ctx->TRUE()) return Value::Bool(true); if(ctx->FALSE()) return Value::Bool(false); if(ctx->NONE()) return Value::None(); if(ctx->OPEN_PAREN()){ return std::any_cast<Value>(visit(ctx->test())); } if(ctx->format_string()){ return visit(ctx->format_string()); } return Value::None(); }

std::any EvalVisitor::visitFormat_string(Python3Parser::Format_stringContext *ctx){ std::string res; for(auto t: ctx->FORMAT_STRING_LITERAL()) res += t->getSymbol()->getText(); for(size_t i=0;i<ctx->OPEN_BRACE().size();++i){ auto val = std::any_cast<Value>(visit(ctx->testlist(i))); res += toStr(val).s; } return Value::Str(res); }

std::any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx){ if(ctx->break_stmt()) return visit(ctx->break_stmt()); if(ctx->continue_stmt()) return visit(ctx->continue_stmt()); if(ctx->return_stmt()) return visit(ctx->return_stmt()); return 0; }
struct BreakSignal{}; struct ContinueSignal{}; struct ReturnSignal{ std::any v; };
std::any EvalVisitor::visitBreak_stmt(Python3Parser::Break_stmtContext *ctx){ throw BreakSignal(); }
std::any EvalVisitor::visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx){ throw ContinueSignal(); }
std::any EvalVisitor::visitReturn_stmt(Python3Parser::Return_stmtContext *ctx){ ReturnSignal r{ ctx->testlist()? visit(ctx->testlist()) : Value::None() }; throw r; }

std::any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx){ for(size_t i=0;i<ctx->test().size();++i){ Value v = std::any_cast<Value>(visit(ctx->test(i))); if(toBool(v).b){ visit(ctx->suite(i)); return 0; } } if(ctx->ELSE()) visit(ctx->suite(ctx->suite().size()-1)); return 0; }

std::any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx){ while(toBool(std::any_cast<Value>(visit(ctx->test()))).b){ try{ visit(ctx->suite()); } catch(ContinueSignal&) { continue; } catch(BreakSignal&) { break; } } return 0; }

std::any EvalVisitor::visitSuite(Python3Parser::SuiteContext *ctx){ if(ctx->simple_stmt()) return visit(ctx->simple_stmt()); for(auto st: ctx->stmt()){ try{ visit(st); } catch(ReturnSignal &ret){ return ret.v; } } return 0; }
