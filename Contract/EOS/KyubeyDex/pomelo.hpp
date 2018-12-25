#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <cmath>

using namespace eosio;
using namespace std;

typedef uint32_t time;

const auto EOS_SYMBOL = eosio::symbol("EOS", 4);
const auto TOKEN_CONTRACT = "eosio.token"_n;

constexpr uint64_t PRICE_SCALE = 100000000;

static uint64_t my_string_to_symbol(const char* str) {
    uint32_t len = 0;
    while (str[len]) ++len;
    
    uint64_t result = 0;
    for (uint32_t i = 0; i < len; ++i) {
        // All characters must be upper case alaphabets
        //eosio_assert(str[i] >= 'A' && str[i] <= 'Z', "...invalid character in symbol name");
        result |= (uint64_t(str[i]) << (8 * (i + 1)));
    }
    return result >> 8;
}

CONTRACT pomelo : public eosio::contract {
public:
    using contract::contract;

    ACTION addfav(string str_symbol) {}
    ACTION removefav(string str_symbol) {}
    ACTION clean(string str_symbol);
    ACTION cancelsell(name account, string str_symbol, uint64_t id);
    ACTION cancelbuy(name account, string str_symbol, uint64_t id);
    ACTION setwhitelist(string str_symbol, name issuer);
    ACTION rmwhitelist(string str_symbol);
    ACTION login(string token) {}

    TABLE buyorder { 
        uint64_t id;
        capi_name account;
        asset bid;
        asset ask;
        uint64_t unit_price;
        time timestamp;

        auto primary_key() const { return id; }
        uint64_t get_price() const { return -unit_price; }
        EOSLIB_SERIALIZE(buyorder, (id)(account)(bid)(ask)(unit_price)(timestamp))
    };

    TABLE sellorder {
        uint64_t id;
        capi_name account;
        asset bid;
        asset ask;
        uint64_t unit_price;
        time timestamp;

        auto primary_key() const { return id; }
        uint64_t get_price() const { return unit_price; }
        EOSLIB_SERIALIZE(sellorder, (id)(account)(bid)(ask)(unit_price)(timestamp))
    };

    TABLE whitelist {
        capi_name contract;
    };

    typedef eosio::multi_index<"buyorder"_n, buyorder, 
        indexed_by<"byprice"_n, const_mem_fun<buyorder, uint64_t, &buyorder::get_price>>
    > buyorders_t;
    
    typedef eosio::multi_index<"sellorder"_n, sellorder, 
        indexed_by<"byprice"_n, const_mem_fun<sellorder, uint64_t, &sellorder::get_price>>
    > sellorders_t;
 
    typedef singleton<"whitelist"_n, whitelist> whitelist_index_t;

    struct match_record {
        uint64_t id;
        capi_name bidder;
        capi_name asker;
        asset bid;
        asset ask;
        uint64_t unit_price;
        time timestamp;
    };

    // rec
    ACTION buyreceipt(buyorder o) {
        require_auth(_self);
    }    

    ACTION sellreceipt(sellorder t) {
        require_auth(_self);
    }   

    ACTION buymatch(match_record t) {
        require_auth(_self);
    }

    ACTION sellmatch(match_record t) {
        require_auth(_self);
    }

    void onTransfer( name from, name to, asset bid, string memo );
    void transfer(name from, name to, asset quantity, string memo) {}

private:
    vector<string> split(string src, char c);
    uint64_t string_to_amount(string s);

    uint64_t my_string_to_symbol(uint8_t precision, const char* str);
    name get_contract_name_by_symbol(symbol sym);
    inline name get_contract_name_by_symbol(string str_symbol) {
        return get_contract_name_by_symbol( symbol(str_symbol,4) );
    }

    void assert_whitelist(symbol sym, name contract);
    void assert_whitelist(string str_symbol, name contract);
    void publish_buyorder_if_needed(name account, asset bid, asset ask);
    void publish_sellorder_if_needed(name account, asset bid, asset ask);
    void buy(name account, asset bid, asset ask);
    void sell(name account, asset bid, asset ask);

    inline bool is_valid_unit_price(uint64_t eos, uint64_t non_eos) {
        return eos * PRICE_SCALE % non_eos == 0;
    }

public:
    void apply(uint64_t receiver, uint64_t code, uint64_t action);    
};

struct st_transfer {
    name from;
    name to;
    asset        quantity;
    string       memo;

    EOSLIB_SERIALIZE( st_transfer, (from)(to)(quantity)(memo) )
};

void pomelo::apply(uint64_t receiver, uint64_t code, uint64_t action) {
    auto &thiscontract = *this;
    if (action == ( "transfer"_n ).value ) {
        auto transfer_data = unpack_action_data<st_transfer>();
        if (transfer_data.quantity.symbol == EOS_SYMBOL)
            eosio_assert(name(code) == TOKEN_CONTRACT, "Transfer EOS must go through eosio.token...");
        else
            assert_whitelist(transfer_data.quantity.symbol, name(code));
        
        onTransfer(transfer_data.from, transfer_data.to, transfer_data.quantity, transfer_data.memo);
        return;
    }

    if (code != get_self().value) return;

    switch (action) {
        EOSIO_DISPATCH_HELPER(pomelo,
            (clean)
            (cancelbuy)
            (cancelsell)
            (setwhitelist)
            (rmwhitelist)
            (login)
            (addfav)
            (removefav)
        )
    }
}

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        pomelo p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(receiver, code, action);
        eosio_exit(0);
    }
}
