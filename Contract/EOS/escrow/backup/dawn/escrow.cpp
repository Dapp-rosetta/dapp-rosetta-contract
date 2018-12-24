/**
 *  @dev minakokojima, yukiexe
 */

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>

using namespace eosio;
using namespace std;

struct st_transfer {
    account_name from;
    account_name to;
    asset        quantity;
    string       memo;
};

class escrow : public contract
{
public:
    escrow(account_name self) : 
        contract(self) {
    }
    
    void onTransfer(account_name from, account_name to, extended_asset quantity, std::string memo) {        
    
        if (to != _self) return;
    
        require_auth(from);
        eosio_assert(quantity.is_valid(), "invalid token transfer");
        eosio_assert(quantity.amount > 0, "must transfer a positive amount");
    
        auto a = asset(quantity.symbol, quantity.amount / 2);
        auto b = asset(quantity.symbol, quantity.amount - quantity.amount / 2);

        if (a.amount > 0) {
            action(
                permission_level{_self, N(active)},
                quantity.contract, N(transfer),
                make_tuple(_self, N(minakokojima), a,
                std::string(""))
            ).send();
        }
        if (b.amount > 0) {
            action(
                permission_level{_self, N(active)},
                quantity.contract, N(transfer),
                make_tuple(_self, N(rukamoemoe51), b,
                std::string(""))
            ).send();
        }
    } // onTransfer()


    void apply(account_name code, action_name action) {
        auto &thiscontract = *this;

        if (action == N(transfer)) {
            auto transfer_data = unpack_action_data<st_transfer>();
            onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, name { .value= code } ), transfer_data.memo);
            return;
        }
    }
};

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        escrow p(receiver);
        p.apply(code, action);
        eosio_exit(0);
    }
}