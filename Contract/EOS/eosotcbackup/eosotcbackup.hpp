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
    void ask(const account_name &owner, const extended_asset &bid, const extended_asset &ask);
    void take(const account_name &owner, const uint64_t &order_id, const extended_asset &bid, const extended_asset &ask);
              
    // @abi table
    struct order {
        uint64_t id;
        account_name owner; // 发起者
        extended_asset bid; // 提供
        extended_asset ask; // 需求
        auto primary_key() const {return id;}
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
        auto transfer_data = unpack_action_data<st_transfer>();
        onTransfer(transfer_data.from, transfer_data.to, extended_asset( transfer_data.quantity,  name { .value= code } ), transfer_data.memo);
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