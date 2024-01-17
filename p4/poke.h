#ifndef POKE_H
#define POKE_H
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <getopt.h>
#include <deque>
#include <stack>
#include <queue>
#include <set>
#include <ctype.h>
#include <algorithm>
#include <exception>
#include <map>
#include <functional>
#include <cassert>
#include <iostream>
#include <unordered_set>
#include <utility>
#include <unordered_map>
#include <exception>

// Project Identifier: 5949F553E20B650AB0FB2266D3C0822B13D248B0

using namespace std;
static option long_options[] = {
  // LONG        ARGUMENT USED?     (ignore) RETURN VALUE
  {"mode",      required_argument, nullptr, 'm'},
  {"help",      no_argument, nullptr, 'h'},
  // this one tells getopt_long that there are no more options:
  {nullptr,      0,                 nullptr,  0}
};

enum class Mode {MST,FAST,OPT};
enum class Terrain{Land,Sea,Coast};
    struct PrimData{
        double d;
        int p;
        bool k;
        Terrain terrain;
        PrimData():d(numeric_limits<double>::infinity()),p(-1),k(false),terrain(Terrain::Coast){}
        void get_terrain(int x, int y){
            if(x > 0 || y > 0){
                terrain = Terrain::Land;
                return;
            }
            if(x < 0){
                if(y < 0){
                    terrain = Terrain::Sea;
                }
                else{
                    terrain = Terrain::Coast;
                }
            }
            else{
                terrain = Terrain::Coast;
            }
        }
    };
class Opt{
public:
Opt(const vector<pair<int,int>> &v_in, const vector<size_t> &p_in, const double &sum):optSum(sum),optimal(p_in),vertices(v_in){optimal.pop_back();path = optimal;fill_distances();}
void fill_distances();
void gen_perms(size_t perm_length);
bool promising(size_t perm_length);
void getpath();
double find_estimate(size_t perm_length);
private:
    //testing only
    void debug_output(double arm1,double arm2, double estimate,size_t perm_length);
    double optSum;
    double pathSum;
    vector<size_t> path;
    vector<size_t> optimal;
    vector<pair<int,int> > vertices;
    vector<vector<double>> distances;
};
#endif
