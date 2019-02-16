/**
 *  @dev minakokojima, yukiexe
 *  @copyright Andoromeda
 *
 */
#pragma once
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
 
namespace nft { // Non-Fungible Tokens
    
    using std::string;
    using namespace eosio;

    struct st_basic_token {
        uint64_t     id;
        name owner ;
        uint64_t primary_key() const { return id; }
    };    

    struct st_feature_tradeable {
        uint64_t price ;
    };

    struct st_nft : st_basic_token, st_feature_tradeable {};

    /* 1. 第一個問題，所有權記載在哪個合約上？
     * 2. 在合約中的哪個 table 裡？
     * 3. 實際的transfer實現
     * ref:
     * https://github.com/EOSIO/eosio.contracts/tree/master/eosio.token
     * 
     * https://stackoverflow.com/questions/15572665/c-structs-with-member-functions-vs-classes-with-public-variables
     * http://www.drdobbs.com/cpp/how-non-member-functions-improve-encapsu/184401197
     * 
    */
    template <typename T>
    void transfer(T table, name from, name to, const uint64_t &id, string memo);

    /*
    void create(name from) {
    }

    void buy(name from) {
        
    }
    */

} /// namespace nft