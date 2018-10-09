#include <eosiolib/currency.hpp>
#include <eosiolib/asset.hpp>
#include <math.h>
#include <string>

#define EOS S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using namespace eosio;
using namespace std;

typedef double real_type;

class escrow : public contract
{
public:
    escrow(account_name self) : 
        contract(self) {
    }
    
    void onTransfer(account_name from, account_name to,
                    extended_asset quantity, string memo);        

    void apply(account_name code, action_name action) {
        auto &thiscontract = *this;

        if (action == N(transfer)) {
            auto transfer_data = unpack_action_data<currency::transfer>();
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