//
//  galaxy.cpp
//  p2a
//
//  Created by Henry Foster on 1/31/23.
//
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
#include <exception>
#include "P2random.h"
#include "xcode_redirect.hpp"
#include <stdio.h>

using namespace std;

static option long_options[] = {
  // LONG        ARGUMENT USED?     (ignore) RETURN VALUE
  {"verbose",      no_argument, nullptr, 'v'},
  {"median",      no_argument, nullptr, 'm'},
  {"general-eval", no_argument, nullptr, 'g'},
  {"watcher",       no_argument, nullptr, 'w'},
  // this one tells getopt_long that there are no more options:
  {nullptr,      0,                 nullptr,  0}
};
//struct CommandLine : exception{
  //  const char* what(){
    //    return "Unknown command line option";
    //}
//};
enum class State{Initial,Wait,Action};
//goes in movieData queue
struct movieData{
    uint32_t stamp;
    int force;
};
struct Movie{
    movieData jediMov;
    movieData sithMov;
    movieData maybe;
    Movie(){//sets force to -1 to show maybe is not being 'used'
        maybe.force = -1;
    }
};
struct Deployment{
    int force;
    mutable uint32_t troops;
    uint32_t time;
    uint32_t gen;
};
//comparator for force sensitivity
class JediComp{
public:
    bool operator() (const Deployment &d1, const Deployment &d2){
        if(d1.force != d2.force){
            return d1.force < d2.force;
        }
        return d1.time < d2.time;
    }
};
class SithComp{
public:
    bool operator() (const Deployment &d1, const Deployment &d2){
        if(d1.force != d2.force){
            return d1.force > d2.force;
        }
        return d1.time < d2.time;
    }
};
struct General{
    uint32_t death_count;
    uint32_t deployedJedi;
    uint32_t deployedSith;
    void print_general(uint32_t &num){
        cout << "General " << num << " deployed " << deployedJedi << " Jedi troops and " <<
        deployedSith <<" Sith troops, and " << (deployedJedi + deployedSith - death_count) <<
        "/" << (deployedJedi + deployedSith) << "\n";
    }
};
class Simulation{
public:
    //basic constructor
    Simulation(const char &in, const uint32_t &gen, const uint32_t &plan): input_mode(in),
    num_generals(gen), num_planets(plan){
        General def;
        generals.resize(num_generals,def);
        medianMode = false;
        generalMode = false;
        verboseMode = false;
        movieWatcher = false;
        input_mode = '.';
    }
    //processes command line
    void take_arguments(int argc, char** argv){
        try{
            int choice = -1;
            while((choice = getopt_long(argc, argv, "vmgw", long_options, nullptr)) != -1){
                switch(choice){
                    case 'v':
                        verboseMode = true;
                        break;
                    case 'm':
                        medianMode = true;
                        break;
                    case 'g':
                        generalMode = true;
                        break;
                    case 'w':
                        movieWatcher = true;
                        break;
                }
            }
        }
        catch(const exception &ex){
            cerr << ex.what() << "\n";
        }
    }
    class Planet{
    private:
        friend class Simulation;
        bool medianMode;
        bool generalMode;
        bool verboseMode;
        bool movieWatcher;
        Deployment recent;
        bool recentJedi;
        vector<General> &generals;
        uint32_t num;//planet num
        priority_queue<Deployment,vector<Deployment>,SithComp> sithPQ;
        priority_queue<Deployment,vector<Deployment>, JediComp> jediPQ;
        priority_queue<uint32_t> second;//greater half for med
        priority_queue<uint32_t,vector<uint32_t>,greater<uint32_t>> first;//smaller half for med
        Movie ambushMov;
        Movie attackMov;
        State attack;
        State ambush;
    public:
        Planet(vector<General> &gin): generals(gin){
            attack = State::Initial;
            ambush = State::Initial;
            medianMode = false;
            generalMode = false;
            verboseMode = false;
            movieWatcher = false;
        }
        //adjusts median pQ
        void add_to_median(const uint32_t &a){
            if(first.empty() && second.empty()){
                first.push(a);
            }
            if(a > first.top()){
                if(second.size() > first.size()){
                    first.push(second.top());
                    second.pop();
                }
                second.push(a);
            }
            else if(a < second.top()){
                if(first.size() > second.size()){
                    second.push(first.top());
                    first.pop();
                }
                first.push(a);
            }
        }
        uint32_t calc_median(){
            if(first.size() > second.size()){
                return first.top();
            }
            else if(second.size() > first.size()){
                return second.top();
            }
            else{
                return ((first.top() + second.top()) / 2);
            }
        }
        //returns number of fights that happen after that deployment
        uint32_t check_fight(uint32_t timeStamp){
            Deployment jedi = jediPQ.top();
            Deployment sith = sithPQ.top();
            uint32_t num_battles = 0;
            while(jedi.force <= sith.force){//condition == true>>> fight happens
                num_battles++;
                uint32_t death = min(jedi.troops,sith.troops);
                jedi.troops -= death;
                sith.troops -= death;
                generals[jedi.gen].death_count += death;
                generals[sith.gen].death_count += death;
                if(jedi.troops == 0){
                    jediPQ.pop();
                }
                if(sith.troops == 0){
                    sithPQ.pop();
                }
                Deployment jedi = jediPQ.top();
                Deployment sith = sithPQ.top();
                num_battles++;
                if(medianMode){
                    add_to_median(death);
                }
                if(verboseMode){
                    cout << "General " << sith.gen << "'s battalion attacked " << jedi.gen << "'s battalion on planet " << num << ". " << 2 * death << "troops were lost \n";
                }
                if(movieWatcher){
                    update_movie(timeStamp);
                }
            }
            return num_battles;
        }//check_fights
        
        void add_deployment(const Deployment &deployment, const bool &evil){
            if(evil){
                sithPQ.push(deployment);
            }
            else{
                jediPQ.push(deployment);
            }
            recent = deployment;
            recentJedi = !evil;
        }
        //updates movie watcher mode after every fight
        void update_movie(const uint32_t timeStamp){
            if(ambush == State::Initial){
                if(!recentJedi){
                    ambush = State::Wait;//must remember this
                    movieData sit;
                    sit.force = recent.force;
                    sit.stamp = timeStamp;
                    ambushMov.jediMov = sit;
                }
            }
            else if(ambush == State::Wait){
                if(recentJedi){
                    movieData &ref = ambushMov.jediMov;
                    if(recent.force < ref.force){
                        movieData replace;
                        replace.force = recent.force;
                        replace.stamp = timeStamp;
                    }
                }
                movieData &ref = ambushMov.sithMov;
                if(recent.force > ref.force){
                    ref.force = recent.force;
                    ref.stamp = timeStamp;
                }
            }
            else if(ambush == State::Action){
                if(recentJedi){
                    movieData &jedRef = ambushMov.jediMov;
                    movieData &sitRef = ambushMov.sithMov;
                    movieData &mayRef = ambushMov.maybe;
                    if((sitRef.force - jedRef.force) < (mayRef.force - recent.force)){
                        sitRef.force = mayRef.force;
                        sitRef.stamp = mayRef.stamp;
                        jedRef.force = recent.force;
                        jedRef.stamp = timeStamp;
                    }
                    else if(recent.force < jedRef.force){
                        jedRef.force = recent.force;
                        jedRef.stamp = timeStamp;
                    }
                }
                movieData &ref = ambushMov.sithMov;
                if(recent.force > ref.force){
                    movieData maybe;
                    maybe.force = recent.force;
                    maybe.stamp = timeStamp;
                }
            }
            if(attack == State::Initial){
                if(recentJedi){
                    attack = State::Wait;
                    movieData jed;
                    jed.force = recent.force;
                    jed.stamp = timeStamp;
                    attackMov.jediMov = jed;
                }
            }
            else if(attack == State::Wait){
                movieData &ref = attackMov.jediMov;
                if(!recentJedi){
                    if(recent.force > ref.force){
                        movieData sit;
                        sit.force = recent.force;
                        sit.stamp = timeStamp;
                        ref = sit;
                    }
                }
                else if(recent.force < ref.force){
                    movieData jed;
                    jed.force = recent.force;
                    jed.stamp = timeStamp;
                    ref = jed;
                }
            }
            else if(attack == State::Action){
                movieData &ref = attackMov.sithMov;
                movieData &jediRef = attackMov.sithMov;
                movieData &mayRef = attackMov.maybe;
                if(!recentJedi){
                    if((mayRef.force - recent.force) > (ref.force - jediRef.force)){
                        ref.force = recent.force;
                        ref.stamp = timeStamp;
                        jediRef.stamp = timeStamp;
                        jediRef.force = mayRef.force;
                    }
                    else if(recent.force > ref.force){
                        movieData mov;
                        mov.force = recent.force;
                        mov.stamp = recent.time;
                        ref = mov;
                    }
                }
                else if(recent.force < ref.force){
                    mayRef.force = recent.force;
                    mayRef.stamp = timeStamp;
                }
            }
        }
        void print_movie(){
            cout << "A movie watcher would enjoy an ambush on planet " <<
            num << "with sith at time " << ambushMov.sithMov.stamp << "and jedi at time "
            << ambushMov.jediMov.stamp << "\n";
            cout << "A movie watcher would enjoy an attack on planet " <<
            num << "with sith at time " << attackMov.sithMov.stamp << "and jedi at time "
            << attackMov.jediMov.stamp << "\n";
        }
    };//Planet
    void run(){
        stringstream ss;
        istream &inputStream = input_mode == 'P' ? ss : cin;
        if(input_mode == 'P'){
            gen_random(ss);
        }
        cout << "Deploying troops...\n";
        uint32_t timeStamp;
        string role;
        char fill;
        uint32_t generalID;
        uint32_t planetID;
        int force;
        uint32_t troops;
        uint32_t tiebreaker = 0;
        uint32_t last_time = 0;//dangerous to assume this, fix it(does it start at 0)
        uint32_t time_deathcount = 0;
        try{
            while(inputStream >> timeStamp >> role >> fill >> generalID >> fill >>
                  planetID >> fill >> force >> fill >> troops){
                //error check first always
                if(generalID >= num_generals || generalID < 0){
                    throw "Invalid general ID";
                }
                if(planetID >= num_planets || planetID < 0){
                    throw "Invalid planet ID";
                }
                if(force < 0){
                    throw "Invalid force sensitivity";
                }
                if(troops < 0){
                    throw "Invalid troop number";
                }
                if(timeStamp < last_time){
                    throw "Invalid decreasing timestamp";
                }
                if(last_time > timeStamp && medianMode){
                    if(time_deathcount > 0){
                        cout << "median troops lost on planet" << planetID << "at "
                        << last_time << "is" << planets[planetID].calc_median() << "\n";
                    }
                }
                Deployment deployment;
                deployment.force = force;
                deployment.troops = troops;
                deployment.time = tiebreaker;
                deployment.gen = generalID;
                tiebreaker++;
                bool evil = (role == "SITH");
                planets[planetID].add_deployment(deployment,evil);//adds deployment
                if(evil){
                    generals[generalID].deployedSith += troops;
                }
                else{
                    generals[generalID].deployedJedi += troops;
                }
                uint32_t update = planets[planetID].check_fight(timeStamp);
                num_battles += update;
                time_deathcount += update;
                last_time = timeStamp;//update last_time
            }
        }
        catch(char const *e){
            cout << *e << "\n";
            exit(1);
        }
        cout <<"---End of Day---\n";
        cout <<"Battles: " << num_battles << "\n";
        if(generalMode){
            cout << "---General Evaluation---\n";
            print_gen();
        }
        if(movieWatcher){
            cout << "---Movie Watcher---\n";
            for(uint32_t i = 0; i < num_planets; i++){
                Planet planet = planets[i];
                planet.print_movie();
            }
        }
    }
    void gen_random(stringstream &s){
        string trash;
        uint32_t seed;
        uint32_t deployments;
        uint32_t rate;
        cin >> trash >> seed >> trash >> deployments >> trash >> rate;
        P2random::PR_init(s, seed, num_generals, num_planets, deployments, rate);
    }
    void set_planets(){
        Planet planet(generals);//this has to be slow??
        planets.resize(num_planets,planet);
    }
    void print_gen(){
        for(uint32_t i = 0; i < num_generals; i++){
            generals[i].print_general(i);
        }
    }
private:
    char input_mode;
    bool medianMode;
    bool generalMode;
    vector<General> generals;
    vector<Planet> planets;
    bool verboseMode;
    bool movieWatcher;
    uint32_t num_battles;
    uint32_t num_generals;
    uint32_t num_planets;
};
int main(int argc, char** argv){
    xcode_redirect(argc,argv);
    ios_base::sync_with_stdio(false);
    string trash;
    char input_mode;
    uint32_t num_generals;
    uint32_t num_planets;
    getline(cin,trash);//comment line, always exists
    cin >> trash >> input_mode >> trash;
    cin >> trash >> num_generals >> trash >> num_planets;
    Simulation simulation(input_mode,num_generals,num_planets);
    simulation.take_arguments(argc, argv);
    simulation.run();
}
