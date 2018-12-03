#include "payout.hpp"

void payout::stake(name from, asset delta) {
    require_auth(from);
    eosio_assert(delta.amount > 0, "must stake a positive amount");
    singleton_voters _voters(_self, from.value);
    auto v = _voters.get_or_create(_self, voter_info{});
    auto g = _global.get();
    v.staked += delta;
    v.payout += g.earnings_per_share * delta.amount / MAGNITUDE;
    _voters.set(v, _self);
    g.total_staked += delta;
    _global.set(g, _self);    
}

void payout::unstake(name from, asset delta) {
    require_auth(from);
    singleton_voters _voters(_self, from.value);
    auto v = _voters.get_or_create(_self, voter_info{});
    auto g = _global.get();
    eosio_assert(delta <= v.staked, "don't have enough token for unstake");

    action(
        permission_level{_self, "active"_n},
        TOKEN_CONTRACT, "transfer"_n,
        make_tuple(_self, from, delta,
                   std::string("transfer token by unstake"))
    ).send();

    if (g.earnings_per_share * delta.amount / MAGNITUDE <= v.payout) {
        v.payout -= g.earnings_per_share * delta.amount / MAGNITUDE;
    } else {
        v.payout = 0;
    }

    v.staked -= delta;
    _voters.set(v, _self);
}

void payout::claim(name from) {
    require_auth(from);
    singleton_voters _voters(_self, from.value);
    auto v = _voters.get_or_create(_self, voter_info{});
    auto g = _global.get();


    // TODO(minakokojima): unvote(v);
    auto delta = asset(0, EOS_SYMBOL);
    auto raw_payout = g.earnings_per_share * v.staked.amount / MAGNITUDE;
    if (raw_payout > v.payout) delta.amount = raw_payout - v.payout;

    v.payout = raw_payout;
    _voters.set(v, _self);

    const auto& sym = eosio::symbol_type(EOS_SYMBOL).name();
    accounts eos_account("eosio.token"_n, _self);
    auto _balance = eos_account.get(sym).balance;

    if (delta.amount > 0 && delta.amount <= _balance) {
        send_defer_action(
            permission_level{_self, "active"_n},
            EOS_CONTRACT, "transfer"_n,
            make_tuple(_self, from, delta,
                string("claim dividend."))
        );
    } else if (delta.amount > 0 && delta.amount > _balance) {
        singleton_refund _refund(_self, from);
        auto r = _refund.get_or_create(_self, refund_request{});
        
    }   
}


void payout::onTransfer(name from, name to, extended_asset quantity, string memo) {        

    /*
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
 
        return;
    }    

    if (params[0] == "make_profit") {
        eosio_assert(quantity.contract == N(eosio.token), "must use true EOS to make profit");
        eosio_assert(quantity.quantity.symbol == EOS_SYMBOL, "must use EOS to make profit");
        // make_profit(quantity.quantity.amount);
        return;
    }
    */
}