/**
 *  @dev minakokojima, yukiexe
 *  @copyright Andoromeda
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>

//#include "config.hpp"
//#include "utils.hpp"
 
typedef double real_type;
using std::string ;
using namespace eosio ;

struct st_transfer {
    account_name from;
    account_name to;
    asset        quantity;
    string       memo;

    EOSLIB_SERIALIZE( st_transfer, (from)(to)(quantity)(memo) )
};

class hotpotato : public contract {
    public: hotpotato(account_name self) :
        contract(self),
        _global(_self, _self),
        _land(_self, _self),
        _player(_self, _self){}

    [[eosio::action]] void init();
    [[eosio::action]] void clear();     
    [[eosio::action]] void transfer(account_name   from,
                  account_name   to,
                  asset          quantity,
                  string         memo);
    
    void onTransfer(account_name from, account_name to,
                    extended_asset quantity, string& memo); 

    [[eosio::action]] void newland(account_name &from, asset &eos);

    void buy_land(account_name from, extended_asset in, const vector<string>& params);

    void apply(account_name code, action_name action);

    struct [[eosio::table]] land {
        uint64_t     id;
        account_name owner = 0;
        uint64_t price;           
        uint64_t parent;
        void tax() {
        }
        uint64_t next_price() const {
            return price * 1.35;
        }

        auto primary_key()const { return id; }
    };    
    
    struct [[eosio::table]] player {
        account_name  account;
        uint64_t land_profit;
        uint64_t ref_profit;
        uint64_t fee_profit;
        uint64_t pool_profit;
        uint64_t staked_income;
        uint64_t council_income;

        uint64_t primary_key() const {return account;}        
        void withdraw() {
        }
    };
        
    struct [[eosio::table]] st_global {       
        uint64_t team;
        uint64_t pool;
        account_name last;
        time st, ed;
    };

    typedef eosio::multi_index<N(land), land> land_t;
    land_t _land;   

    typedef eosio::multi_index<N(player), player> player_t;
    player_t _player;  

    typedef singleton<N(global), st_global> singleton_global;
    singleton_global _global;       
    
};

void hotpotato::apply(account_name code, action_name action) {   
    auto &thiscontract = *this;

    if (action == N(transfer)) {
        if (code == N(eosio.token)) {
            auto transfer_data = unpack_action_data<st_transfer>();
            onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, code), transfer_data.memo);
        }
        return;
    }

    if (code != _self) return;
    switch (action) {
        EOSIO_API(hotpotato, (init)(newland));
    };
}

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) 
    {
        hotpotato p(receiver);
        p.apply(code, action);
        eosio_exit(0);
    }
}
