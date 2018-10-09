/**
 *  @dev minakokojima
 */

#include "escrow.hpp"

void escrow::onTransfer(account_name from, account_name to, extended_asset quantity, std::string memo) {        
    
    if (to != _self) return;
    
    require_auth(from);
    eosio_assert(quantity.is_valid(), "invalid token transfer");
    eosio_assert(quantity.amount > 0, "must transfer a positive amount");
    
    asset a = asset(quantity.symbol, quantity.amount / 2);
    asset b = asset(quantity.symbol, quantity.amount - quantity.amount / 2);

    if (a.amount > 0) {
        action(
            permission_level{_self, N(active)},
            quantity.countract, N(transfer),
            make_tuple(_self, N(minakokojima), a,
            std::string(""))
        ).send();
    }
    if (b.amount > 0) {
        action(
            permission_level{_self, N(active)},
            quantity.countract, N(transfer),
            make_tuple(_self, N(rukamoemoe51), b,
            std::string(""))
        ).send();
    }
}