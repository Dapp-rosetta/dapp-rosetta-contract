#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>

using namespace eosio;
using namespace std;

namespace kyubey {

typedef uint32_t time;

constexpr auto EOS_SYMBOL = eosio::symbol("EOS", 4);
constexpr auto EOS_CONTRACT = "eosio.token"_n;
constexpr auto TOKEN_CONTRACT = "eosio.token"_n;

constexpr uint64_t PRICE_SCALE = 100000000;

CONTRACT kyubeydex : public eosio::contract {
public:
    using contract::contract;

    ACTION clean( const string str_symbol);
    ACTION cancelbuy( const name executor, string &str_symbol, const uint64_t id) {
        require_auth(executor);
        cancelorder<buyorders_t>(executor, symbol_code{str_symbol}, id);
    }
 
    ACTION cancelsell( const name executor, string &str_symbol, const uint64_t id) {
        require_auth(executor);
        cancelorder<sellorders_t>(executor, symbol_code{str_symbol}, id);
    }

    // whitelist management
    ACTION setwhitelist( name issuer, asset token );
    ACTION rmwhitelist( asset token );

    ACTION login(string token) {}
    ACTION addfav( string str_symbol) {}
    ACTION removefav( string str_symbol) {}

    struct st_order {
        uint64_t id;
        capi_name account;
        asset bid;
        asset ask;
        uint64_t unit_price;
        time timestamp;
        
        auto primary_key() const { return id; }
    };

    TABLE buyorder : st_order {
        uint64_t get_price() const { return -unit_price; }
        EOSLIB_SERIALIZE(buyorder, (id)(account)(bid)(ask)(unit_price)(timestamp))
    };

    TABLE sellorder : st_order {
        uint64_t get_price() const { return unit_price; }
        EOSLIB_SERIALIZE(sellorder, (id)(account)(bid)(ask)(unit_price)(timestamp))
    };

    TABLE whitelist {
        capi_name contract;
    };

    typedef eosio::multi_index< "buyorder"_n, buyorder, 
                                indexed_by<"byprice"_n, const_mem_fun<buyorder, uint64_t, &buyorder::get_price> > 
            > buyorders_t;
    typedef eosio::multi_index< "sellorder"_n, sellorder, 
                                indexed_by<"byprice"_n, const_mem_fun<sellorder, uint64_t, &sellorder::get_price> > 
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
    ACTION buyreceipt( const buyorder &buy_order) const {
        require_auth(_self);
    }    

    ACTION sellreceipt( const sellorder &sell_order) const {
        require_auth(_self);
    }   

    ACTION buymatch( const match_record &record) const {
        require_auth(_self);
    }

    ACTION sellmatch( const match_record &record) const {
        require_auth(_self);
    }

    void transfer(name from, name to, asset quantity, string memo) {}

private:
    vector<string> split( const string &src, const char c ) const ;
    uint64_t string_to_amount( const string &s ) const ;
    asset string_to_asset( const string &s1, const string &s2 ) const ;
    void onTransfer( name from, name to, asset bid, string memo );

    name get_contract_name_by_symbol (symbol sym) const ;
    inline name get_contract_name_by_symbol( const string str_symbol) const {
        return get_contract_name_by_symbol( symbol(str_symbol,4) );
    }

/**
 * @brief Check if the currency corresponds to the correct contract
 * 
 * @param sym - currency
 * @param contract - contract
 **/
    template <typename T>
    inline void assert_whitelist(T &sym, name contract) {
        auto account = get_contract_name_by_symbol(sym);
        eosio_assert(account == contract, "Transfer code does not match the contract in whitelist.");
    }
    inline void action_transfer_token(const name &to, const asset &quantity,
                                              const string memo = string("transfer") );

    void buy(name account, asset bid, asset ask);
    void sell(name account, asset bid, asset ask);
    void match_processing(const match_record &rec);
    template <typename T>
    void publish_order(name account, asset bid, asset ask);
    template <typename T>
    void cancelorder( const name &executor, const symbol_code &code, const uint64_t &id);

    void market_price_trade(name account, asset bid, asset ask);

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

void kyubeydex::apply(uint64_t receiver, uint64_t code, uint64_t action) {
    auto &thiscontract = *this;
    if (action == ( "transfer"_n ).value ) {
        auto transfer_data = unpack_action_data<st_transfer>();
        if (transfer_data.quantity.symbol == EOS_SYMBOL)
            eosio_assert(name(code) == TOKEN_CONTRACT, "Transfer EOS must go through eosio.token...");
        else
            assert_whitelist<symbol>(transfer_data.quantity.symbol, name(code));
        
        onTransfer(transfer_data.from, transfer_data.to, transfer_data.quantity, transfer_data.memo);
        return;
    }

    if (code != get_self().value) return;

    switch (action) {
        EOSIO_DISPATCH_HELPER(kyubeydex,
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
        kyubeydex p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(receiver, code, action);
        eosio_exit(0);
    }
}

} /// namespace kyubey

/*
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

static uint64_t my_string_to_symbol(uint8_t precision, const char* str) {
    uint32_t len = 0;
    while (str[len]) ++len;

    uint64_t result = 0;
    for (uint32_t i = 0; i < len; ++i) {
        result |= (uint64_t(str[i]) << (8 * (i + 1)));
    }
    result |= uint64_t(precision);
    return result >> 8;
}
*/