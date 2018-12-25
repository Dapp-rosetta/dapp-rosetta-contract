/**
 *  @dev minakokojima, yukiexe
 *  @env cdt v1.2.x
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>

#include "config.hpp"
#include "utils.hpp"

using namespace eosio;
using namespace std;

struct st_transfer {
    name   from;
    name   to;
    asset  quantity;
    string memo;
};

CONTRACT payout : public contract {
public:
    payout(name receiver, name code, datastream<const char*> ds): 
        contract(receiver, code, ds),
        _global(receiver, receiver.value) {
    }

    TABLE voter_info {
        name     to;
        asset    staked;
        int64_t  payout;        
    };

    TABLE refund_request {
        time     request_time;
        asset    amount;
    };

    TABLE global_info {
        uint64_t defer_id;
        asset    total_staked;
        int128_t earnings_per_share;
    };

    typedef singleton<"voters"_n, voter_info> singleton_voters;
    typedef singleton<"global"_n, global_info> singleton_global;
    typedef singleton<"refunds"_n, refund_request> singleton_refunds;

    singleton_global _global;

    uint64_t get_next_defer_id() {
        auto g = _global.get();    
        g.defer_id += 1;
        _global.set(g,_self);
        return g.defer_id;
    }

    void send_defer_refund_action(name from) {
        transaction out;
        out.actions.emplace_back(
            permission_level{ _self, "active"_n}, 
            _self, "refund"_n, 
            from);
        out.delay_sec = refund_delay;         
        cancel_deferred(from.value); // TODO: Remove this line when replacing deferred trxs is fixed
        out.send(from.value, _self, true);
    }

    template <typename... Args>
    void send_defer_action(Args&&... args) {
        transaction trx;
        trx.actions.emplace_back(std::forward<Args>(args)...);
        trx.send(get_next_defer_id(), _self, false);
    }    


    ACTION init();
    ACTION unstake(name from, asset delta);
    ACTION claim(name from);    
    ACTION refund(name from);    
    ACTION transfer(name from, name to, asset quantity, string memo);
    void onTransfer(name from, name to, extended_asset in, string memo);
    void stake(name from, asset delta);
    void make_profit(uint64_t delta);

    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        auto &thiscontract = *this;
        if (action == name("transfer").value) {
            auto transfer_data = unpack_action_data<st_transfer>();
            onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, name(code)), transfer_data.memo);
            return;
        }

        switch (action) {
            EOSIO_DISPATCH_HELPER(payout, 
                (init)
                (unstake)
                (claim)
                (refund)
            )
        }
    }
};

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        payout p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(receiver, code, action);
        eosio_exit(0);
    }
}