/**
 *  @dev minakokojima, rukamoemoe51
 */

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <string>

using namespace eosio;
using namespace std;

struct st_transfer {
    name   from;
    name   to;
    asset  quantity;
    string memo;
};

CONTRACT escrow : public contract {
public:
    escrow(name receiver, name code, datastream<const char*> ds): 
        contract(receiver, code, ds) {
    }

    ACTION transfer(name from, name to, asset quantity, string memo);

    void onTransfer(name from, name to, extended_asset in, string memo) {        
    
        if (to != _self) return;
        require_auth(from);
        eosio_assert(in.quantity.is_valid(), "invalid token transfer");
        eosio_assert(in.quantity.amount > 0, "must transfer a positive amount");
        eosio_assert(in.contract == _code, "code is different");

        auto a = asset(in.quantity.amount / 2, in.quantity.symbol);
        auto b = asset(in.quantity.amount - in.quantity.amount / 2, in.quantity.symbol);

        if (a.amount > 0) {
            action(
                permission_level{_self, "active"_n},
                in.contract, name("transfer"),
                make_tuple(_self, name("minakokojima"), a,
                string(""))
            ).send();
        }

        if (b.amount > 0) {            
            action(
                permission_level{_self, name("active")},
                in.contract, name("transfer"),
                make_tuple(_self, name("rukamoemoe51"), b,
                string(""))
            ).send();
        }
    } // onTransfer()

    void apply(uint64_t receiver, uint64_t code, uint64_t action) {
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
        escrow p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(receiver, code, action);
        eosio_exit(0);
    }
}