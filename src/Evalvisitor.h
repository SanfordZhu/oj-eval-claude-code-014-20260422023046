#pragma once
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include "Python3ParserBaseVisitor.h"
#include "Python3Parser.h"
#include <any>
#include <map>
#include <string>
#include <vector>

struct BigInt {
    static constexpr int base = 1000000000;
    std::vector<int> d; // little-endian
    int sign = 1;
    BigInt() = default;
    BigInt(long long v){ *this = v; }
    BigInt(const std::string &s){ fromString(s); }
    BigInt& operator=(long long v){ d.clear(); sign = v<0?-1:1; unsigned long long x = v<0?-v:v; while(x){ d.push_back(x%base); x/=base; } return *this; }
    void trim(){ while(!d.empty() && d.back()==0) d.pop_back(); if(d.empty()) sign=1; }
    void fromString(const std::string &s){ sign=1; d.clear(); size_t i=0; if(i<s.size() && (s[i]=='+'||s[i]=='-')){ if(s[i]=='-') sign=-1; i++; } for(size_t j=s.size(); j>i;){ size_t k = (j>=9? j-9: i); int chunk = std::stoi(s.substr(k, j-k)); d.push_back(chunk); j=k; } trim(); }
    static int cmpAbs(const BigInt &a, const BigInt &b){ if(a.d.size()!=b.d.size()) return a.d.size()<b.d.size()?-1:1; for(int i=(int)a.d.size()-1;i>=0;--i){ if(a.d[i]!=b.d[i]) return a.d[i]<b.d[i]?-1:1; } return 0; }
    friend bool operator<(const BigInt &a, const BigInt &b){ if(a.sign!=b.sign) return a.sign<b.sign; int c=cmpAbs(a,b); return a.sign==1? c<0 : c>0; }
    friend bool operator==(const BigInt &a, const BigInt &b){ return a.sign==b.sign && a.d==b.d; }
    static BigInt addAbs(const BigInt &a, const BigInt &b){ BigInt r; r.sign=1; int n=std::max(a.d.size(), b.d.size()); r.d.resize(n); long long carry=0; for(int i=0;i<n;++i){ long long s = carry + (i<(int)a.d.size()?a.d[i]:0) + (i<(int)b.d.size()?b.d[i]:0); r.d[i]=int(s%base); carry=s/base; } if(carry) r.d.push_back((int)carry); return r; }
    static BigInt subAbs(const BigInt &a, const BigInt &b){ BigInt r; r.sign=1; r.d.resize(a.d.size()); long long carry=0; for(size_t i=0;i<a.d.size();++i){ long long s = (long long)a.d[i] - (i<b.d.size()?b.d[i]:0) - carry; if(s<0){ s+=base; carry=1; } else carry=0; r.d[i]=(int)s; } r.trim(); return r; }
    friend BigInt operator+(const BigInt &a, const BigInt &b){ if(a.sign==b.sign){ BigInt r=addAbs(a,b); r.sign=a.sign; return r; } int c=cmpAbs(a,b); if(c==0) return BigInt(0); if(c>0){ BigInt r=subAbs(a,b); r.sign=a.sign; return r; } BigInt r=subAbs(b,a); r.sign=b.sign; return r; }
    friend BigInt operator-(const BigInt &a, const BigInt &b){ BigInt nb=b; nb.sign*=-1; return a+nb; }
    friend BigInt operator*(const BigInt &a, const BigInt &b){ if(a.d.empty()||b.d.empty()) return BigInt(0); BigInt r; r.sign=a.sign*b.sign; r.d.assign(a.d.size()+b.d.size(),0); for(size_t i=0;i<a.d.size();++i){ long long carry=0; for(size_t j=0;j<b.d.size();++j){ long long cur = r.d[i+j] + (long long)a.d[i]*b.d[j] + carry; r.d[i+j]=(int)(cur%base); carry=cur/base; } size_t j=a.d.size()+i; while(carry){ long long cur = r.d[j] + carry; r.d[j]=(int)(cur%base); carry=cur/base; j++; } } r.trim(); return r; }
    static void mulInt(BigInt &r, int m){ if(m==0 || r.d.empty()){ r.d.clear(); r.sign=1; return; } long long carry=0; for(size_t i=0;i<r.d.size();++i){ long long cur = (long long)r.d[i]*m + carry; r.d[i]=(int)(cur%base); carry=cur/base; } while(carry){ r.d.push_back((int)(carry%base)); carry/=base; } }
    static BigInt divmod(const BigInt &a1, const BigInt &b1, BigInt &rem){ BigInt a=a1, b=b1; a.sign=b.sign=1; rem=BigInt(0); BigInt q; if(b.d.empty()) return q; q.d.assign(a.d.size(),0);
        BigInt cur; for(int i=(int)a.d.size()-1;i>=0;--i){ // cur = cur*base + a.d[i]
            if(!cur.d.empty()) cur.d.insert(cur.d.begin(), 0); else cur.d.push_back(0);
            cur.d[0]=a.d[i]; cur.trim(); // estimate quotient digit
            int qt=0; if(!cur.d.empty()){ long long top = cur.d.size()>=2? (long long)cur.d.back()*base + cur.d[cur.d.size()-2] : cur.d.back(); long long dv = b.d.size()>=2? (long long)b.d.back()*base + b.d[b.d.size()-2] : b.d.back(); qt = (int)(top / dv); if(qt>=base) qt=base-1; }
            // subtract qt*b while too big
            BigInt t=b; mulInt(t, qt); while(cmpAbs(cur,t)<0){ qt--; t=b; mulInt(t, qt); }
            cur = subAbs(cur,t); q.d[i]=qt; }
        q.trim(); rem=cur; q.sign = a1.sign*b1.sign; rem.sign = a1.sign; if(q.d.empty()) q.sign=1; if(rem.d.empty()) rem.sign=1; return q; }
    friend BigInt operator/(const BigInt &a, const BigInt &b){ BigInt rem; BigInt q=divmod(a,b,rem); // floor division
        if((a.sign*b.sign)<0 && !rem.d.empty()) q = q - BigInt(1); return q; }
    friend BigInt operator%(const BigInt &a, const BigInt &b){ BigInt rem; BigInt q=divmod(a,b,rem); if((a.sign*b.sign)<0 && !rem.d.empty()) rem = rem + b; return rem; }
};

struct Value {
    enum Type {TNone, TBool, TInt, TFloat, TString};
    Type type=TNone; bool b=false; BigInt i; double f=0.0; std::string s;
    static Value None(){ return Value(); }
    static Value Bool(bool v){ Value x; x.type=TBool; x.b=v; return x; }
    static Value Int(const BigInt &v){ Value x; x.type=TInt; x.i=v; return x; }
    static Value Float(double v){ Value x; x.type=TFloat; x.f=v; return x; }
    static Value Str(std::string v){ Value x; x.type=TString; x.s=std::move(v); return x; }
};

struct FunctionDef { std::vector<std::string> params; std::vector<bool> hasDefault; std::vector<Python3Parser::TestContext*> defaults; Python3Parser::SuiteContext* body=nullptr; };

struct Env { std::vector<std::map<std::string, Value>> scopes; std::map<std::string, FunctionDef> funcs; Env(){ scopes.emplace_back(); } void push(){ scopes.emplace_back(); } void pop(){ scopes.pop_back(); }
    Value get(const std::string &name){ for(int k=(int)scopes.size()-1;k>=0;--k){ auto it=scopes[k].find(name); if(it!=scopes[k].end()) return it->second; } auto it0=scopes[0].find(name); if(it0!=scopes[0].end()) return it0->second; return Value::None(); }
    void set(const std::string &name, const Value &v){ scopes.back()[name]=v; } };

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
