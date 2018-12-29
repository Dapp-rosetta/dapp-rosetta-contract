/**
 *  @copyright
 */

#include "pomelo.hpp"

namespace kyubey {

/**
 * @brief Clean buyorders & sellorders table
 *
 * @param str_symbol - token's symbol
 */
void pomelo::clean(string str_symbol) {
    require_auth(get_self());
   
    buyorders_t t(get_self(), symbol(str_symbol, 4).raw());
    while (t.begin() != t.end()) {    
        auto itr = t.begin();
        action(
            permission_level{get_self(), "active"_n},
            TOKEN_CONTRACT, "transfer"_n,
            make_tuple(get_self(), itr->account, itr->bid,
                std::string("trade cancel successed."))
        ).send();   
        t.erase(itr);
    }

    sellorders_t t2(get_self(), symbol(str_symbol, 4).raw());
    while (t2.begin() != t2.end()) {    
        auto itr = t2.begin();        
        action(
            permission_level{get_self(), "active"_n},
            get_contract_name_by_symbol(itr->bid.symbol), "transfer"_n,
            make_tuple(get_self(), itr->account, itr->bid,
                std::string("trade cancel successed.."))
        ).send();    
        t2.erase(itr);
    }  
}

uint64_t pomelo::string_to_amount(string s) {
    uint64_t z = 0;
    for (int i=0;i<s.size();++i) {
        if ('0' <= s[i] && s[i] <= '9') {
            z *= 10; 
            z += s[i] - '0';
        }
    }
    return z;
}

vector<string> pomelo::split(string src, char c) {
    vector<string> z;
    string t;
    for (int i = 0; i < src.size(); ++i){
        if (src[i] == c) {
            z.push_back(t);
            t.clear();
        } 
        else t += src[i];
        
    }
  if (!t.empty()) z.push_back(t);
  return z;
}

name pomelo::get_contract_name_by_symbol(symbol sym) {
    if ( sym == EOS_SYMBOL ) return EOS_CONTRACT ;
    auto _whitelist = whitelist_index_t(get_self(), sym.raw());
    return name(_whitelist.get().contract);
}

template <typename T>
void pomelo::publish_order(name account, asset bid, asset ask) {
    const bool isBuyorder = bid.symbol == EOS_SYMBOL ;
    if ( isBuyorder ) { // Validate bid & ask symbol
        eosio_assert(ask.is_valid(), "Ask must be valid");
        eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS");
    }
    else {
        eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS");
    }
    if (ask.amount > 0) {
        T _table( get_self(), isBuyorder ? ask.symbol.raw() : bid.symbol.raw());
        _table.emplace(get_self(), [&](auto &t) {
            t.id = _table.available_primary_key();
            t.account = account.value;
            t.bid = bid;
            t.ask = ask;
            // EOS / XXX
            t.unit_price = isBuyorder ? (bid.amount * PRICE_SCALE / ask.amount) : (ask.amount * PRICE_SCALE / bid.amount);
            t.timestamp = current_time();

            action(
                permission_level{get_self(), "active"_n},
                get_self(), isBuyorder ? "buyreceipt"_n : "sellreceipt"_n, t)
                .send();
        });
    }
}

void pomelo::action_transfer_token(const name &to, const asset &quantity, const string memo )
{
    action( // Transfer Token
        permission_level{get_self(), "active"_n},
        get_contract_name_by_symbol(quantity.symbol), "transfer"_n,
        make_tuple(get_self(), to, quantity, memo))
        .send();
}

void pomelo::match_processing(const bool &isBuyorder, const match_record &m_rec)
{
    action(
        permission_level{get_self(), "active"_n},
        get_self(), (isBuyorder ? "buymatch"_n : "sellmatch"_n), m_rec)
        .send();

    action_transfer_token(name(isBuyorder ? m_rec.asker : m_rec.bidder),
                          isBuyorder ? m_rec.bid : m_rec.ask);
    action_transfer_token(name(isBuyorder ? m_rec.bidder : m_rec.asker),
                          isBuyorder ? m_rec.ask : m_rec.bid);
}

void pomelo::buy(name account, asset bid, asset ask) {
    eosio_assert(bid.symbol == EOS_SYMBOL, "Bid must be EOS");                                    // Validate bid symbol
    eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS...");                             // Validate ask symbol
    eosio_assert(is_valid_unit_price(bid.amount, ask.amount), "Bid mod ask must be 0!!!"); // Validate unit price is integer
    asset &bid_eos = bid ;

    uint64_t order_unit_price = bid_eos.amount * PRICE_SCALE / ask.amount; // Calculate unit price  
            
    sellorders_t sell_table(get_self(), ask.symbol.raw()); // Retrive the sell table for current token
    auto unit_price_index = sell_table.get_index<"byprice"_n>(); // Get unit price index
    
    for (auto itr = unit_price_index.begin(); itr != unit_price_index.end();){ // Visit sell orders table
        if (itr->unit_price > order_unit_price) 
            break;

        uint64_t sold_token = ask.amount <= itr->bid.amount ? ask.amount : itr->bid.amount;
        uint64_t sold_eos = sold_token * itr->unit_price / PRICE_SCALE;

        unit_price_index.modify(itr, get_self(), [&](auto& t) { // Modify sell order record
            t.bid.amount -= sold_token;
            t.ask.amount -= sold_eos;
        });
        
        bid_eos.amount -= sold_eos;
        ask.amount -= sold_eos * PRICE_SCALE / order_unit_price;        
        
        auto token_contract = get_contract_name_by_symbol(ask.symbol); // Retrive issue contract of this token

        action(
            permission_level{get_self(), "active"_n},
            get_self(), "buymatch"_n,
            match_record{
                .id = itr->id,
                .bidder = account.value,
                .asker = itr->account,
                .bid = asset(sold_eos, EOS_SYMBOL),
                .ask = asset(sold_token, ask.symbol),
                .unit_price = itr->unit_price,
                .timestamp = static_cast<time>(current_time()),
            }
        ).send();

        action( // Transfer EOS to seller
            permission_level{ get_self(), "active"_n },
            TOKEN_CONTRACT, "transfer"_n,
            make_tuple(get_self(), itr->account, asset(sold_eos, EOS_SYMBOL), string("transfer")))
        .send();
    
        action( // Transfer Token to buyer
            permission_level{ get_self(), "active"_n },
            token_contract, "transfer"_n,
            make_tuple(get_self(), account, asset(sold_token, ask.symbol), string("transfer")))
        .send();
                
        // Erase the sell order from sell order table if the order has been took.
        if (itr->ask.amount == 0 || itr->bid.amount == 0) {
            itr = unit_price_index.erase(itr);
            if (bid_eos.amount == 0 || ask.amount == 0) return;
        }        
        else return;
    }

    publish_order<buyorders_t>(account, bid_eos, ask); // The current order is not fully matched, publish the order
}

void pomelo::sell(name account, asset bid, asset ask) {
    // Validate bid symbol
    // Validate ask symbol
    // Validate unit price is integer
    eosio_assert(bid.symbol != EOS_SYMBOL, "Bid must be non-EOS");
    eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS..");    
    eosio_assert(is_valid_unit_price(ask.amount, bid.amount), "Ask mod bid must be 0");

    // Retrive the buy table for current token
    buyorders_t buy_table(get_self(), bid.symbol.raw());
    auto order_unit_price = ask.amount * PRICE_SCALE / bid.amount; // Calculate unit price
    auto unit_price_index = buy_table.get_index<"byprice"_n>(); // Get unit price index
    
    // Visit buy orders table    
    for (auto itr = unit_price_index.begin(); itr != unit_price_index.end(); ) {    
        if (itr->unit_price < order_unit_price) break;

        uint64_t sold_token = bid.amount <= itr->ask.amount ? bid.amount : itr->ask.amount;
        uint64_t sold_eos = sold_token * itr->unit_price / PRICE_SCALE;

        // Modify sell order record
        unit_price_index.modify(itr, get_self(), [&](auto& t) {
            t.bid.amount -= sold_eos;
            t.ask.amount -= sold_token;
        });

        bid.amount -= sold_token;
        ask.amount -= sold_token * order_unit_price / PRICE_SCALE;
        
        // Retrive issue contract of this token
        auto token_contract = get_contract_name_by_symbol(bid.symbol);

        action(
            permission_level{get_self(), "active"_n},
            get_self(), "sellmatch"_n,
            match_record {
                .id = itr->id,
                .bidder = itr->account,
                .asker = account.value,
                .bid = asset(sold_token, bid.symbol),
                .ask = asset(sold_eos, EOS_SYMBOL),
                .unit_price = itr->unit_price,
                .timestamp = static_cast<time>(current_time()),
            }
        ).send();

        action( // Transfer EOS to seller
            permission_level{ get_self(), "active"_n },
            TOKEN_CONTRACT, "transfer"_n,
            make_tuple(get_self(), name(account), asset(sold_eos, EOS_SYMBOL), string("transfer"))
        ).send();
    
        action( // Transfer Token to buyer
            permission_level{ get_self(), "active"_n },
            token_contract, "transfer"_n,
            make_tuple(get_self(), name(itr->account), asset(sold_token, bid.symbol), string("transfer"))
        ).send();
                
        // Erase the buy order from buy order table if the order has been took.
        if (itr->ask.amount == 0 || itr->bid.amount == 0) {
            itr = unit_price_index.erase(itr);
            if (bid.amount == 0 || ask.amount == 0) return;
        }        
        else return;
    }

    publish_order<sellorders_t>(account, bid, ask); // The current order is not fully matched, publish the order
}

template <typename T>
void pomelo::cancelorder(name &account, string &str_symbol, const uint64_t &id) {
    require_auth(account);

    symbol sym(str_symbol, 4);
    T _table(get_self(), sym.raw());  
    auto itr = _table.require_find(id, "Trade id is not found");
    eosio_assert(name(itr->account) == account || account == "kyubeydex.bp"_n, "Account does not match");

    action(
        permission_level{get_self(), "active"_n},
        get_contract_name_by_symbol(sym), "transfer"_n,
        make_tuple(get_self(), name(itr->account), itr->bid,
            std::string("trade cancel successed"))
    ).send();
    
    _table.erase(itr);
}

void pomelo::market_price_trade(const bool &isBuyorder, name account, asset bid, asset ask)
{
     if ( isBuyorder ) // Validate bid & ask symbol            
        eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS...");
    else 
        eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS..");

    auto unit_price_index = sellorders_t( get_self(), ask.symbol.raw() ).get_index< "byprice"_n >();
    auto unit_price_index2 = buyorders_t( get_self(), bid.symbol.raw() ).get_index< "byprice"_n >();
    auto itr_a = unit_price_index.begin();
    auto itr_b = unit_price_index2.rbegin();
    
    if ( isBuyorder ) { // refund for not found order
        if (itr_a == unit_price_index.end()) {
            action_transfer_token( account, bid, string("refund") );
            return;
        }
    } else {
        if ( itr_b == unit_price_index2.rend() ) {
            action_transfer_token( account, bid, string("refund") );
            return;
        }
    }

    // found
    uint64_t order_unit_price = isBuyorder ? itr_a->unit_price : itr_b->unit_price ; // EOS / KBY
    uint64_t max_amount_for_sell = bid.amount * PRICE_SCALE / order_unit_price ; 
    uint64_t sold_token = isBuyorder ? (max_amount_for_sell <= itr_a->bid.amount ? max_amount_for_sell : itr_a->bid.amount) :
                                       (max_amount_for_sell <= itr_b->ask.amount ? max_amount_for_sell : itr_b->ask.amount);    
    uint64_t sold_eos = sold_token * order_unit_price / PRICE_SCALE;
    uint64_t &delta = isBuyorder ? sold_eos : sold_token;
    uint64_t &rdelta = isBuyorder ? sold_token : sold_eos;
    auto lambda = [&](auto &t) {
        t.bid.amount -= rdelta;
        t.ask.amount -= delta;
    };

    if (isBuyorder) // Modify order record
        unit_price_index.modify(itr_a, get_self(), lambda);
    else
        unit_price_index2.modify( --unit_price_index2.end(), get_self(), lambda);

    bid.amount -= delta;
    
    auto rec_bid = isBuyorder ? asset(sold_eos, EOS_SYMBOL) : asset(sold_token, bid.symbol);
    auto rec_ask = isBuyorder ? asset(sold_token, ask.symbol) : asset(sold_eos, EOS_SYMBOL);

    match_processing(isBuyorder,
                     match_record{
                         .id = isBuyorder ? itr_a->id : itr_b->id,
                         .bidder = isBuyorder ? account.value : itr_b->account,
                         .asker = isBuyorder ? itr_a->account : account.value,
                         .bid = rec_bid,
                         .ask = rec_ask,
                         .unit_price = order_unit_price,
                         .timestamp = static_cast<time>(current_time()),
                     });

    if (isBuyorder) { // Erase the order from order table if the order has been took.
        if (itr_a->ask.amount == 0 || itr_a->bid.amount == 0)
            unit_price_index.erase(itr_a);
    }
    else {
        if (itr_b->ask.amount == 0 || itr_b->bid.amount == 0)
            unit_price_index2.erase(--unit_price_index2.end());
    }

    if (bid.amount != 0)
        market_price_trade( isBuyorder, account, bid, ask );
}

void pomelo::setwhitelist(string str_symbol, name issuer) {
    require_auth(get_self());
    whitelist_index_t _whitelist(get_self(), symbol(str_symbol, 4).raw());
    _whitelist.set( whitelist{ .contract = issuer.value }, get_self()); 
}

void pomelo::rmwhitelist(string str_symbol) {
    require_auth(get_self());
    whitelist_index_t _whitelist(get_self(), symbol(str_symbol, 4).raw());
    _whitelist.remove();
}

void pomelo::onTransfer( name from, name to, asset bid, string memo ) {
    // x.xxxx KBY price 0.0000000
    // x.xxxx EOS price 0.0000000 
    if (to != get_self()) return;    
    require_auth(from);
    eosio_assert(bid.is_valid(), "invalid token transfer");
    eosio_assert(bid.amount > 0, "must be a positive amount");

    auto splited_asset = split(memo, ' ');
    eosio_assert( splited_asset.size() == 2, "Memo error");

    asset ask( string_to_amount(splited_asset[0]),
               symbol(splited_asset[1], 4)
             );

    if ( ask.amount == 0 )
        market_price_trade(bid.symbol == EOS_SYMBOL, from, bid, ask);

    if (bid.symbol == EOS_SYMBOL) {
        eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS");
        buy(from, bid, ask);
    }
    else {
        eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS");
        sell(from, bid, ask);
    }
}

} /// namespace kyubey 