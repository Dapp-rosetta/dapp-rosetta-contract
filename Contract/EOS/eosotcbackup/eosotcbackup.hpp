#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <cmath>
#include <string>

#define EOS S(4, EOS)
#define TOKEN_CONTRACT N(eosio.token)

using namespace eosio;
using namespace std;

struct st_transfer {
    name   from;
    name   to;
    asset  quantity;
    string memo;
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



CONTRACT eosotcbackup : public contract {
public:
    using contract::contract;

    void init();    
    void clean();
    void test();
        
    void onTransfer(name from, name to, extended_asset in, string memo);
    void retrieve(name owner, uint64_t order_id, extended_asset ask);                    
    void ask(name owner, extended_asset bid, extended_asset ask);
    void take(name owner, uint64_t order_id, extended_asset bid, extended_asset ask);

    struct [[eosio::table]] order {
        uint64_t id;
        name owner;         // 发起者
        extended_asset bid; // 提供
        extended_asset ask; // 需求
        auto primary_key() const {return id;}
    };

    typedef eosio::multi_index<"order"_n, order> order_index;
    
    /*
    // @abi action
    void receipt(const order& recepit) {
        require_auth(_self);
    }*/

    void apply(uint64_t code, uint64_t action) { 
        auto &thiscontract = *this;

        if (action == name("transfer").value) {
            auto transfer_data = unpack_action_data<st_transfer>();
            onTransfer(transfer_data.from, transfer_data.to, extended_asset(transfer_data.quantity, name(code)), transfer_data.memo);
            return;
        }
    
        if (name(code) != _self) return;
        switch (action) {
            //EOSIO_DISPATCH(eosotcbackup, (init)(test)(clean)(retrieve));
        };
    }    
};

extern "C" {
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        eosotcbackup p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(code, action);
        eosio_exit(0);
    }
}