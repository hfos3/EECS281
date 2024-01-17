// Project Identifier: C0F4DFE8B340D81183C208F70F9D2D797908754D

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
#include "TableEntry.h"

using namespace std;
enum class tab_choice:uint8_t {first,second};
static option long_options[] = {
  // LONG        ARGUMENT USED?     (ignore) RETURN VALUE
  {"help",      no_argument, nullptr, 'h'},
  {"quiet",      no_argument, nullptr, 'q'},
  // this one tells getopt_long that there are no more options:
  {nullptr,      0,                 nullptr,  0}
};

class Database{
    class Table{
        private:
        bool comp(const TableEntry &a, const TableEntry &b, const char &op)const{
            switch (op){
                case '<':
                    if(a < b){
                        return true;
                    }
                    return a < b;
                    break;
                case '=':
                    return a == b;
                    break;
                case '>':
                    return a > b;
                    break;
            }
            return false;//never gets here
        }
        void print_row(const size_t &idx, const vector<string> &names)const{//prints row given vec and newline
            for(auto i : names){
                size_t col = find_idx(i);
                cout << data[idx][col] << " ";
            }
            cout << "\n";
        }
        string name;
        vector<vector<TableEntry> > data;
        unordered_map<string,size_t> column_idx;
        vector<EntryType> columns;
        unordered_map<TableEntry, vector<size_t> >  idxH;
        map<TableEntry, vector<size_t> > tempidx;
        map<TableEntry, vector<size_t> > idxM;
        size_t index_of_gen;
        public:
        size_t find_idx(const string &s)const{
            auto it = column_idx.find(s);
            if(it == column_idx.end()){
                return num_columns() + 1;
            }
            return column_idx.find(s)->second;
        }
        EntryType find_type(const string &s)const{return columns[find_idx(s)];}
        map<TableEntry,vector <size_t>>::iterator find_temp(TableEntry &entry){return tempidx.find(entry);}
        map<TableEntry,vector<size_t>>::iterator end_temp(){return tempidx.end();}
        TableEntry getval(const size_t &a, const size_t &b)const{return data[a][b];}
        const TableEntry getEntry(EntryType type,const string& val)const{//return entry based on type
            switch(type){
                case EntryType::String:
                    {TableEntry entry(val);
                    return entry;};
                    break;
                case EntryType::Double:{
                    double temp = atof(val.c_str());
                    return TableEntry(temp);
                }
                    break;
                case EntryType::Int:{
                    int temp = stoi(val);
                    return TableEntry(temp);
                }
                    break;
                case EntryType::Bool:
                    if(val[0] == 't'){
                        return TableEntry(true);
                    }
                    return TableEntry(false);
                    break;
            }
            return TableEntry(val);//should NEVER get here
        }
        Table(const string &name_in, const vector<EntryType> &info, const unordered_map<string,size_t> &other)
        : name(name_in), column_idx(other),columns(info){
            index_of_gen = columns.size() + 1;
        }
        size_t num_columns()const{ return columns.size();}//num columns
        size_t size(){ return data.size();}
        void gen_bin(const size_t &idx,const bool &temp){
            if(!temp){idxM.clear(); idxH.clear(); index_of_gen = idx;}
            tempidx.clear();//cleared in between joins
            auto &treeRef = temp ? tempidx : idxM;
            for(size_t i = 0, n = data.size(); i < n; i++){
                auto it = treeRef.find(data[i][idx]);
                if(it == idxM.end() || it == tempidx.end()){
                    vector<size_t> ins;
                    ins.push_back(i);
                    treeRef.emplace(TableEntry(data[i][idx]), ins);
                }
                else{
                    it->second.push_back(i);
                }
            }
        }
        void gen_hash(const string &colname){
            size_t idx = find_idx(colname);
            idxH.clear();
            idxM.clear();
            index_of_gen = num_columns() + 1;//i dont use hash anywhere else, so should not save as idx
            for(size_t i = 0, n = data.size(); i < n; i++){
                auto it = idxH.find(data[i][idx]);
                if(it == idxH.end()){
                    vector<size_t> idxs;
                    idxs.push_back(i);
                    idxH.emplace((TableEntry(data[i][idx])), idxs);
                }
                else{
                    it->second.push_back(i);
                }
            }

        }
        void del(const string &col_name, const string &val, const char &op){//delete is keyword
            size_t prev = data.size();
            size_t idx = find_idx(col_name);
            EntryType type = columns[idx];
            TableEntry other = getEntry(type,val);
            auto it = remove_if(data.begin(),data.end(),
            [this,other,op,idx](const vector<TableEntry> &row){
                return comp(row[idx],other,op);
            });
            auto sum = [this](const size_t &n){cout << "Deleted " << n << " rows from " << name << "\n";};
            if(it == data.end()){
                sum(0);
                return;
            }
            data.erase(it,data.end());
            //update index if one exists
            if(index_of_gen < num_columns()){
                gen_bin(index_of_gen,false);
            }
            sum(prev-data.size());
        }
        void print_columns(const vector<string> &names)const{
            for(auto s:names){
                cout << s << " ";
            }
        }
        void print_sum(const size_t &count)const{
            cout << "Printed " << count << " matching rows from " << name << "\n";
        }
        //if there is an index on that column
        void print_with_gen(const vector<string> &print_col,const char &op,const TableEntry &entry,const bool &quiet)const{
            auto first = idxM.begin();
            auto end = idxM.end();
            auto row_print = [&](const vector<size_t> &in){
            for(size_t n : in){print_row(n,print_col);}};

            auto partition = lower_bound(idxM.begin(),idxM.end(),entry,[&]
            (const pair<TableEntry,vector<size_t> > &in,const TableEntry &bench){ 
                return comp(in.first,bench,'<');});
            switch(op){
                case '<':
                    end = partition;
                    break;
                case '>':
                    first = partition;
                    if(first != idxM.end()){
                        if(first->first == entry){first++;}
                    }
                    break;
                case '=':
                    if(partition == idxM.end()){ print_sum(0); return;}
                    if(partition->first == entry){
                        if(!quiet){
                            row_print(partition->second);
                        }
                        print_sum(partition->second.size());
                        return;
                    }
                    print_sum(0);
                    return;
            }
            if(first == idxM.end()){
                print_sum(0);
                return;
            }
            size_t num = 0;
            for_each(first,end,[&](pair<TableEntry,vector<size_t> > in){
                if(!quiet){
                    row_print(in.second);
                }
                num += in.second.size();
            });
            print_sum(num);
        }
        //false if there is error
        bool check_column(const string &s,const string &cmd)const{
            if(find_idx(s) == num_columns() + 1){
                cout << "Error during " << cmd << ": ";
                cout <<  (s + string(" does not name a column in ") + name) << "\n";
                return false;
            }
            return true;
        }
        //print where:
        void print(const vector<string> &s,const char &op,const string &val,const string &colname,const bool &quiet)const{
            if(!quiet){
                print_columns(s);
                cout << "\n";
            }
            if(data.size() == 0){
                print_sum(0);
                return;//no data, so no matching rows
            }
            const TableEntry &ref = getEntry(find_type(colname),val);
            size_t col_idx = find_idx(colname);
            if(col_idx == index_of_gen){
                print_with_gen(s,op,ref,quiet);
                return;
            }
            size_t counter = 0;
            for(size_t j = 0, k = data.size(); j < k; j++){
                if(comp(data[j][col_idx],ref,op)){
                    counter++;
                    if(!quiet){
                        print_row(j,s);
                    }
                }
            }
            print_sum(counter);
        }
        void print_all(const vector<string> &s, const bool &quiet)const{
          if(!quiet){
            print_columns(s);
            cout << "\n";
          }
          if(!quiet){
            for(size_t j = 0, n = data.size(); j < n; j++){
                for(size_t i = 0, k = s.size(); i < k; i++){
                    cout << data[j][find_idx(s[i])] << " ";
                }
                cout << "\n";
            }
          }
          cout << "Printed " << data.size() << " matching rows from " << name << "\n";
        }

        void insert(uint32_t new_rows){
            size_t start = data.size();
            cout << "Added " << new_rows << " rows to " << name << " from position " << start 
            << " to " << (new_rows + start - 1) << "\n";
            for(uint32_t i = 0; i < new_rows; i++){
                vector<TableEntry> row;
                row.reserve(num_columns());
                for(uint32_t j = 0; j < num_columns(); j++){
                    string temp;
                    cin >> temp;
                    TableEntry entry = getEntry(columns[j],temp);
                    if(index_of_gen == j){
                        auto it = idxM.find(entry);
                        if(it == idxM.end()){
                            vector<size_t> ins;
                            ins.push_back(i + start);
                            idxM.emplace(entry, ins);
                        }
                        else{
                            it->second.push_back(i + start);
                        }
                    }
                    row.push_back(entry);
                }
                data.push_back(row);
            }
        }
    };
    private:
    bool quiet_mode;
    //bool help_mode;

    void table_error(const string &tablename, const string &cmd){
        cout << "Error during " << cmd << ": ";
        string message = string(" does not name a table in the database");
        cout << (tablename + message.c_str()) << "\n";
    }
    unordered_map<string,Table> tables;
    public:
    Database(const bool &q):quiet_mode(q){}
    void read_in(){
        string cmd;
            do{
                if (cin.fail()) {
                    cerr << "Error: Reading from cin has failed" << endl;
                    exit(1);
                } // if
                cout << "% ";
                cin >> cmd;
                string tablename;
                cin >> tablename;
                switch(cmd[0]){
                    case '#':
                        getline(cin,cmd);
                        break;
                    case 'C':
                        process_create(tablename);
                        break;
                    case 'R':
                        process_remove(tablename);
                        break;
                    case 'I':
                        cin >> tablename;
                        process_insert(tablename);
                        break;
                    case 'D':
                        cin >> tablename;
                        process_delete(tablename);        
                        break;                
                    case 'P':
                        cin >> tablename;
                        process_print(tablename);
                        break;
                    case 'J':{
                        string name2;
                        cin >> name2 >> name2;
                        process_join(tablename,name2);
                    }
                        break;
                    case 'G':
                        cin >> tablename;
                        process_generate(tablename);
                        break;
                    case 'Q':break;
                    default:
                        cout << "Error: unrecognized command\n";
                        {string temp; getline(cin,temp);}
                        break;
                }
            } while(cmd != "QUIT");
        cout << "Thanks for being silly!\n";
    }
    void process_generate(const string &tablename){
        const auto &tab = tables.find(tablename);
        string colname;
        if(tab == tables.end()){
            table_error(tablename,"GENERATE");
            getline(cin,colname);
            return;
        }
        char option;
        cin >> option;
        cin >> colname >> colname >> colname >> colname;
        if(!tab->second.check_column(colname,"GENERATE")){getline(cin,colname); return;}
        cout << "Created ";
        switch(option){
            case 'h':
                tab->second.gen_hash(colname);
                cout << "hash ";
                break;
            case 'b':
                //false is to have extra index for other commands
                auto &tab = tables.find(tablename)->second;
                tab.gen_bin(tab.find_idx(colname),false);
                cout << "bst ";
                break;
        }
        cout << "index for table " << tablename << " on column " << colname << "\n";
    }
    void print_for_join(const vector<size_t> &rows, const vector<pair<string,tab_choice>> &info, 
    const Table &table1, const Table &table2, size_t tab1_row){
        for(size_t i : rows){//rows are only the rows for 2
            for(auto select : info){
                switch(select.second){
                    case tab_choice::first:
                        cout << table1.getval(tab1_row,table1.find_idx(select.first)) << " ";
                        break;
                    case tab_choice::second:
                        cout << table2.getval(i,table2.find_idx(select.first)) << " ";
                        break;
                }
            }
            cout << "\n";
        }
    }
    //only command that has most of execution outside of table
    //needs to use two seperate tables
    void process_join(const string &name1, const string &name2){
        string fill;
        cin >> fill;
        string col1;
        string col2;
        cin >> col1 >> col2 >> col2;
        cin >> fill >> fill;
        size_t cols;
        cin >> cols;

        auto table1it = tables.find(name1);
        if(table1it == tables.end()){
            table_error(name1,"JOIN");
            getline(cin,fill);
            return;
        }
        auto table2it = tables.find(name2);
        if(table2it == tables.end()){
            table_error(name2,"JOIN");
            getline(cin,fill);
            return;
        }
        if(!table1it->second.check_column(col1,"JOIN")){getline(cin,fill); return;}
        if(!table2it->second.check_column(col2,"JOIN")){getline(cin,fill); return;}
        vector<pair<string,tab_choice> > print_info;//saves name and 1st/2nd table
        for(size_t i = 0; i < cols; i++){//fills up colnames
            string name;
            size_t opt;
            cin >> name >> opt;
            if(opt == 1){
                if(!table1it->second.check_column(name,"JOIN")){getline(cin,fill); return;}
                print_info.emplace_back(name,tab_choice::first);
            }
            else{
                if(!table2it->second.check_column(name,"JOIN")){getline(cin,fill); return;}
                print_info.emplace_back(name,tab_choice::second);
            }
        }
        //prints col names
        if(!quiet_mode){for_each(print_info.begin(),print_info.end(),[](pair<string,tab_choice> in){
            cout << in.first << " ";
        }); cout << "\n";}

        size_t counter = 0;//for # matching rows
        size_t idx1 = table1it->second.find_idx(col1);
        size_t idx2 = table2it->second.find_idx(col2);
        table2it->second.gen_bin(idx2,true);//creates index for table 2
        for(size_t i = 0, n = table1it->second.size(); i < n; i++){//for each row
            TableEntry entry = table1it->second.getval(i,idx1);//gets val from current row
            auto match = table2it->second.find_temp(entry);//checks for match in correct col of tab2
            if(match != table2it->second.end_temp()){//prints out from vec of print_info
                counter += match->second.size();
                if(!quiet_mode){
                    print_for_join(match->second,print_info,table1it->second,table2it->second, i);
                }
            }
        }
        cout << "Printed " << counter << " rows from joining " << name1 << " to " << name2 << "\n";
    }
    void process_delete(const string &tablename){
        string fill;
        cin >> fill;
        string column;
        string val;
        char op;
        cin >> column >> op >> val;
        const auto &tab = tables.find(tablename);
        if(tab == tables.end()){
            table_error(tablename,"DELETE");
            getline(cin,column);
            return;
        }
        if(!tab->second.check_column(column,"DELETE")){
            return;
        }
        tab->second.del(column,val,op);
    }
    void process_print(const string &tablename){
        const auto &tab = tables.find(tablename);
        string fill;
        if(tab == tables.end()){
            table_error(tablename,"PRINT");
            getline(cin,fill);
            return;
        }
        uint32_t cols;
        cin >> cols;
        vector<string> names;
        names.reserve(cols);
        for(uint32_t i = 0; i < cols; i++){
            cin >> fill;
            if(!tab->second.check_column(fill,"PRINT")){
                getline(cin,fill);
                return;
            }
            names.push_back(fill);
        }
        string condition;
        cin >> condition;
        if(condition == "ALL"){
            tables.find(tablename)->second.print_all(names,quiet_mode);
        }
        else{
            string colname;
            string val;
            char op;
            cin >> colname >> op >> val;
            if(!tab->second.check_column(colname,"PRINT")){
                return;
            }
            tables.find(tablename)->second.print(names,op,val,colname,quiet_mode);
        }
    }
    void process_insert(const string &tablename){
        auto it = tables.find(tablename);
        string fill;
        if(it == tables.end()){
            table_error(tablename,"INSERT");
            getline(cin,fill);
            return;
        }
        uint32_t new_rows;
        cin >> new_rows;
        cin >> fill;//no longer have access to tablename, need for filler
        it->second.insert(new_rows);
    }
    void process_remove(const string &tablename){
        auto it = tables.find(tablename);
        if(it == tables.end()){
            table_error(tablename,"REMOVE");
            return;
        }
        tables.erase(tablename);
        cout << "Table " << tablename << " deleted\n";
    }
    void process_create(const string &tablename){
        size_t columns;
        auto it = tables.find(tablename);
        if(it != tables.end()){
            cout << "Error during CREATE: ";
            cout << "Cannot create already existing table " << tablename << "\n";
            string trash; getline(cin,trash);
            return;
        }
        cin >> columns;
        vector<EntryType> types;
        unordered_map<string,size_t> names;
        types.reserve(columns);
        for(uint32_t i = 0; i < columns; i++){
            string type;
            cin >> type;
            switch(type[0]){
                case 's':
                    types.push_back(EntryType::String);
                    break;
                case 'd':
                    types.push_back(EntryType::Double);
                    break;
                case 'i':
                    types.push_back(EntryType::Int);
                    break;
                case 'b':
                    types.push_back(EntryType::Bool);
                    break;
            }
        }
        cout << "New table " << tablename << " with column(s) ";
        for(size_t i = 0; i < columns; i++){
            string name;
            cin >> name;
            names[name] = i;
            cout << name << " ";
        }
        cout << "created\n";
        Table table(tablename,types,names);
        tables.insert({tablename,table});
    }
};

int main(int argc, char** argv){
    ios_base::sync_with_stdio(false);
    cin >> std::boolalpha;
    cout << std::boolalpha;
    bool quiet = false;
    int choice = -1;
    while((choice = getopt_long(argc, argv, "hq", long_options, nullptr)) != -1){
        switch(choice){
            case 'h':
                cout << "THIS IS HELP MODE\n SPEC HAS INSTRUCTIONS";
                exit(0);
            case 'q':
                quiet = true;
                break;
        }
    }
    Database database(quiet);
    database.read_in();
}