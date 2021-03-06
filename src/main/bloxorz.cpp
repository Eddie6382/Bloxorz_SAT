#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <time.h>
#include <sys/resource.h> 
#include "constraint.h"

using namespace std;

#define MAXLEN 10000
inline long hashNode(int i, int j, int a) { return ((long)i*MAXLEN + (long)j)*10 + (long)a; }
inline long hashMove(int i, int j, int a, int m) {
   return ((long)i*MAXLEN + (long)j)*100 + (long)a*10 + (long)m;
}
inline long hashDistance(int i, int j, int a, int p) {
   return ((long)p*MAXLEN*MAXLEN + (long)i*MAXLEN + (long)j)*10 + (long)a;
}

long get_mem_usage() {
    struct rusage mysuage;
    getrusage(RUSAGE_SELF, &mysuage);
    return mysuage.ru_maxrss;
}

class Manager {
public:
   Manager() {}
   ~Manager() {};

   void readMap(fstream &);
   void initVariable();
   void genProofModel(SatSolver& );
   void reportResult(const SatSolver&, bool);
   void printStat();
   void printPath(SatSolver& );
private:
   vector<vector<int> > _map;
   map<long, Variable *> _nodes;
   map<long, Variable *> _moves;
   // map<long, Variable *> _distances;

   int _n, _m;
   int _iStart, _jStart;
   int _iEnd, _jEnd;
   int _mapSize;
};

void Manager::printStat() {
   cout << "map boundary (_m, _n) = (" << _m << ", " << _n << "), size = " << _mapSize << "\n";
   cout << "starting index = (" << _iStart << ", " << _jStart << ")\n";
   cout << "ending index   = (" << _iEnd << ", " << _jEnd << ")\n\n";
   cout << "nodes size     = " << _nodes.size() << "\n";
   cout << "moves size     = " << _moves.size() << "\n";
}

void Manager::printPath(SatSolver &s) {
   vector<Variable *> paths;
   vector<int> actions;
   map<int, string> moveName;
   moveName[UP]="UP"; moveName[DOWN]="DOWN"; moveName[RIGHT]="RIGHT"; moveName[LEFT]="LEFT"; 

   paths.push_back(_nodes[hashNode(_iStart, _jStart, S)]);
   int i = _iStart, j = _jStart, m=S;
   bool error;
   while (!((i==_iEnd) && (j==_jEnd) && (m==S))) {
      error = true;
      for (int a=0; a<4; ++a) {
         if (s.getValue(_moves[hashMove(i, j, m, a)]->getVar()) == 1) {
            error = false;
            switch(m) {
               case S:
                  switch(a) {
                     case UP:
                        i=i; j=j+1; m=Ly; break;
                     case DOWN:
                        i=i; j=j-2; m=Ly; break;
                     case RIGHT:
                        i=i+1; j=j; m=Lx; break;
                     case LEFT:
                        i=i-2; j=j; m=Lx; break;
                  }
                  break;
               case Lx:
                  switch(a) {
                     case UP:
                        i=i; j=j+1; m=Lx; break;
                     case DOWN:
                        i=i; j=j-1; m=Lx; break;
                     case RIGHT:
                        i=i+2; j=j; m=S; break;
                     case LEFT:
                        i=i-1; j=j; m=S; break;
                  }
                  break;
               case Ly:
                  switch(a) {
                     case UP:
                        i=i; j=j+2; m=S; break;
                     case DOWN:
                        i=i; j=j-1; m=S; break;
                     case RIGHT:
                        i=i+1; j=j; m=Ly; break;
                     case LEFT:
                        i=i-1; j=j; m=Ly; break;
                  }
                  break;
            }

            actions.push_back(a);
            paths.push_back(_nodes[hashNode(i, j, m)]);
         }
      }
      if (error) {
         cout << "Something wrong, some node has no out-going edges\n";
         break;
      }
   }

   for (int step=0; step<actions.size(); ++step) {
      cout << "state " << paths[step]->getName() << "\n";
      cout << "                        step " << step << " : " << moveName[actions[step]] << "\n";
   }
   cout << "state  " << paths[actions.size()]->getName() << "\n";
}

void Manager::readMap(fstream &fin) {
   string line;
   int s;
   while (getline(fin, line)) {
      int size = line.size();
      vector<int> tem;
      for (int i=0; i<size; ++i) {
         if (line[i] <= 57 && line[i] >=  48)
            s = line[i] - 48;
         else
            break;
         tem.push_back(s); 
      }
      _map.push_back(tem);
   }

   _m = _map.size();
   _n = _map.size() ? _map[0].size() : 0;

   _mapSize = 0;
   for (int i=0; i<_m; ++i) {
      for (int j=0; j<_n; ++j) {
         if (_map[i][j] == 2) {
            _iStart = i;
            _jStart = j;
         }
         if (_map[i][j] == 3) {
            _iEnd = i;
            _jEnd = j;
         }
         if (_map[i][j] != 0) _mapSize++;
      }
   }
}

void Manager::initVariable() {
   // Init Variables
   for (int i=0; i<_m; ++i) {
      for (int j=0; j<_n; ++j) {
         if (_map[i][j] != 0) {
            // for "b"
            string stateIx = to_string(i) + "_" + to_string(j) + "_Lx" ;
            string stateIy = to_string(i) + "_" + to_string(j) + "_Ly" ;
            string stateS = to_string(i) + "_" + to_string(j) + "_S" ;
            _nodes[hashNode(i, j, Lx)] = new Variable("b_" + stateIx);
            _nodes[hashNode(i, j, Ly)] = new Variable("b_" + stateIy);
            _nodes[hashNode(i, j, S )] = new Variable("b_" + stateS);

            // for "move"
            _moves[hashMove(i, j, Lx, UP)]    = new Variable("move_" + stateIx + "_up");
            _moves[hashMove(i, j, Lx, DOWN)]  = new Variable("move_" + stateIx + "_down");
            _moves[hashMove(i, j, Lx, RIGHT)] = new Variable("move_" + stateIx + "_right");
            _moves[hashMove(i, j, Lx, LEFT)]  = new Variable("move_" + stateIx + "_left");

            _moves[hashMove(i, j, Ly, UP)]    = new Variable("move_" + stateIy + "_up");
            _moves[hashMove(i, j, Ly, DOWN)]  = new Variable("move_" + stateIy + "_down");
            _moves[hashMove(i, j, Ly, RIGHT)] = new Variable("move_" + stateIy + "_right");
            _moves[hashMove(i, j, Ly, LEFT)]  = new Variable("move_" + stateIy + "_left");

            _moves[hashMove(i, j, S , UP)]    = new Variable("move_" + stateS + "_up");
            _moves[hashMove(i, j, S , DOWN)]  = new Variable("move_" + stateS + "_down");
            _moves[hashMove(i, j, S , RIGHT)] = new Variable("move_" + stateS + "_right");
            _moves[hashMove(i, j, S , LEFT)]  = new Variable("move_" + stateS + "_left");
         }
      }
   }
}

void Manager::genProofModel(SatSolver& s) {
// Allocate and record variables
   for (auto it: _nodes) {
      Var v = s.newVar();
      it.second->setVar(v);
   }
   for (auto it: _moves) {
      Var v = s.newVar();
      it.second->setVar(v);
   }

// - If a state is choosen, some related states are disabled
   for (int i=0; i<_m; ++i) {
      for (int j=0; j<_n; ++j) {
         if (_map[i][j]) {
            // Lx
            s.addOr2CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _nodes[hashNode(i, j, Ly)]->getVar(), true);
            s.addOr2CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _nodes[hashNode(i, j, S )]->getVar(), true);
            if (i+1 < _m && _map[i+1][j] != 0) {
               s.addOr2CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _nodes[hashNode(i+1, j, Lx)]->getVar(), true);
               s.addOr2CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _nodes[hashNode(i+1, j, Ly)]->getVar(), true);
               s.addOr2CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _nodes[hashNode(i+1, j, S )]->getVar(), true);
            }
            else
               s.addSingleCNF(_nodes[hashNode(i, j, Lx)]->getVar(), true);
            // Ly
            s.addOr2CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _nodes[hashNode(i, j, Lx)]->getVar(), true);
            s.addOr2CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _nodes[hashNode(i, j, S )]->getVar(), true);
            if (j+1 < _n && _map[i][j+1] != 0) {
               s.addOr2CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _nodes[hashNode(i, j+1, Lx)]->getVar(), true);
               s.addOr2CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _nodes[hashNode(i, j+1, Ly)]->getVar(), true);
               s.addOr2CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _nodes[hashNode(i, j+1, S )]->getVar(), true);
            }
            else
               s.addSingleCNF(_nodes[hashNode(i, j, Ly)]->getVar(), true);
            // S
            s.addOr2CNF(_nodes[hashNode(i, j, S)]->getVar(), true, _nodes[hashNode(i, j, Lx)]->getVar(), true);
            s.addOr2CNF(_nodes[hashNode(i, j, S)]->getVar(), true, _nodes[hashNode(i, j, Ly)]->getVar(), true);
         }
      }
   }
   // set start node and end node to be selected
   s.addSingleCNF(_nodes[hashNode(_iStart, _jStart, S)]->getVar(), false);
   s.addSingleCNF(_nodes[hashNode(_iEnd, _jEnd, S)]->getVar(), false);
   

// - Choosen b_ija can form a single path
   // P1
   //    A node on the path has its "one" out going edge except end point (node <--> move)
   for (int i=0; i<_m; ++i) {
      for (int j=0; j<_n; ++j) {
         if (_map[i][j] != 0 && !(i==_iEnd && j==_jEnd)) {
            for (int a=0; a<3; ++a) {
               // node --> move
               vector<Var> vars;
               vector<bool> fs;
               vars.push_back(_nodes[hashNode(i, j, a)]->getVar()); fs.push_back(true);
               for (int m=0; m<4; ++m) {
                  vars.push_back(_moves[hashMove(i, j, a, m)]->getVar());
                  fs.push_back(false);
               }
               s.addSeqCNF(vars, fs);

               // node <-- move
               for (int m=0; m<4; ++m)
                  s.addOr2CNF(_moves[hashMove(i, j, a, m)]->getVar(), true, _nodes[hashNode(i, j, a)]->getVar(), false);

               // no two moves are selected same time
               for (int m1=0; m1<4; ++m1) {
                  for (int m2=m1+1; m2<4; ++m2)
                     s.addOr2CNF(_moves[hashMove(i, j, a, m1)]->getVar(), true, _moves[hashMove(i, j, a, m2)]->getVar(), true);
               }
            }
         }
      }
   }
   //    A node on the path has its "one" in-going edge except start point (node <--> move)
   for (int i=0; i<_m; ++i) {
      for (int j=0; j<_n; ++j) {
         if (_map[i][j] != 0 && !(i==_iStart && j==_jStart)) {
            // Standing
            vector<Variable *> temp;
            vector<Var> vars;
            vector<bool> fs;
            if (j+2<_n && (_map[i][j+2] && _map[i][j+1])) temp.push_back(_moves[hashMove(i, j+1, Ly, DOWN)]);
            if (j-2>=0 && (_map[i][j-1] && _map[i][j-2])) temp.push_back(_moves[hashMove(i, j-2, Ly, UP)]);
            if (i+2<_m && (_map[i+2][j] && _map[i+1][j])) temp.push_back(_moves[hashMove(i+1, j, Lx, LEFT)]);
            if (i-2>=0 && (_map[i-1][j] && _map[i-2][j])) temp.push_back(_moves[hashMove(i-2, j, Lx, RIGHT)]);
            vars.push_back(_nodes[hashNode(i, j, S)]->getVar()); fs.push_back(true);
            for (int m=0; m<temp.size(); ++m) {
               vars.push_back(temp[m]->getVar());
               fs.push_back(false);
            }
            s.addSeqCNF(vars, fs);
            for (int m1=0; m1<temp.size(); ++m1)
               for (int m2=m1+1; m2<temp.size(); ++m2)
                  s.addOr2CNF(temp[m1]->getVar(), true, temp[m2]->getVar(), true);

            // Lying-X
            temp.clear(); vars.clear(); fs.clear();
            if (i+1<_m && j+1<_n && (_map[i][j+1] && _map[i+1][j+1])) temp.push_back(_moves[hashMove(i, j+1, Lx, DOWN)]);
            if (i+1<_m && j-1>=0 && (_map[i][j-1] && _map[i+1][j-1])) temp.push_back(_moves[hashMove(i, j-1, Lx, UP)]);
            if (i+2<_m && (_map[i+2][j])) temp.push_back(_moves[hashMove(i+2, j, S, LEFT)]);
            if (i-1>=0 && (_map[i-1][j])) temp.push_back(_moves[hashMove(i-1, j, S, RIGHT)]);
            vars.push_back(_nodes[hashNode(i, j, Lx)]->getVar()); fs.push_back(true);
            for (int m=0; m<temp.size(); ++m) {
               vars.push_back(temp[m]->getVar());
               fs.push_back(false);
            }
            s.addSeqCNF(vars, fs);
            for (int m1=0; m1<temp.size(); ++m1)
               for (int m2=m1+1; m2<temp.size(); ++m2)
                  s.addOr2CNF(temp[m1]->getVar(), true, temp[m2]->getVar(), true);

            // Lying-Y
            temp.clear(); vars.clear(); fs.clear();
            if (j+2<_n && (_map[i][j+2])) temp.push_back(_moves[hashMove(i, j+2, S, DOWN)]);
            if (j-1>=0 && (_map[i][j-1])) temp.push_back(_moves[hashMove(i, j-1, S, UP)]);
            if (i+1<_m && j+1<_n && (_map[i+1][j] && _map[i+1][j+1])) temp.push_back(_moves[hashMove(i+1, j, Ly,LEFT)]);
            if (i-1>=0 && j+1<_n && (_map[i-1][j] && _map[i-1][j+1])) temp.push_back(_moves[hashMove(i-1, j, Ly, RIGHT)]);
            vars.push_back(_nodes[hashNode(i, j, Ly)]->getVar()); fs.push_back(true);
            for (int m=0; m<temp.size(); ++m) {
               vars.push_back(temp[m]->getVar());
               fs.push_back(false);
            }
            s.addSeqCNF(vars, fs);
            for (int m1=0; m1<temp.size(); ++m1)
               for (int m2=m1+1; m2<temp.size(); ++m2)
                  s.addOr2CNF(temp[m1]->getVar(), true, temp[m2]->getVar(), true);
         }
      }
   }
   //       end point has no output edge
   for (int m=0; m<4; ++m)
      s.addSingleCNF(_moves[hashMove(_iEnd, _jEnd, S, m)]->getVar(), true);
   //       start point has no input edge
   if (_jStart+2<_n && (_map[_iStart][_jStart+2] && _map[_iStart][_jStart+1])) s.addSingleCNF(_moves[hashMove(_iStart, _jStart+1, Ly, DOWN)]->getVar(), true);
   if (_jStart-2>=0 && (_map[_iStart][_jStart-1] && _map[_iStart][_jStart-2])) s.addSingleCNF(_moves[hashMove(_iStart, _jStart-2, Ly, UP)]->getVar(), true);
   if (_iStart+2<_m && (_map[_iStart+2][_jStart] && _map[_iStart+1][_jStart])) s.addSingleCNF(_moves[hashMove(_iStart+1, _jStart, Lx, LEFT)]->getVar(), true);
   if (_iStart-2>=0 && (_map[_iStart-1][_jStart] && _map[_iStart-2][_jStart])) s.addSingleCNF(_moves[hashMove(_iStart-2, _jStart, Lx, RIGHT)]->getVar(), true);
   //       exclude all illegal state at the boundary
   for (int i=0; i<_m; ++i) {
      for (int j=0; j<_n; ++j) {
         if (_map[i][j]) {
            // Stading
            // Lying-x
            if (!(i+1<_m && _map[i+1][j]))
               s.addSingleCNF(_nodes[hashNode(i, j, Lx)]->getVar(), true);
            // Lying-y
            if (!(j+1<_n && _map[i][j+1]))
               s.addSingleCNF(_nodes[hashNode(i, j, Ly)]->getVar(), true);
         }
      }
   }

   // -----------------------------------------------------------------------------
   // P2: One output and one input on the path
   //       output node on the path
   for (int i=0; i<_m; ++i) {
      for (int j=0; j<_n; ++j) {
         if (_map[i][j] != 0 && !(i==_iEnd && j==_jEnd)) {
            // Standing
            if (j+2<_n && (_map[i][j+2] && _map[i][j+1])) {
               s.addOr3CNF(_nodes[hashNode(i, j, S)]->getVar(), true, _moves[hashMove(i, j, S, UP)]->getVar(), true, _nodes[hashNode(i, j+1, Ly)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, S, UP)]->getVar(), true);
            if (j-2>=0 && (_map[i][j-1] && _map[i][j-2])) {
               s.addOr3CNF(_nodes[hashNode(i, j, S)]->getVar(), true, _moves[hashMove(i, j, S, DOWN)]->getVar(), true, _nodes[hashNode(i, j-2, Ly)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, S, DOWN)]->getVar(), true);
            if (i+2<_m && (_map[i+2][j] && _map[i+1][j])) {
               s.addOr3CNF(_nodes[hashNode(i, j, S)]->getVar(), true, _moves[hashMove(i, j, S, RIGHT)]->getVar(), true, _nodes[hashNode(i+1, j, Lx)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, S, RIGHT)]->getVar(), true);
            if (i-2>=0 && (_map[i-1][j] && _map[i-2][j])) {              
               s.addOr3CNF(_nodes[hashNode(i, j, S)]->getVar(), true, _moves[hashMove(i, j, S, LEFT)]->getVar(), true, _nodes[hashNode(i-2, j, Lx)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, S, LEFT)]->getVar(), true);

            // Lying-x
            if (i+1<_m && j+1<_n && (_map[i][j+1] && _map[i+1][j+1])) {               
               s.addOr3CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _moves[hashMove(i, j, Lx, UP)]->getVar(), true, _nodes[hashNode(i, j+1, Lx)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, Lx, UP)]->getVar(), true);
            if (i+1<_m && j-1>=0 && (_map[i][j-1] && _map[i+1][j-1])) {    
               s.addOr3CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _moves[hashMove(i, j, Lx, DOWN)]->getVar(), true, _nodes[hashNode(i, j-1, Lx)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, Lx, DOWN)]->getVar(), true);
            if (i+2<_m && (_map[i+2][j])) {
               s.addOr3CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _moves[hashMove(i, j, Lx, RIGHT)]->getVar(), true, _nodes[hashNode(i+2, j, S)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, Lx, RIGHT)]->getVar(), true);
            if (i-1>=0 && (_map[i-1][j])) {
               s.addOr3CNF(_nodes[hashNode(i, j, Lx)]->getVar(), true, _moves[hashMove(i, j, Lx, LEFT)]->getVar(), true, _nodes[hashNode(i-1, j, S)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, Lx, LEFT)]->getVar(), true);

            // Lying-y
            if (j+2<_n && (_map[i][j+2])) {              
               s.addOr3CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _moves[hashMove(i, j, Ly, UP)]->getVar(), true, _nodes[hashNode(i, j+2, S)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, Ly, UP)]->getVar(), true);
            if (j-1>=0 && (_map[i][j-1])) {              
               s.addOr3CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _moves[hashMove(i, j, Ly, DOWN)]->getVar(), true, _nodes[hashNode(i, j-1, S)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, Ly, DOWN)]->getVar(), true);
            if (i+1<_m && j+1<_n && (_map[i+1][j] && _map[i+1][j+1])) {              
               s.addOr3CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _moves[hashMove(i, j, Ly, RIGHT)]->getVar(), true, _nodes[hashNode(i+1, j, Ly)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, Ly, RIGHT)]->getVar(), true);
            if (i-1>=0 && j+1<_n && (_map[i-1][j] && _map[i-1][j+1])) {       
               s.addOr3CNF(_nodes[hashNode(i, j, Ly)]->getVar(), true, _moves[hashMove(i, j, Ly, LEFT)]->getVar(), true, _nodes[hashNode(i-1, j, Ly)]->getVar(), false);
            }
            else
               s.addSingleCNF(_moves[hashMove(i, j, Ly, LEFT)]->getVar(), true);
         }
      }
   }
   //       One input node on the path (cover by P1, one input node)
}

int main(int argc, char* argv[]) {
   if (argc != 2) {
      cout << "usage: bloxorz <input_map>\n";
      return -1;
   }
   fstream fin(argv[1]);

   SatSolver solver;
   solver.initialize();
    
   clock_t start = clock();

   Manager mgr;
   mgr.readMap(fin);
   mgr.initVariable();
   mgr.printStat();
   mgr.genProofModel(solver);
   cout << "=====  Begin to solve  =====\n";

   solver.assumeRelease();
   bool result = solver.assumpSolve();
   solver.printStats();
   cout << (result? "SAT\n\n" : "UNSAT\n\n");
   if (result)
      mgr.printPath(solver);

   std::cout << "\nTotal memory usage: " << get_mem_usage() << "\n";
   std::cout << "Total time usage: " << double(clock()-start)/CLOCKS_PER_SEC << " s\n";
}