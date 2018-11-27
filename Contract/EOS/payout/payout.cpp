#include "payout.hpp"

void payout::onTransfer(account_name from, account_name to, extended_asset quantity, std::string memo) {        

    if (to != _self) return;
    require_auth(from);
    eosio_assert(quantity.quantity.is_valid(), "invalid token transfer");
    eosio_assert(quantity.quantity.amount > 0, "must transfer a positive amount");

    auto params = split(memo, ' ');
    eosio_assert(params.size() >= 1, "error memo");    
    
    if (params[0] == "stake") {        
        eosio_assert(quantity.contract == TOKEN_CONTRACT, "must use true CTN to stake");
        eosio_assert(quantity.quantity.symbol == TOKEN_SYMBOL, "must use CTN to stake");
        stake(from, quantity.quantity.amount);
        auto g = _global.get();
        g.total_staked += quantity.quantity.amount;
        _global.set(g, _self);     
        return;
    }    

    if (params[0] == "make_profit") {
        eosio_assert(quantity.contract == N(eosio.token), "must use true EOS to make profit");
        eosio_assert(quantity.quantity.symbol == EOS_SYMBOL, "must use EOS to make profit");
        // make_profit(quantity.quantity.amount);
        return;
    }
}