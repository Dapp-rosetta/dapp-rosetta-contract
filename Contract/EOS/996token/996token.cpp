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

    void take_out(name from, extended_asset out) {
        if (from != _self) return;
        auto itr = connector.get();
        delta = itr.supply / itr.balance * in.amout;
        itr.balance -= out.amount;

        action(
            permission_level{_self, "active"_n},
            out.contract, name("transfer"),
            make_tuple(_self, from, out.amount,
            string("take out some eos"))
        ).send();
    }

    void put_in(name from, extented_asset in) {
        auto itr = connector.get();
        delta = itr.supply / itr.balance * in.amout;
        itr.balance += in.amount;
        itr.supply += delta;
        connector.set(itr, _self);

        action(
            permission_level{_self, "active"_n},
            _self, name("transfer"),
            make_tuple(_self, from, delta,
            string("issue new token"))
        ).send();
    }

    void onTransfer(name from, name to, extended_asset in, string memo) {
        if (to != _self) return;
        require_auth(from);
        eosio_assert(in.quantity.is_valid(), "invalid token transfer");
        eosio_assert(in.quantity.amount > 0, "must transfer a positive amount");
        eosio_assert(in.quantity.symbol == "EOS 4"_n, "only allow eos");
        eosio_assert(in.contract == "eosio.token"_n, "only allow eos");
        put_in(from, in);
    } // onTransfer()


    TABLE connector_t {
        asset supply;
        asset balance;
    };

    typedef singleton<"connector_t"_n, connector_t> connector_table;
    connector_table connector;

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
