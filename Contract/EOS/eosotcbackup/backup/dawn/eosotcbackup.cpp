/**
 *  @dev minakokojima, yukiexe
 */

#include "eosotcbackup.hpp"


void eosotcbackup::init() {
    require_auth(_self);    
}

void eosotcbackup::clean() {
    require_auth(_self);
    /*
    while(offers.begin() != offers.end()) {        
	    offers.erase(offers.begin());
    }
    */
}

void eosotcbackup::test() {
    require_auth(_self);
}

void eosotcbackup::ask(const account_name &owner, const extended_asset &bid, const extended_asset &ask) {
    eosio_assert(bid.contract != N("eosotcbackup"), "fake" );
    eosio_assert(ask.contract != N("eosotcbackup"), "fake" );
    order_index orders(_self, ask.contract);    
    
    auto id = orders.available_primary_key();

    orders.emplace(_self, [&](auto& o) {    
        o.id = id;
        o.owner = owner;
        o.bid = bid;
        o.ask = ask;
        action(permission_level{_self, N(active)},
            _self, N(receipt), o
        ).send();
    });
}

void eosotcbackup::take(const account_name &owner, const uint64_t &order_id, const extended_asset &bid, const extended_asset &ask) {
    
    const char* str_fake_currency = "fake currency." ;
    eosio_assert(bid.contract != N("eosotcbackup"), str_fake_currency );
    eosio_assert(ask.contract != N("eosotcbackup"), str_fake_currency );
    
    order_index orders(_self, bid.contract); 
    auto itr = orders.find(order_id);  
    if (itr->bid.contract == N("eosotcbackup") ){
        eosio_assert(false, str_fake_currency );
    }

    eosio_assert(itr != orders.end(), "order is not exist.");

  //  eosio_assert(itr->bid == ask, "ask is not equal to order bid.");
//    eosio_assert(itr->ask == bid, "bid is not equal to order ask.");
// bid: 6  1
// ask: 3  1.5
    // partial take

    eosio_assert( 
        uint128_t(itr->bid.quantity.amount) * bid.quantity.amount > uint128_t(itr->ask.quantity.amount) * ask.quantity.amount, 
       "price is too high");

    auto t = now();
    
    if (itr->bid.quantity.amount <= ask.quantity.amount) {
        auto _bid = itr->bid.quantity;
        auto _ask = itr->ask.quantity;

        action(permission_level{_self, N(active)},
            _self, N(receipt), *itr)
        .send();          

        // 将 bid 交给买家        
        action(
            permission_level{_self, N(active)},
            ask.contract, N(transfer),
            make_tuple(_self, owner, _bid,
                std::string("trade success"))
        ).send();

        // 将 ask 交给卖家
        action(
            permission_level{_self, N(active)},
            bid.contract, N(transfer),
            make_tuple(_self, itr->owner, _ask,
                std::string("trade success"))
        ).send();        

        orders.erase(itr);
    } else {
        auto _bid = ask.quantity;
        auto _ask = bid.quantity;
        _ask.amount = (uint128_t(ask.quantity.amount) * itr->bid.quantity.amount) / itr->ask.quantity.amount;
        

        action(permission_level{_self, N(active)},
            _self, N(receipt), *itr)
        .send();

        // 将 bid 交给买家           
        action(
            permission_level{_self, N(active)},
            ask.contract, N(transfer),
            make_tuple(_self, owner, _bid,
                std::string("trade success"))
        ).send();

        // 将 ask 交给卖家
        action(
            permission_level{_self, N(active)},
            bid.contract, N(transfer),
            make_tuple(_self, itr->owner, _ask,
                std::string("trade success"))
        ).send();

        orders.modify(itr, 0, [&](auto &o) {
            o.bid.quantity.amount -= _ask.amount;
            o.ask.quantity.amount -= _bid.amount;
        });
    } 
}

void eosotcbackup::retrieve(account_name owner, uint64_t order_id, extended_asset ask) {
    order_index orders(_self, ask.contract); 
    auto itr = orders.find(order_id);    
    eosio_assert(itr != orders.end(), "order is not exist.");
    eosio_assert(itr->owner == owner, "not the owner.");
    
    action(permission_level{_self, N(active)},
        _self, N(receipt), *itr
    ).send();

    auto _bid = itr->bid.quantity;

    action(
        permission_level{_self, N(active)},
        itr->bid.contract, N(transfer),
        make_tuple(_self, owner, _bid,
            std::string("order retrieve"))
    ).send();
    orders.erase(itr);
}
    
// memo [ask,0.5000 HPY,happyeosslot]
// memo [take,0.5000 HPY,happyeosslot,id]
void eosotcbackup::onTransfer(account_name from, account_name to, extended_asset bid, std::string memo) {        
    
    if (to != _self) return;
    
    require_auth(from);
    eosio_assert(bid.quantity.is_valid(), "invalid token transfer");
    eosio_assert(bid.quantity.amount > 0, "must bid a positive amount");
    
    if (memo.substr(0, 3) == "ask") {        
        memo.erase(0, 4);
        std::size_t p = memo.find(',');
        std::size_t f = memo.find('.');  
        std::size_t s = memo.find(' ');   

        extended_asset _ask;
        _ask.quantity.amount = string_to_price(memo.substr(0, s));
        _ask.quantity.symbol = string_to_symbol(s-f-1, memo.substr(s+1, s-p-1).c_str());

        eosio_assert(_ask.quantity.is_valid(), "invalid token in ask");
        eosio_assert(_ask.quantity.amount > 0, "must ask a positive amount");

        eosio_assert(bid.quantity.is_valid(), "invalid token in bid");
        eosio_assert(bid.quantity.amount > 0, "must bid a positive amount");

        memo.erase(0, p+1);
        auto issuer = name { .value = string_to_name(memo.c_str()) };
        _ask.contract = issuer;
        ask(from, bid, _ask);
    } else if (memo.substr(0, 4) == "take"){	
        memo.erase(0, 5);

    // memo
    // take,1.0000 HPY,happyeosslot,7

        std::size_t p = memo.find(',');
        std::size_t f = memo.find('.');  
        std::size_t s = memo.find(' ');   

        extended_asset _ask;
        _ask.quantity.amount = string_to_price(memo.substr(0, s));
        _ask.quantity.symbol = string_to_symbol(s-f-1, memo.substr(s+1, p-s-1).c_str());
    
        eosio_assert(_ask.quantity.is_valid(), "invalid token transfer");
        eosio_assert(_ask.quantity.amount > 0, "must ask a positive amount");
        memo.erase(0, p+1);

        p = memo.find(',');

        
        auto issuer = name { .value = string_to_name(memo.substr(0, p).c_str()) } ;
        _ask.contract = issuer;
        memo.erase(0, p+1);
        auto id = string_to_price(memo);        
        take(from, id, bid, _ask);
    }
}