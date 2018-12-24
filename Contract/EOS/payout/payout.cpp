#include "payout.hpp"

void payout::init() {
    require_auth(_self);
}

void payout::make_profit(uint64_t delta) {
    auto g = _global.get();
    g.earnings_per_share += MAGNITUDE * delta / g.total_staked.amount;
    _global.set(g, _self);
}

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

    if (g.earnings_per_share * delta.amount / MAGNITUDE <= v.payout) {
        v.payout -= g.earnings_per_share * delta.amount / MAGNITUDE;
    } else {
        v.payout = 0;
    }

    v.staked -= delta;
    _voters.set(v, _self);    

    singleton_refunds _refunds( _self, from.value );
    auto req = _refunds.get_or_create(_self, refund_request{.amount = asset(0, EOS_SYMBOL)});
    req.request_time = now();
    req.amount += delta;
    send_defer_refund_action(from);
    _refunds.set(req, _self);
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

    if (delta.amount > 0) {
        action(
            permission_level{_self, "active"_n},
            EOS_CONTRACT, "transfer"_n,
            make_tuple(_self, from, delta,
                string("claim dividend."))
        ).send();        
    }
}

void payout::refund(name from) {
    singleton_refunds refunds_tbl( _self, from.value );
    eosio_assert( refunds_tbl.exists(), "refund request not found" );
    auto req = refunds_tbl.get();
    eosio_assert( req.request_time + refund_delay <= now(), "refund is not available yet" );
    
    // Until now() becomes NOW, the fact that now() is the timestamp of the previous block could in theory
    // allow people to get their tokens earlier than the 1 day delay if the unstake happened immediately after many
    // consecutive missed blocks.

    action(
        permission_level{_self, "active"_n},
        EOS_CONTRACT, "transfer"_n,
        make_tuple(_self, from, req.amount, "unstake refund")
    ).send();

    refunds_tbl.remove();
}

void payout::onTransfer(name from, name to, extended_asset in, string memo) {        

    if (to != _self) return;
    require_auth(from);
    eosio_assert(in.quantity.is_valid(), "invalid token transfer");
    eosio_assert(in.quantity.amount > 0, "must transfer a positive amount");

    auto params = split(memo, ' ');
    eosio_assert(params.size() >= 1, "error memo");    
    
    if (params[0] == "stake") {        
        eosio_assert(in.contract == TOKEN_CONTRACT, "must use true target TOKEN to stake");
        eosio_assert(in.quantity.symbol == TOKEN_SYMBOL, "must use target TOKEN to stake");
        stake(from, in.quantity);
        return;
    }    
    
    if (params[0] == "make_profit") {
        eosio_assert(in.contract == EOS_CONTRACT, "must use true EOS to make profit");
        eosio_assert(in.quantity.symbol == EOS_SYMBOL, "must use EOS to make profit");
        make_profit(in.quantity.amount);
        return;
    }
}