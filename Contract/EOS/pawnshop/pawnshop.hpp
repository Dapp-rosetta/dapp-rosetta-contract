/**
 *  @dev minakokojima, yukiexe
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>
using namespace eosio;
using namespace std;

struct st_transfer {
    name   from;
    name   to;
    asset  quantity;
    string memo;
};


CONTRACT pawnshop : public eosio::contract {
public:
    pawnshop(name receiver, name code, datastream<const char*> ds): 
        contract(receiver, code, ds),
        _plan(receiver, receiver.value),
        _order(receiver, receiver.value) {
    }

    ACTION init();
    ACTION transfer(name from, name to, asset quantity, string memo);
    void onTransfer(name from, name to, asset in, string memo);

    TABLE plan {
        uint64_t id;
        name contract;
        name symbol;
        uint64_t current_price;
        uint64_t lend_rate;
        auto primary_key() const {return id;}
    };    

    TABLE order {
        uint64_t id;
        uint64_t plan_id;
        uint64_t amount;     
        auto primary_key() const {return id;}        
    };

    typedef multi_index<"plans"_n, plan> plan_table;
    typedef multi_index<"orders"_n, order> order_table;
    plan_table _plan;
    order_table _order;    


    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        auto &thiscontract = *this;
        if (action == name("transfer").value) {
            auto transfer_data = unpack_action_data<st_transfer>();
            onTransfer(transfer_data.from, transfer_data.to, transfer_data.quantity, transfer_data.memo);
            return;
        }
    }
};

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        pawnshop p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(receiver, code, action);
        eosio_exit(0);
    }
}

