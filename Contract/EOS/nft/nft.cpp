#include "nft.hpp"

template <typename T>
void nft::transfer(T table, name from, name to, const uint64_t &id, string memo)
{
    // 驗證權限
    eosio_assert(from != to, "cannot transfer to self");
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");

    // 回執
    require_recipient(from);
    require_recipient(to);

    // 驗證 token
    auto itr = table.require_find(id, "Unable to find NFT");
    /*
            eosio_assert(quantity.is_valid(), "invalid quantity");
            eosio_assert(token.amount > 0, "must transfer positive quantity");
            eosio_assert(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    */
    eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

    // 誰付 ram 錢
    auto payer = has_auth(to) ? to : from;

    // 表操作
    table.modify(itr, payer, [&](auto &t) {
        t.owner = to;
    });
}