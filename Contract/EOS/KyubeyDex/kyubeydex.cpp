/**
 *  @copyright
 *  Rule1: Any scope is used for token locating, must be symbol.code().raw() .
 */

#include "kyubeydex.hpp"

namespace kyubey {

/**
 * @brief Clean buyorders & sellorders table
 *
 * @param str_symbol - token's symbol
 */
void kyubeydex::clean( const string str_symbol) {
    require_auth(get_self());
    eosio::symbol_code code( str_symbol );
    buyorders_t t(get_self(), code.raw());
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

    sellorders_t t2(get_self(), code.raw());
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

uint64_t kyubeydex::string_to_amount( const string &s ) const {
    uint64_t z = 0;
    for (int i=0;i<s.size();++i) {
        if ('0' <= s[i] && s[i] <= '9') {
            z *= 10; 
            z += s[i] - '0';
        }
    }
    return z;
}

asset kyubeydex::string_to_asset( const string &s1, const string &s2 ) const {
    int64_t a = 0;
    uint8_t p = 0;
    for (int i=0;i<s1.size();++i) { // 01.3456
        if ( s1[i] == '.' ) p = s1.size() - (i + 1); 
        if ('0' <= s1[i] && s1[i] <= '9') {
            a *= 10; 
            a += s1[i] - '0';
        }
    }
    return asset{a, symbol{s2, p}};
}

vector<string> kyubeydex::split( const string &src, const char c ) const {
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

/**
 * @brief Find the corresponding contract in the whitelist by currency
 * 
 * @param sym_symbol - currency
 **/
name kyubeydex::get_contract_name_by_symbol (symbol sym) const {
    if ( sym == EOS_SYMBOL ) return EOS_CONTRACT ;
    auto _whitelist = whitelist_index_t(get_self(), sym.code().raw());
    return name(_whitelist.get().contract);
}

/**
 * @brief If the buy operation does not match exactly, add the purchased demand to the table and print the log
 * 
 * @param account - buyer
 * @param bid - Bid (EOS)
 * @param ask - Other currencies required

 * @brief If the sell operation does not match exactly, add the Selling demand to the table and print the log
 * 
 * @param account - seller
 * @param bid - Other currencies required
 * @param ask - ask (EOS)
 **/
template <typename T>
void kyubeydex::publish_order(name account, asset bid, asset ask) {
    const bool isBuyorder = bid.symbol == EOS_SYMBOL ;
    if ( isBuyorder ) { // Validate bid & ask symbol
        eosio_assert(ask.is_valid(), "Ask must be valid");
        eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS");
    }
    else {
        eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS");
    }
    if (ask.amount > 0) {
        T _table( get_self(), isBuyorder ? ask.symbol.code().raw() : bid.symbol.code().raw());
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

void kyubeydex::action_transfer_token(const name &to, const asset &quantity, const string memo )
{
    action( // Transfer Token
        permission_level{get_self(), "active"_n},
        get_contract_name_by_symbol(quantity.symbol), "transfer"_n,
        make_tuple(get_self(), to, quantity, memo))
        .send();
}

void kyubeydex::match_processing(const match_record &rec)
{
    const bool isBuyorder = rec.bid.symbol == EOS_SYMBOL ;

    action(
        permission_level{get_self(), "active"_n},
        get_self(), (isBuyorder ? "buymatch"_n : "sellmatch"_n), rec)
        .send();

    const asset &eos = isBuyorder ? rec.bid : rec.ask ;
    const asset &token = isBuyorder ? rec.ask : rec.bid ;
    eosio_assert(eos.symbol == EOS_SYMBOL, "1st asset must be EOS @ match");
    eosio_assert(token.symbol != EOS_SYMBOL, "2nd asset must be non-EOS @ match");

    action_transfer_token(name(rec.asker), eos);
    action_transfer_token(name(rec.bidder), token);
}

/**
 * @brief In the sell form, the unit price is searched for from the low to the high and then
 *        print the completed transaction as a log. Finally, publish the transaction without an exact match.
 * 
 * @param account - buyer
 * @param bid - (EOS)
 * @param ask - Other currencies required
 * 
 **/
void kyubeydex::buy(name account, asset bid, asset ask) {
    eosio_assert(bid.symbol == EOS_SYMBOL, "Bid must be EOS");                                    // Validate bid symbol
    eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS...");                             // Validate ask symbol
    eosio_assert(is_valid_unit_price(bid.amount, ask.amount), "Bid mod ask must be 0!!!"); // Validate unit price is integer
    asset &bid_eos = bid ;

    // 100.0001 EOS / 10.001 IQ
    // 1000001 / 10001
    uint64_t order_unit_price = bid_eos.amount * PRICE_SCALE / ask.amount; // Calculate unit price  
    
    sellorders_t sell_table(get_self(), ask.symbol.code().raw()); // Retrive the sell table for current token
    auto unit_price_index = sell_table.get_index<"byprice"_n>(); // Get unit price index
    for (auto itr = unit_price_index.begin(); itr != unit_price_index.end();){ // Visit sell orders table
        if (itr->unit_price > order_unit_price) break;

        int64_t sold_token = ask.amount <= itr->bid.amount ? ask.amount : itr->bid.amount;
        int64_t sold_eos = sold_token * itr->unit_price / PRICE_SCALE;

        unit_price_index.modify(itr, get_self(), [&](auto& t) { // Modify sell order record
            t.bid.amount -= sold_token;
            t.ask.amount -= sold_eos;
        });
        
        bid_eos.amount -= sold_eos;
        ask.amount -= sold_eos * PRICE_SCALE / order_unit_price;

        match_processing(match_record{
            .id = itr->id,
            .bidder = account.value,
            .asker = itr->account,
            .bid = asset{sold_eos, EOS_SYMBOL},
            .ask = asset{sold_token, ask.symbol},
            .unit_price = itr->unit_price,
            .timestamp = static_cast<time>(current_time()),
        });
         
        // Erase the sell order from sell order table if the order has been took.
        if (itr->ask.amount == 0 || itr->bid.amount == 0) {
            itr = unit_price_index.erase(itr);
            if (bid_eos.amount == 0 || ask.amount == 0) return;
        }        
        else return;
    }

    publish_order<buyorders_t>(account, bid_eos, ask); // The current order is not fully matched, publish the order
}

/**
 * @brief In the buy form, the unit price is searched for from the high to the low and then
 *        print the completed transaction as a log. Finally, publish the transaction without an exact match.
 * 
 * @param account - seller
 * @param bid - Other currencies required
 * @param ask - ask(EOS)
 * 
 **/
void kyubeydex::sell(name account, asset bid, asset ask) {
    eosio_assert(bid.symbol != EOS_SYMBOL, "Bid must be non-EOS"); // Validate bid symbol & ask symbol
    eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS..");    
    eosio_assert(is_valid_unit_price(ask.amount, bid.amount), "Ask mod bid must be 0"); // Validate unit price is integer

    buyorders_t buy_table(get_self(), bid.symbol.code().raw()); // Retrive the buy table for current token
    auto order_unit_price = ask.amount * PRICE_SCALE / bid.amount; // Calculate unit price
    auto unit_price_index = buy_table.get_index<"byprice"_n>(); // Get unit price index
    
    for (auto itr = unit_price_index.begin(); itr != unit_price_index.end(); ) {  // Visit buy orders table  
        if (itr->unit_price < order_unit_price) break;

        int64_t sold_token = bid.amount <= itr->ask.amount ? bid.amount : itr->ask.amount;
        int64_t sold_eos = sold_token * itr->unit_price / PRICE_SCALE;

        // Modify sell order record
        unit_price_index.modify(itr, get_self(), [&](auto& t) {
            t.bid.amount -= sold_eos;
            t.ask.amount -= sold_token;
        });

        bid.amount -= sold_token;
        ask.amount -= sold_token * order_unit_price / PRICE_SCALE;

        match_processing(match_record{
            .id = itr->id,
            .bidder = itr->account,
            .asker = account.value,
            .bid = asset{sold_token, bid.symbol},
            .ask = asset{sold_eos, EOS_SYMBOL},
            .unit_price = itr->unit_price,
            .timestamp = static_cast<time>(current_time()),
        });
        
        // Erase the buy order from buy order table if the order has been took.
        if (itr->ask.amount == 0 || itr->bid.amount == 0) {
            itr = unit_price_index.erase(itr);
            if (bid.amount == 0 || ask.amount == 0) return;
        }        
        else return;
    }

    publish_order<sellorders_t>(account, bid, ask); // The current order is not fully matched, publish the order
}

/**
 * @brief Cancel an existing buy or sell order 
 * 
 * @param account - buyer
 * @param sym - token symbol
 * @param id - Order id 
 **/
template <typename T>
void kyubeydex::cancelorder( const name &executor, const symbol_code &code, const uint64_t &id) {
    T _table(get_self(), code.raw());
    auto itr = _table.require_find(id, "Trade id is not found");
    eosio_assert(executor == name(itr->account) || executor == "kyubeydex.bp"_n, "Account does not match");

    action_transfer_token( name(itr->account), itr->bid, string("trade cancel successed") );
    
    _table.erase(itr);
}

void kyubeydex::market_price_trade(name account, asset bid, asset ask)
{
    const bool isBuyorder = bid.symbol == EOS_SYMBOL ;
    if ( isBuyorder ) // Validate bid & ask symbol            
        eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS...");
    else 
        eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS..");

    sellorders_t sell_table(get_self(), ask.symbol.code().raw());
    buyorders_t buy_table(get_self(), bid.symbol.code().raw());
    auto unit_price_index_sell_table = sell_table.get_index<"byprice"_n>();
    auto unit_price_index_buy_table = buy_table.get_index<"byprice"_n>();
    auto itr_a = unit_price_index_sell_table.begin();
    auto itr_b = unit_price_index_buy_table.begin();
    
    if ( isBuyorder ) { // refund for not found order
        if (itr_a == unit_price_index_sell_table.end()) {
            action_transfer_token( account, bid, string("refund") );
            return;
        }
    } else {
        if ( itr_b == unit_price_index_buy_table.end() ) {
            action_transfer_token( account, bid, string("refund") );
            return;
        }
    }
    
    // found
    uint64_t order_unit_price = isBuyorder ? itr_a->unit_price : itr_b->unit_price ; // EOS / KBY
    eosio_assert(order_unit_price != 0, "No 0.");
    uint64_t max_amount_for_sell = bid.amount * PRICE_SCALE / order_unit_price ;
    eosio_assert(max_amount_for_sell != 0, "No 0.");
    int64_t sold_token = isBuyorder ? (max_amount_for_sell <= itr_a->bid.amount ? max_amount_for_sell : itr_a->bid.amount) :
                                       (max_amount_for_sell <= itr_b->ask.amount ? max_amount_for_sell : itr_b->ask.amount);    
    int64_t sold_eos = static_cast<int64_t>(sold_token * order_unit_price / PRICE_SCALE);
    eosio_assert(sold_eos != 0, "No 0.");
    int64_t &delta = isBuyorder ? sold_eos : sold_token;
    int64_t &rdelta = isBuyorder ? sold_token : sold_eos;
    auto lambda = [&](auto &t) {
        t.bid.amount -= rdelta;
        t.ask.amount -= delta;
    };
    
    if (isBuyorder) // Modify order record
        unit_price_index_sell_table.modify(itr_a, get_self(), lambda);
    else
        unit_price_index_buy_table.modify( itr_b/*--unit_price_index_buy_table.end()*/, get_self(), lambda);

    bid.amount -= delta;
    
    auto rec_bid = isBuyorder ? asset{sold_eos, EOS_SYMBOL} : asset{sold_token, bid.symbol};
    auto rec_ask = isBuyorder ? asset{sold_token, ask.symbol} : asset{sold_eos, EOS_SYMBOL};

    match_processing(match_record{
        .id = isBuyorder ? itr_a->id : itr_b->id,
        .bidder = isBuyorder ? account.value : itr_b->account,
        .asker = isBuyorder ? itr_a->account : account.value,
        .bid = rec_bid,
        .ask = rec_ask,
        .unit_price = order_unit_price,
        .timestamp = static_cast<time>(current_time()),
    });

    if (isBuyorder) { // Erase the order from order table if the order has been took.
        if (itr_a->bid.amount == 0 || itr_a->ask.amount == 0)
            unit_price_index_sell_table.erase(itr_a); 
    } else {
        if (itr_b->bid.amount == 0 || itr_b->ask.amount == 0)
            unit_price_index_buy_table.erase(itr_b/*--unit_price_index_buy_table.end()*/);
    }

    if (bid.amount != 0) market_price_trade(account, bid, ask); // next run
}

/**
 * @brief Set the whitelist of currencies
 * 
 * @param str_symbol - Currency name
 * @param issuer - Currency contract address
 **/
void kyubeydex::setwhitelist( name issuer, asset token ) {
    require_auth(get_self());
    whitelist_index_t _whitelist(get_self(), token.symbol.code().raw());
    _whitelist.set( whitelist{ .contract = issuer.value }, get_self()); 
}

/**
 * @brief Remove currency from whitelist
 * 
 * @param str_symbol - Currency name
 **/
void kyubeydex::rmwhitelist( asset token ) {
    require_auth(get_self());
    whitelist_index_t _whitelist(get_self(), token.symbol.code().raw());
    _whitelist.remove();
}

/**
 * @brief Determining the transfer type by memo
 * 
 * @param from - The originator of the transfer
 * @param to - Transfer address
 * @param bid - Transfer amount
 * @param memo - Use of transfer
 **/
void kyubeydex::onTransfer( name from, name to, asset bid, string memo ) {
    // x.xxxx EOS
    // x.xxxx KBY
    if (to != get_self()) return;    
    require_auth(from);
    eosio_assert(bid.is_valid(), "invalid token transfer");
    eosio_assert(bid.amount > 0, "must be a positive amount");

    auto splited_asset = split(memo, ' ');
    eosio_assert(splited_asset.size() == 2, "Memo error");
    asset ask = string_to_asset(splited_asset[0], splited_asset[1]);

    if ( ask.amount == 0 ) {
        // eosio_assert( false, "Testing");
        market_price_trade(from, bid, ask);
        return ;
    }
    if (bid.symbol == EOS_SYMBOL) {
        eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS");
        buy(from, bid, ask);
        return ;
    }
    else {
        eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS");
        sell(from, bid, ask);
        return ;
    }
}

} /// namespace kyubey 