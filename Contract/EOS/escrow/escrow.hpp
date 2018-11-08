/**
 *  @dev minakokojima, yukiexe
 */

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <cmath>
#include <string>

#define EOS S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using namespace eosio;
using namespace std;


struct st_transfer {
    account_name from;
    account_name to;
    asset        quantity;
    string       memo;

    EOSLIB_SERIALIZE( st_transfer, (from)(to)(quantity)(memo) )
};

uint64_t string_to_price(string s) {
    uint64_t z = 0;
    for (int i=0;i<s.size();++i) {
        if ('0' <= s[i] && s[i] <= '9') {
            z *= 10; 
            z += s[i] - '0';
        }
    }
    return z;
} // string_to_price()
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
    
        asset a = asset(quantity.symbol, quantity.amount / 2);
        asset b = asset(quantity.symbol, quantity.amount - quantity.amount / 2);

        if (a.amount > 0) {
            action(
                permission_level{_self, N(active)},
                quantity.countract, N(transfer),
                make_tuple(_self, N(minakokojima), a,
                std::string(""))
            ).send();
        }
        if (b.amount > 0) {
            action(
                permission_level{_self, N(active)},
                quantity.countract, N(transfer),
                make_tuple(_self, N(rukamoemoe51), b,
                std::string(""))
            ).send();
        }
    } // onTransfer()


    void apply(account_name code, action_name action) {
        auto &thiscontract = *this;

        if (action == N(transfer)) {
            auto transfer_data = unpack_action_data<st_transfer>();
            onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, code), transfer_data.memo);
            return;
        }
    }                    
};

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) 
    {
        escrow p(receiver);
        p.apply(code, action);
        eosio_exit(0);
    }
}