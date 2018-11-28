/**
 *  @dev minakokojima, yukiexe
 *  @env cdt v1.2.x
 */

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>

#include "config.hpp"
#include "utils.hpp"

using namespace eosio;
using namespace std;

struct st_transfer {
    account_name from;
    account_name to;
    asset        quantity;
    string       memo;

    EOSLIB_SERIALIZE( st_transfer, (from)(to)(quantity)(memo) )
};

class payout : public contract
{
  public:
    payout(account_name self) : contract(self),
                                _global(self, self) {}


    uint64_t get_next_defer_id() {
    auto g = _global.get();    
    g.defer_id += 1;
    _global.set(g,_self);
        return g.defer_id;
    }

    template <typename... Args>
    void send_defer_action(Args&&... args) {
        transaction trx;
        trx.actions.emplace_back(std::forward<Args>(args)...);
        trx.send(get_next_defer_id(), _self, false);
    }

    
    void onTransfer(account_name from, account_name to, extended_asset quantity, std::string memo);

    [[eosio::action]] void stake(account_name from, uint64_t delta)
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

    [[eosio::action]] void unstake(account_name from, uint64_t delta) {
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

    [[eosio::action]] void claim(const account_name &account)
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

        //singleton_players _players(_self, from);
        //auto p = _players.get_or_create(_self, player_info{});

        if (delta > 0)
        {
            send_defer_action(
                permission_level{_self, N(active)},
                TOKEN_CONTRACT, N(transfer),
                make_tuple(_self, account,
                           asset(delta, TOKEN_SYMBOL),
                           string("claim dividend.")));
        }

        // p.pool_profit = 0;
        // _players.set(p, _self);
    }

    struct [[eosio::table]] voter_info
    {
        account_name to = 0;
        uint64_t staked = 0;
        uint64_t payout = 0;
    };

    struct [[eosio::table]] st_global
    {
        uint64_t defer_id = 0;
        uint64_t total_staked;
        uint128_t earnings_per_share;
    };
    typedef singleton<N(voters), voter_info> singleton_voters;
    typedef singleton<N(global), st_global> singleton_global;
    singleton_global _global;

    void apply(account_name code, action_name action)
    {
        auto &thiscontract = *this;

        if (action == N(transfer))
        {
            auto transfer_data = unpack_action_data<st_transfer>();
            onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, name{.value = code}), transfer_data.memo);
            return;
        }
    }
};

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) 
    {
        payout p(receiver);
        p.apply(code, action);
        eosio_exit(0);
    }
}