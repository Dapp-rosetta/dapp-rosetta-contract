/**
 *  @dev minakokojima, yukiexe
 *  @env cdt v1.2.x
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/transaction.h>

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

static constexpr uint32_t refund_delay = 1*24*3600;

CONTRACT payout : public contract {
public:
    payout(name receiver, name code, datastream<const char*> ds): 
        contract(receiver, code, ds),
        _global(receiver, receiver.value) {
    }

    void onTransfer(name from, name to, extended_asset in, string memo);

    [[eosio::action]] void stake(name from, asset delta);
    [[eosio::action]] void unstake(name from, asset delta);
    [[eosio::action]] void claim(name from);

    struct [[eosio::table]] voter_info {
        name     to;
        asset    staked;
        int64_t  payout;        
    };

    struct [[eosio::table]] refund_request {
        uint32_t request_time;
        asset    amount;
    };

    struct [[eosio::table]] global_info {
        uint64_t defer_id;
        asset    total_staked;
        int128_t earnings_per_share;
    };

    typedef singleton<"voters"_n, voter_info> singleton_voters;
    typedef singleton<"global"_n, global_info> singleton_global;
    typedef singleton<"refunds"_n, refund_request> refunds_table;

    singleton_global _global;

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
        trx.delay_sec = REFUND_DELAY;
        trx.send(get_next_defer_id(), _self, false);
    }    

   void refund(name owner) {
        require_auth( owner );
        
        refunds_table refunds_tbl( _self, owner.value );
        eosio_assert( refunds_tbl.exists(), "refund request not found" );
        auto req = refunds_tbl.get();
        eosio_assert( req.request_time + refund_delay <= now(), "refund is not available yet" );
        
        // Until now() becomes NOW, the fact that now() is the timestamp of the previous block could in theory
        // allow people to get their tokens earlier than the 3 day delay if the unstake happened immediately after many
        // consecutive missed blocks.

      //  INLINE_ACTION_SENDER(eosio::token, transfer)( N(eosio.token), {N(eosio.stake),N(active)},
        //                                            { N(eosio.stake), req->owner, req->net_amount + req->cpu_amount, std::string("unstake") } );
        
        action(
            permission_level{_self, "active"_n},
            "eosio.token"_n, "transfer"_n,
            make_tuple(_self, owner, req.amount,
                    std::string("refund"))
        ).send(); 

        refunds_tbl.remove();
    }

    ACTION apply(uint64_t code, uint64_t action) {
        auto &thiscontract = *this;
        if (action == name("transfer").value) {
            auto transfer_data = unpack_action_data<st_transfer>();
            onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, name(code)), transfer_data.memo);
            return;
        }
    }    
};

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        payout p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(code, action);
        eosio_exit(0);
    }
}