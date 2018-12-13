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

    ACTION init(name self) {
        eosio_assert(self == _self, "only contract itself."); 	       
        // todos.emplace(self, [&](auto& new_todo) {
        //     new_todo.id = 0;
        //     new_todo.counter = 1;
        // });
        todos.set( todo{ .id = 0, .counter = 1 } , get_self() );   
    }

    ACTION add(name author) {
        // auto itr = todos.find(0);
        auto itr = todos.get();
        // eosio_assert(itr != todos.end(), "No found");
        // todos.modify(itr, author, [&](auto& new_todo) {
        //   new_todo.id = 0;
        //   new_todo.counter += 1;
        // });
        itr.counter += 1;
        todos.set( itr, _self) ;

        eosio::print("Now counter is ", itr.counter);
    }

    TABLE todo {
        uint64_t id;
        uint64_t counter;
        uint64_t primary_key() const { return id; }
        uint64_t getCounter() const { return counter; }
    };

    // typedef eosio::multi_index<"todos"_n, todo> todo_table;
    typedef singleton<"todo"_n, todo> todo_table;
    todo_table todos;
};

EOSIO_DISPATCH( counter_contract, (add)(init) )

