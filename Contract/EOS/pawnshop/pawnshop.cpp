#include "pawnshop.hpp"

void pawnshop::onTransfer(name from, name to, asset quantity, string memo) {
    if (to != _self) return;
    require_auth(from);

    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "must buy a positive amount");
    
    /*auto params = split(memo, ' ');
    eosio_assert(params.size() >= 1, "Error params");

    if (params[0] == "buy_land") {
        buy_land(from, quantity, params);
        return;
    }*/
}