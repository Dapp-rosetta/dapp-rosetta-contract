#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
using namespace eosio;

CONTRACT counter : public contract {
public:
    counter(name receiver, name code, datastream<const char*> ds)
        :contract(receiver, code, ds), todos(receiver, receiver.value) {}

    ACTION init() {
        require_auth(_self);
        todos.set(todo{}, _self);   
    }

    ACTION add(name author) {
        auto itr = todos.get();
        itr.counter += 1;
        todos.set(itr, _self);
        print("Now counter is ", itr.counter);
    }

    TABLE todo {
        uint64_t counter;
    };

    typedef singleton<"todo"_n, todo> todo_table;
    todo_table todos;
};

EOSIO_DISPATCH( counter, (init)(add) )

