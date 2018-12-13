#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
using namespace eosio;

class counter_contract : public eosio::contract {
    public:
    counter_contract(account_name self):eosio::contract(self),todos(_self, _self) {
    }
    using eosio::contract::contract;

    void init(account_name self) {
        eosio_assert(self == _self, "only contract itself."); 	          
        todos.emplace(self, [&](auto& new_todo) {
            new_todo.id = 0;
            new_todo.counter = 1;
        });
    }

    void add(account_name author) {
        auto itr = todos.find(0);
        eosio_assert(itr != todos.end(), "No found");
        todos.modify(itr, author, [&](auto& new_todo) {
            new_todo.id = 0;
            new_todo.counter += 1;
        });
        eosio::print("Now counter is ", itr->counter);
    }

    private:
  // @abi table todos i64
    struct todo {
        uint64_t id;
        uint64_t counter;
        uint64_t primary_key() const { return id; }
        uint64_t getCounter() const { return counter; }
        EOSLIB_SERIALIZE(todo, (id)(counter))
    };

    typedef eosio::multi_index<N(todos), todo> todo_table;
    todo_table todos;
};

EOSIO_ABI( counter_contract, (init)(add) )
