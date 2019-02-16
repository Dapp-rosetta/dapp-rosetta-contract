#include "nft.hpp"

template <typename T, typename Token>
void nft::transfer(T table, name from, name to, const Token &token, string memo)
{
    // 驗證權限
    eosio_assert(from != to, "cannot transfer to self");
    require_auth(from);
    eosio_assert(is_account(to), "to account does not exist");

    // 開表
    //auto sym = quantity.symbol.code();
    // T table(receiver, code.value);
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
    auto itr = table.require_find(token.id, "Unable to find NFT");
    table.modify(itr, payer, [&](auto &t) {
        t.owner = to;
    });
}