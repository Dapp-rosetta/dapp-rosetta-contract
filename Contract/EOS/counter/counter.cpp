#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/singleton.hpp>
using namespace eosio;
using namespace std;

CONTRACT counter_contract : public eosio::contract {
public:
    counter_contract(name receiver, name code, datastream<const char*> ds): 
        contract(receiver, code, ds),
        todos(receiver, receiver.value) {
    }

    ACTION init() {
        require_auth(_self);
        todos.set(todo{}, _self);   
    }
    
    ACTION add(name author) {
        auto itr = todos.get();
        itr.counter += 1;
        todos.set(itr, _self);
        eosio::print("Now counter is ", itr.counter);
    }

    TABLE todo {
        uint64_t id;
        uint64_t counter;
        uint64_t primary_key() const { return id; }
        uint64_t getCounter() const { return counter; }
    };

    typedef singleton<"todo"_n, todo> todo_table;
    todo_table todos;
};

EOSIO_DISPATCH( counter_contract, (add)(init) )

