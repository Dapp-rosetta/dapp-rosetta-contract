#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
using namespace eosio;

CONTRACT petshop : public contract {
public:
    petshop(name receiver, name code, datastream<const char*> ds)
        :contract(receiver, code, ds), pets(receiver, receiver.value) {}

    ACTION adopt(name from, int petId) {
        auto itr = pets.get();
        itr.owner[petId] = from;
        pets.set(itr, _self);
    }

    TABLE pet {
        name owner[16];
    };

    typedef singleton<"pet"_n, pet> pet_table;
    pet_table pets;
};

EOSIO_DISPATCH( petshop, (adopt) )

