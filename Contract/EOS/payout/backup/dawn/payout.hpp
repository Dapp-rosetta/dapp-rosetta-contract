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

    [[eosio::action]] void stake(account_name from, uint64_t delta);
    [[eosio::action]] void unstake(account_name from, uint64_t delta);
    [[eosio::action]] void claim(const account_name &account);

    struct [[eosio::table]] voter_info {
        account_name to = 0;
        uint64_t staked = 0;
        uint64_t payout = 0;
    };

    struct [[eosio::table]] st_global {
        uint64_t defer_id = 0;
        uint64_t total_staked;
        uint128_t earnings_per_share;
    };
    typedef singleton<N(voters), voter_info> singleton_voters;
    typedef singleton<N(global), st_global> singleton_global;
    singleton_global _global;

    void apply(account_name code, action_name action) {
        auto &thiscontract = *this;

        if (action == N(transfer)) {
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