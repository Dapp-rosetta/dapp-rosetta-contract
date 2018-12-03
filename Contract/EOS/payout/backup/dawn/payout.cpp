#include "payout.hpp"

void payout::stake(account_name from, uint64_t delta)
{
    require_auth(from);
    eosio_assert(delta > 0, "must stake a positive amount");
    singleton_voters _voters(_self, from);
    auto v = _voters.get_or_create(_self, voter_info{});
    auto g = _global.get();
    v.staked += delta;
    v.payout += g.earnings_per_share * delta / MAGNITUDE;
    _voters.set(v, _self);
}

void payout::unstake(account_name from, uint64_t delta)
{
    require_auth(from);
    singleton_voters _voters(_self, from);
    auto v = _voters.get_or_create(_self, voter_info{});
    auto g = _global.get();
    eosio_assert(delta <= v.staked, "don't have enough CMU for unstake");

    action(
        permission_level{_self, N(active)},
        N(dacincubator), N(transfer),
        make_tuple(_self, from, asset(delta, TOKEN_SYMBOL),
                   std::string("transfer token by unstake")))
        .send();

    if (g.earnings_per_share * delta / MAGNITUDE <= v.payout)
    {
        v.payout -= g.earnings_per_share * delta / MAGNITUDE;
    }
    else
    {
        v.payout = 0;
    }

    v.staked -= delta;
    _voters.set(v, _self);
}

void payout::claim(const account_name &account)
{
    require_auth(account);
    singleton_voters _voters(_self, account);
    auto v = _voters.get_or_create(_self, voter_info{});
    auto g = _global.get();

    // TODO(minakokojima): unvote(v);
    uint64_t delta = 0;
    if (g.earnings_per_share * v.staked / MAGNITUDE <= v.payout)
        delta = 0;
    else
        delta = g.earnings_per_share * v.staked / MAGNITUDE - v.payout;

    v.payout = g.earnings_per_share * v.staked / MAGNITUDE;
    _voters.set(v, _self);

    if (delta > 0) {
        send_defer_action(
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, account,
                       asset(delta, EOS_SYMBOL),
                       string("claim dividend.")));
    }

    // p.pool_profit = 0;
    // _players.set(p, _self);
}

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