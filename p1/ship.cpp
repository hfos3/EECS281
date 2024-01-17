#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <getopt.h>
#include <deque>
#include <stack>
#include <queue>
#include <ctype.h>
#include "xcode_redirect.hpp"

using namespace std;
//950181F63D0A883F183EC0A5CC67B19928FE896A

static option long_options[] = {
  // LONG        ARGUMENT USED?     (ignore) RETURN VALUE
  {"stack",      no_argument, nullptr, 's'},
  {"queue",      no_argument, nullptr, 'q'},
  {"output",     required_argument, nullptr, 'o'},
  {"help",       no_argument, nullptr, 'h'},
  // this one tells getopt_long that there are no more options:
  {nullptr,      0,                 nullptr,  0}
};

struct Coordinate{
  uint32_t level;
  uint32_t row;
  uint32_t column;
};

struct Square{
  char val;
  char path;
  bool discovered;
  Square(){
    val = '.';
    path = '.';
    discovered = false;
  }
};

bool operator ==(const Coordinate &cord, const Coordinate &other){
    return (cord.level == other.level && cord.row == other.row && cord.column == other.column);
}
bool operator !=(const Coordinate &cord, const Coordinate &other){
    return !(cord == other);
}
class World{
  public:
  World(){
    loc[0] = 0;
    loc[1] = 0;
    loc[2] = 0;
  }
  Square& grab(const Coordinate &cur){
    return world[cur.level][cur.row][cur.column];
  }
    void set_discover(const Coordinate &cur){
        world[cur.level][cur.row][cur.column].discovered = true;
    }
  void set_world(const uint32_t &floors_in, const uint32_t &dimensions_in){
    floors = floors_in;
    dimensions = dimensions_in;
    Square square;
    world.resize(floors,vector<vector<Square> >(dimensions,vector<Square>(dimensions,square)));
  }
    uint32_t* get_index(){
        return loc;
    }
  void fill(char c){
    //cout << "here";
    Square square;
    square.val = c;
    world[loc[0]][loc[1]][loc[2]] = square;
    update_loc();
    }
  void set(const uint32_t &level, const uint32_t &row, const uint32_t &column, const char &c){
    world[level][row][column].val = c;
  }
    //set if you need more than a char
    void set(const Coordinate &ref, const Square &square){
        world[ref.level][ref.row][ref.column] = square;
    }
    
    void set_path(const Coordinate &loc,const Coordinate &other){
        char path = ',';
//        if(world[other.level][other.row][other.column].path != '.'){
  //          return;
    //    }
        if(loc.row < other.row){
           path = 's';
        }
        else if(loc.row > other.row){
          path = 'n';
        }
        else if(loc.column > other.column){
          path = 'w';
        }
        else if(loc.column < other.column){
          path = 'e';
        }
        else{
            uint32_t l = loc.level;
            path = static_cast<char>('0' + l);
        }
        world[other.level][other.row][other.column].path = path;
    }
    //Output
    void print_path(const Coordinate &hanger,const Coordinate &start, const bool &list_out){
      if(list_out){
          cout << "//path taken" << "\n";
      }
      Coordinate place = hanger;
      Square square = grab(place);
      stack<Coordinate> track;
      while (place != start){
          if(list_out){
              track.push(place);
              char move = square.path;
              reverse_flow(place,move);
              square = grab(place);
          }
          else{
              char move = square.path;
              reverse_flow(place,move);
              square = grab(place);
              if(square.val == 'E' && isdigit(square.path)){
                  Coordinate other = place;
                  other.level = static_cast<uint32_t>(square.path - '0');
                  char special = static_cast<char>(place.level + '0');
                  set(other.level,other.row,other.column,special);
              }
              if(!isdigit(square.val)){
                  square.val = move;
                  set(place,square);
              }
          }
      }
        
        if(list_out){
            track.push(place);
            while(!track.empty()){
              Coordinate out = track.top();
              track.pop();
                if(track.empty()){
                    break;
                }
              Coordinate other = track.top();
              Square after = grab(other);
              char move = after.path;
              if(other.level != out.level){
                  move = static_cast<char>('0' + other.level);
              }
              print_row(out,move);
            }
        }
        else{
            map_print(start);
        }
    }
    void map_print(const Coordinate &start){
      cout << "Start in level " << start.level << ", row "
      << start.row << ", column " << start.column << "\n";
      for(uint32_t l = 0; l < floors; l++){
          cout << "//level " << l << "\n";
        for(uint32_t r = 0; r < dimensions; r++){
          for(uint32_t c = 0; c < dimensions; c++){
            Coordinate out;
            out.column = c;
            out.row = r;
            out.level = l;
            Square square = grab(out);
              cout << square.val;
          }
          cout << "\n";
        }
      }
    }

  private:
  void print_row(const Coordinate &out, const char &move){
    //Square square = grab(out);
    cout << "(" << out.level << "," << out.row << ","
    << out.column << "," << move << ")" << "\n";
  }
    
    //modifies update
    void reverse_flow(Coordinate & update, const char &c){
      if(c == 'n'){
        update.row++;
      }
      else if(c == 's'){
        update.row--;
      }
      else if(c == 'w'){
        update.column++;
      }
      else if(c == 'e'){
        update.column--;
      }
      else{
          update.level = static_cast<uint32_t>(c - '0');
      }
    }
  void update_loc(){
    if(loc[2] == dimensions - 1){//column at max
      if(loc[1] == dimensions - 1){//row at max
        if(loc[0] == floors - 1){//at capacity, undefined behavior, would be a segfault
          return;
        }
        loc[0]++;
        loc[1] = 0;
      }
      else{//just column at max
        loc[1]++;
      }
      loc[2] = 0;
    }
    else{//normal scenario, no overflow
      loc[2]++;
    }
    //cout << "The coordinates are " <<
    //loc[0] << " " << loc[1] << " " << loc[2] << endl;
  }
  uint32_t floors;
  uint32_t dimensions;
  uint32_t loc[3];
  vector<vector<vector<Square> > > world;
};

class Search{
  public:
  Search(bool stack_choice, bool list_out_in, bool help_mode_in){
    stack_in = stack_choice;
    list_out = list_out_in;
    help = help_mode_in;
    char mode;
    cin >> mode;
    input_map = (mode == 'M');
    cin >> floors >> dimensions;
    Square square;
    valid[0] = '.';
    valid[1] = '#';
    valid[2] = 'S';
    valid[3] = 'H';
    valid[4] = 'E';
  }
    bool contains(const char &c){
        for(int i = 0; i < 5; i++){
            if(valid[i] == c){
                return true;
            }
        }
        return false;
    }
    pair<bool,char const*> within(const int &level, const int &row, const int &column){
        if(row < 0 || row >= static_cast<int>(dimensions)){
            return {false,"Invalid map row"};
        }
        else if(column < 0 || column >= static_cast<int>(dimensions)){
            return {false,"Invalid map column"};
        }
        else if(level < 0 || level >= static_cast<int>(floors)){
            return {false,"Invalid map column"};
        }
        return {true,""};
    }
  void read_in(){
    world.set_world(floors,dimensions);
    char s;
    //int n = 0;
      try{
          while(cin >> s){
            if(s == '/'){
              string trash;
              getline(cin,trash);
            }
            else if(input_map){
              //cout << s << "is index" << n << endl;
              //n++;
                if(!contains(s)){
                    throw "Invalid map character";
                }
              if(s == 'S'){
                  uint32_t * index = world.get_index();
                  start.level = index[0];
                  start.row = index[1];
                  start.column = index[2];
              }
              world.fill(s);
            }
            else{
              int level;
              int row;
              int column;
              char what;
              cin >> level >> s >> row >>
              s >> column >> s >> what >> s;
              //char const* overflow;
              uint32_t l = static_cast<uint32_t>(level);
              uint32_t r = static_cast<uint32_t>(row);
              uint32_t c = static_cast<uint32_t>(column);
              if(!contains(what)){
                throw "Invalid map character";
              }
              pair<bool, char const*> accept = within(level,row,column);
              if(!accept.first){
                throw accept.second;
              }
              if(what == 'S'){
                  start.level = l;
                  start.row = r;
                  start.column = c;
              }
              world.set(l,r,c,what);
            }
          }
      }
      catch(const char *err){
          cerr << err << "\n";
          exit(1);
      }
  }

  bool explore(const vector<vector<int> > &directions){
    Coordinate loc;
    for(const vector<int> &direction : directions) {
      int row = (int) cur.row + direction[0];
      int col = (int) cur.column + direction[1];
      int dim = (int) dimensions;
      if (row < 0 || row >= dim || col < 0 || col >= dim) {
        continue; // skips the current direction and moves on to the next one
      }
      loc.level = cur.level;
      loc.row = static_cast<uint32_t>(row);
      loc.column = static_cast<uint32_t>(col);
      Square square = world.grab(loc);
      bool wall = (square.val == '#');
      if(square.discovered || wall){
        continue;
      }
      world.set_path(cur,loc);
      world.set_discover(loc);
      search.push_back(loc);
      if(square.val == 'H'){
        hanger = loc;
        return true;
      }
    }
    Square block = world.grab(cur);
    if(block.val == 'E'){
        explore_elevator(cur);
    }
    return false;
  }

  void explore_elevator(Coordinate ele){
      Coordinate place = ele;
    for(uint32_t l = 0; l < floors; l++){
        ele.level = l;
        Square square = world.grab(ele);
      if(square.val == 'E' && !square.discovered){
          world.set_discover(ele);
          world.set_path(place,ele);
          search.push_back(ele);
      }
    }
  }
  
  void operate(){
    const vector<vector<int> > directions = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    read_in();
    //cout << endl << "Here";
      cur = start;
      world.set_discover(start);
    while(!explore(directions)){
    //cout << "coords are " << cur.level << " " << cur.row
      //  << " " << cur.column << endl;
      Coordinate ref = cur;
      cur = next();
        if(ref == cur){
            if(list_out){
                cout << "//path taken" << "\n";
            }
            else{
                world.map_print(start);
            }
            exit(0);
        }
    }
      world.print_path(hanger,start,list_out);
  }
  
  Coordinate next(){
     if(search.empty()){//no path available
         return cur;
      }
    if(stack_in){
      Coordinate place = search.back();
      search.pop_back();
      return place;
    }
    else{
      Coordinate place = search.front();
      search.pop_front();
      return place;
    }
  }

  private:
    char valid[5];
  Coordinate start;
  Coordinate hanger;
  Coordinate cur;
  bool stack_in;
  bool list_out;
  bool help;
  bool input_map;
  deque<Coordinate> search;
  uint32_t floors;
  uint32_t dimensions;
  World world;
};

int main(int argc, char** argv){
  ios_base::sync_with_stdio(false);
  xcode_redirect(argc,argv);
  int choice = 0;
  bool mode_selected = false;
  bool stack = true;
  bool list_out = false;
  bool output_chosen = false;
  bool help_mode = false;
  try{
    while((choice = getopt_long(argc,argv,"sqo:h",long_options,nullptr)) != -1){
    switch (choice) {
    case 's':
      if(mode_selected){
        throw "Multiple routing modes specified";
      }
      mode_selected = true;
      break;
    case 'q':
      if(mode_selected){
        throw "Multiple routing modes specified";
      }
      stack = false;
      mode_selected = true;
      break;
    case 'o':
      if(output_chosen){
        throw "output mode already chosen";
      }
        if(*optarg != 'M' && *optarg != 'L'){
            throw "Invalid output mode specified";
        }
      list_out = (*optarg == 'L');
      break;
    case 'h':
      help_mode = true;
      break;
    }
  }
      if(!mode_selected){
          throw "No routing mode specified";
      }
  }
  catch(char const *e){
    cerr << e << "\n";
      exit(1);
  }
  Search search(stack,list_out,help_mode);
  search.operate();
}
