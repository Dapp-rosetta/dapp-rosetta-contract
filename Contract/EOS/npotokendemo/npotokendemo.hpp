/**
 *  @dev minakokojima
 */

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <string>

using namespace eosio;
using namespace std;

struct st_transfer {
    name   from;
    name   to;
    asset  quantity;
    string memo;
};

CONTRACT npotokendemo : public contract {
public:
    npotokendemo(name receiver, name code, datastream<const char*> ds):
        contract(receiver, code, ds), connector(receiver, receiver.value) {
    }

   /**
    * 取钱，NPO 从存款池中取钱，并减少单位 NPO token 的价值
    *
    * @param from - 取款人
    * @param out - 取款金额 
    */    
    void take_out(name from, extended_asset out);    
    
   /**
    * 存錢，并印制对应金额的 NPO token 给存款人
    *
    * @param from - 存款人
    * @param in - 存款金额
    */    
    void put_in(name from, extended_asset in);

    void onTransfer(name from, name to, extended_asset in, string memo);    

    TABLE connector_t {
        asset supply;
        asset balance;
    };

    typedef singleton<"connector"_n, connector_t> connector_table;
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
        npotokendemo p( name(receiver), name(code), datastream<const char*>(nullptr, 0) );
        p.apply(receiver, code, action);
        eosio_exit(0);
    }
}
