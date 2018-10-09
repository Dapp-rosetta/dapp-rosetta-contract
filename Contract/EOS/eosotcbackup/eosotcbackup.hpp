#include <eosiolib/currency.hpp>
#include <eosiolib/asset.hpp>
#include <math.h>
#include <string>

#define EOS S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using namespace eosio;
using namespace std;

typedef double real_type;

class eosotcbackup : public contract
{
public:
    eosotcbackup(account_name self) : 
        contract(self) {
    }

    // @abi action
    void init();    
    // @abi action    
    void clean();
    // @abi action 
    void test();
    // @abi action    
    void retrieve(account_name owner, uint64_t order_id, extended_asset ask);    
    // @abi action
    void apply(account_name code, action_name action);
    
    void onTransfer(account_name from, account_name to,
                    extended_asset quantity, string memo);
    void ask(account_name owner, extended_asset bid, extended_asset ask);
    void take(account_name owner, uint64_t order_id, extended_asset bid, extended_asset ask);
              
    // @abi table
    struct order {
        uint64_t id;
        account_name owner; // 发起者
        extended_asset bid; // 提供
        extended_asset ask; // 需求
        uint64_t primary_key() const {return id;}
    };    
    typedef eosio::multi_index<N(order), order> order_index;
    
    // @abi action
    void receipt(const order& recepit) {
        require_auth(_self);
    }
};

void eosotcbackup::apply(account_name code, action_name action) {   
    auto &thiscontract = *this;

    if (action == N(transfer)) {
        auto transfer_data = unpack_action_data<currency::transfer>();
        onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, code), transfer_data.memo);
        return;
    }

    if (code != _self) return;
    switch (action) {
        EOSIO_API(eosotcbackup, (init)(test)(clean)(retrieve));
    };
}

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) 
    {
        eosotcbackup p(receiver);
        p.apply(code, action);
        eosio_exit(0);
    }
}