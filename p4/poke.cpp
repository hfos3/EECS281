#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <getopt.h>
#include <deque>
#include <math.h>
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
#include <iomanip>
#include <unordered_set>
#include <utility>
#include <unordered_map>
#include <exception>
#include "poke.h"

// Project Identifier: 5949F553E20B650AB0FB2266D3C0822B13D248B0

using namespace std;

const double INF = numeric_limits<double>::infinity();
    //does not do terrain check
    double calc_dist(const vector<pair<int,int>> &vertices,size_t a, size_t b){
        //now we know regular distance calc can happen
        pair<int,int> first = vertices[a];
        pair<int,int> second = vertices[b];
        int firstdiff = first.first - second.first;
        int secdiff = first.second- second.second;
        double out = static_cast<double>(firstdiff) * static_cast<double>(firstdiff) 
        + static_cast<double>(secdiff) * static_cast<double>(secdiff);
        return out;
    }
    //returns squared value
    double squared_dist(const vector<pair<int,int>> &vertices,const vector<PrimData> &data,size_t a, size_t b){
        if(data[a].terrain != data[b].terrain){
            if(data[a].terrain != Terrain::Coast && data[b].terrain != Terrain::Coast){
                return INF;
            }
        }
        return calc_dist(vertices,a,b);
    }
    //start is if we want partial MST sum
    double find_tree(const vector<pair<int,int> > &vertices, vector<PrimData> &data, queue<pair<int,int>> &info,const size_t start){
        double sum = 0;
        size_t i = start;
        size_t cur = 0;
        while(data[cur].k == true){cur++;}
        data[cur].d = 0;
        while(i < data.size()){
            //linear search of all d values
            double minYet = INF;
            size_t vMin = data.size();//causes segfault which we lowkey want if error
            for(size_t j = 0, n = data.size(); j < n; j++){
                if(data[j].d < minYet && !data[j].k){
                    minYet = data[j].d;
                    vMin = j;
                }
            }
            //vMin is the next smallest value
            data[vMin].k = true;
            //update neighbors
            for(size_t j = 0, n = data.size(); j < n; j++){
                if(!data[j].k){
                    double val = squared_dist(vertices,data,j,vMin);
                    if(val < data[j].d){
                        data[j].d = val;
                        data[j].p = static_cast<int>(vMin);
                    }
                }
            }
            info.push({static_cast<int>(vMin),data[vMin].p});
            cur = vMin;
            sum += sqrt(data[vMin].d);
            i++;
        }
        return sum;
    }
    //fills in coords
    void read_vec(vector<pair<int,int>> &vertices){
        int x,y;
        while(cin >> x >> y){
            vertices.emplace_back(x,y);
        }
    }
    void calc_MST(const vector<pair<int,int>> &vertices){
        vector<PrimData> data(vertices.size());
        //fills in data with terrain, worth it later I think
        //can also leave as unitialized, have 4th enum value of 'unknown' that gets updated
        //like path compression, save once calculated to reuse if will be called several times
        for(size_t i = 0; i < vertices.size(); i++){data[i].get_terrain(vertices[i].first,vertices[i].second);}
        queue<pair<int,int>> min_tree;
        //fills in min_tree
        double val = find_tree(vertices,data,min_tree,0);
        min_tree.pop();
        auto print_row = [](int a, int b)
        {if(a < b){cout << a << " " << b << "\n";}else{cout << b << " " << a << "\n";}};
        cout << setprecision(2) << fixed << showpoint << val << "\n";
        while(!min_tree.empty()){
            print_row(min_tree.front().first,min_tree.front().second);
            min_tree.pop();
        }
    }
    //returns sum, fills vec path
    //using just this func for part c
    //can make quicker by having a stack of unvisited vertices, arbitrary in one step every time

    double fill_arbitrary_path(const vector<pair<int,int> > &vertices, vector<size_t> &path){
        path.push_back(0);
        path.push_back(1);
        path.push_back(0);
        double minYet = INF;
        double val = 0;
        double pathSum = sqrt(calc_dist(vertices,0,1)) * 2;
        size_t least = 0;
        for(size_t k = 2; k < vertices.size(); k++){
            minYet = INF;
            for(size_t i = 0, n = path.size() - 1; i < n; i++){
                size_t j = i + 1;
                val = -sqrt(calc_dist(vertices,path[i],path[j])) + 
                sqrt(calc_dist(vertices,path[i],k)) + sqrt(calc_dist(vertices,path[j], k));
                if(val < minYet){
                    least = i;
                    minYet = val;
                }
            }
            path.insert(path.begin() + int(least) + 1, k);
            pathSum += minYet;
        }
        return pathSum;
    }
    //arbitrary insertion
    //1.find closest node
    //loop for each n node
    // 2. arbitarily select node k
    // 3. find best edge(u,v) in current subgraph to insert node k in between u and v
    //      ex: minimuze d_kv + c_uk − d_uv
    void arbitrary(const vector<pair<int,int> > &vertices){
        vector<size_t> path;//represents path, wraps around
        path.reserve(vertices.size());
        double pathSum = fill_arbitrary_path(vertices,path);
        cout << fixed << setprecision(2) << pathSum << "\n";
        for_each(path.begin(),path.end(),[](size_t s){cout << s << " ";});cout << "\n";
    }

int main(int argc, char**argv){
    cerr << fixed << showpoint << setprecision(2);
    int choice = -1;
    bool selection = false;
    size_t num_vertices;
    cin >> num_vertices;
    vector<pair<int,int> > vertices;
    vertices.reserve(num_vertices);
    read_vec(vertices);
    //if(!selection){
    //}
    while((choice = getopt_long(argc, argv, "m:q", long_options, nullptr)) != -1){
        switch(choice){
            case 'h':
                cout << "HELP MODE\n";
                return 0;
            case 'm':
                selection = true;
                //char * debug = optarg;
                //cout << debug;
                switch(*optarg){
                    case 'M':
                        calc_MST(vertices);
                        break;
                    case 'F':
                        arbitrary(vertices);
                        break;
                    case 'O':{
                        vector<size_t> path;
                        path.reserve(num_vertices);
                        double bound = fill_arbitrary_path(vertices,path);
                        Opt opt(vertices,path,bound);
                        opt.getpath();
                        return 0;
                    }
                        break;
                   // default:cout << *optarg << endl;
                }
        }
    }
    if(!selection){
        cout << "make a selection for mode with -m\n";
        return 1;
    }
}


//ALL PART C DOWN HERE
void Opt::fill_distances(){
    distances.resize(vertices.size(),vector<double>(vertices.size(),-1));
    for(size_t i = 0, n = vertices.size(); i < n; i++){
        for(size_t j = 0; j < n; j++){
            if(distances[i][j] != -1){
                continue;
            }
            distances[i][j] = calc_dist(vertices,i,j);
            distances[j][i] = calc_dist(vertices,i,j);
        }
    }
}
void Opt::debug_output(double arm1, double arm2, double mstweight, size_t perm_length){
    double final_estimate = arm1 + arm2 + mstweight + pathSum;
    bool sat = final_estimate < optSum;
    string truth = sat ? "true" : "false";
    for (size_t i = 0; i < path.size(); ++i){
        cerr << setw(2) << path[i] << ' ';
    }
    cerr << setw(4) << perm_length << setw(10) << pathSum;
    cerr << setw(10) << arm1 << setw(10) << arm2;
    cerr << setw(10) << mstweight << setw(10) << final_estimate << "  " << truth << "\n";
}
//MAYBE USE DIST MATRIX IN PRIMS
double Opt::find_estimate(size_t perm_length){
    //arm1: beginning of path to closest MST node
    //arm2: same but from end of path(perm_length - 1)
    double min1 = INF;
    double min2 = INF;
    for(size_t i = perm_length; i < path.size(); i++){
        min1 = min(min1,distances[path[0]][path[i]]);
        min2 = min(min2,distances[path[perm_length - 1]][path[i]]);
    }
    vector<PrimData> data(vertices.size());
    for(size_t i = 0; i < perm_length; i++){
        data[path[i]].k = true;
    }
    queue<pair<int,int> > info;
    min1 = sqrt(min1); min2 = sqrt(min2);//stored squared vals
    //double weight = find_tree(vertices,data,info,perm_length);
    //debug_output(min1,min2,weight,perm_length);
    return min1 + min2 + find_tree(vertices,data,info,perm_length) + pathSum;
}

bool Opt::promising(size_t perm_length){
    if(path.size() - perm_length < 5){return true;}
    return find_estimate(perm_length) < optSum;
}

void Opt::gen_perms(size_t perm_length){
    if(perm_length == path.size()){
        double last = sqrt(distances[path[perm_length - 1]][path[0]]);
        pathSum += last;
        if(pathSum < optSum){
            optSum = pathSum;
            optimal = path;
        }
        pathSum -= last;
        return;
    }
    if(!promising(perm_length)){
        return;
    }
    for(size_t i = perm_length; i < path.size(); ++i) {
        swap(path[perm_length], path[i]);
        double initial_add = sqrt(distances[path[perm_length - 1]][path[perm_length]]);
        pathSum += initial_add;
        gen_perms(perm_length + 1);
        pathSum -= initial_add;
        swap(path[perm_length], path[i]);
  }  // for ..unpermuted elements
}

//driver for Opt
void Opt::getpath(){
   // path = {0,1,2,3,4,5,6,7,8,9,10};
    gen_perms(1);
    cout << setprecision(2) << fixed << showpoint << optSum << "\n";
    for_each(optimal.begin(),optimal.end(),[](size_t val){cout << val << " ";});
}