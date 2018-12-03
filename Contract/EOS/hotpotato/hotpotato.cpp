/**
 *  @dev minakokojima, yukiexe
 *  @copyright Andoromeda
 */

#include "hotpotato.hpp"

void hotpotato::init() {
    require_auth(_self);    

    while (_land.begin() != _land.end()) {
        _land.erase(_land.begin());
    }

    auto g = _global.get_or_create( _self, global{});    
    g.st = 1542884400;
    g.ed = 1542884400 + 12*60*60;
    g.pool = 0;
    _global.set(g, _self);    
}  

// create new card for sell
void hotpotato::newland(account_name &from, asset &eos) {
    require_auth(_self);
    // need check newland can be called only once
    for (int i=0;i<60;++i) {
        _land.emplace(_self, [&](auto &p) {
            p.id = _land.available_primary_key();
            p.owner = from;
            p.price = eos.amount;
        });
    }
}

// buy card
void hotpotato::buy_land(account_name from, extended_asset in, const vector<string>& params) {
    auto g = _global.get();
    eosio_assert(now() >= g.st, "game not start");
    eosio_assert(now() <= g.ed, "game ended");

    eosio_assert(in.contract == N(eosio.token), "only true EOS token is allowed");
    eosio_assert(in.symbol == EOS_SYMBOL, "only EOS token is allowed");
   
    eosio_assert(params.size() >= 2, "No ID found.");
    auto id = string_to_price(params[1].c_str());

    auto itr = _land.find(id);
    eosio_assert(itr != _land.end(), "no land exist");
    eosio_assert(in.amount >= itr->next_price(), "no enough eos");
    eosio_assert(from != itr->owner, "cannot buy with yourself");

    auto exceed = in.amount - itr->next_price();

    // send exceed value
    action(  // send back spare money
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, from, asset(exceed, EOS_SYMBOL),
            std::string("qian da duo le"))
    ).send();    

    auto delta = itr->next_price() - itr->price;
    delta /= 100;    

    // 50% to the old owner 
    if (delta > 0) {
        action( // winner winner chicken dinner
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, itr->owner, asset(itr->price + delta*50, EOS_SYMBOL),
                std::string("gong xi jie tao!"))
        ).send();        
    }

    // 46 to the pool
    g.pool += delta*46;
    // update ed
    if (g.ed + 30 - now() > 12*60*60) {
        g.ed = now() + 12*60*60;
    } else {
        g.ed += 30;
    }
    _global.set(g, _self);

    // update land price
    _land.modify(itr, 0, [&](auto &t) {
        t.owner = from;
        t.price = itr->next_price();
    });    
}

void hotpotato::onTransfer(account_name from, account_name to, extended_asset quantity, string& memo){
    if (to != _self) return;
    require_auth(from);

    eosio_assert(quantity.is_valid(), "Invalid token transfer");
    eosio_assert(quantity.amount > 0, "must buy a positive amount");
    
    auto params = split(memo, ' ');
    eosio_assert(params.size() >= 1, "Error params");

    if (params[0] == "buy_land") {
        buy_land(from, quantity, params);
        return;
    }
}
