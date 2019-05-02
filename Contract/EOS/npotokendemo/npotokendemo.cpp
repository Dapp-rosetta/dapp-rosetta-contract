/**
 *  @dev minakokojima
 */

#include "npotokendemo.hpp"

/**
* 取钱，NPO 从存款池中取钱，并减少单位 NPO token 的价值
*
* @param from - 取款人
* @param out - 取款金额 
*/    
void npotokendemo::take_out(name from, extended_asset out) {        
    if (from != _self) return;
    auto itr = connector.get();
    itr.balance -= out.quantity;

    action(
        permission_level{_self, "active"_n},
        out.contract, name("transfer"),
        make_tuple(_self, from, out.quantity,
        string("take out some eos"))
    ).send();
}
    
/**
* 存錢，并印制对应金额的 NPO token 给存款人
*
* @param from - 存款人
* @param in - 存款金额
*/    
void npotokendemo::put_in(name from, extended_asset in) {        
    auto itr = connector.get();
    auto delta = asset(itr.supply * in.quantity.amount / itr.balance, itr.supply.symbol);
    itr.balance += in.quantity;
    itr.supply += delta;
    connector.set(itr, _self);    
    action(
        permission_level{_self, "active"_n},
        _self, name("transfer"),
        make_tuple(_self, from, delta,
        string("issue new token"))
    ).send();
}

void npotokendemo::onTransfer(name from, name to, extended_asset in, string memo) {
    if (to != _self) return;        
    require_auth(from);
    eosio_assert(in.quantity.is_valid(), "invalid token transfer");
    eosio_assert(in.quantity.amount > 0, "must transfer a positive amount");
//        eosio_assert(in.quantity.symbol == "EOS 4"_n, "only allow eos");
    eosio_assert(in.contract == "eosio.token"_n, "only allow eos");
    put_in(from, in); 
}