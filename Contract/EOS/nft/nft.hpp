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


        /* 1. 第一個問題，所有權記載在哪個合約上？
         * 2. 在合約中的哪個 table 裡？
         * 3. 實際的transfer實現
         * 參考 https://github.com/EOSIO/eosio.contracts/tree/master/eosio.token
        */
        template <typename T, typename T2>
        void transfer(name receiver, name code, name from, name to, T2 &token, string memo) {
            // 驗證權限
            eosio_assert(from != to, "cannot transfer to self");
            require_auth(from);
            eosio_assert(is_account(to), "to account does not exist");

            // 開表
            //auto sym = quantity.symbol.code();
            T table(receiver, code.value);
            //const auto &st = statstable.get(sym.raw());

            // 回執
            require_recipient(from);
            require_recipient(to);

            // 驗證 token
            /*
            eosio_assert(quantity.is_valid(), "invalid quantity");
            eosio_assert(token.amount > 0, "must transfer positive quantity");
            eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
            */
            eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");
            
            // 誰付 ram 錢
            auto payer = has_auth(to) ? to : from;

            // 表操作
            /*
            sub_balance(from, quantity);
            add_balance(to, quantity, payer);
            */
            auto itr = table.require_find( id, "Unable to find NFT" );
            table.modify(itr, receiver, [&](auto &nft) {
                nft.owner = to ;
            });
        }
    };    

    struct st_feature_tradeable {
        uint64_t price ;
    };

    struct st_nft : st_basic_token, st_feature_tradeable {};

    /*
    void create(name from) {
    }

    void buy(name from) {
        
    }
    */

} /// namespace nft