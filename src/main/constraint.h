#include <iostream>
#include <vector>
#include <string>
#include "../sat/sat.h"

using namespace std;

enum VarType {
   NODE,
   MOVE,
   U,
};

enum MoveType {
   UP,
   DOWN,
   RIGHT,
   LEFT,
};


enum StateType {
   Lx,
   Ly,
   S,
};

class Variable
{
public:
   Variable(string name): _name(name) {}
   ~Variable();

   Var getVar() const { return _var; }
   void setVar(const Var& v) { _var = v; } 
private:
   string _name;
   Var    _var;
};