#include "pomelo.hpp"

/**
 * @brief Clear all transactions in a currency
 * 
 * @param str_symbol - Currency's name
 **/

void pomelo::clean(string str_symbol) {
    require_auth(get_self());
    /*
    while (buy_table.begin() != buy_table.end()) {
        buy_table.erase(buy_table.begin());
    }*/
    /*
    auto t = buyorders_t(get_self(), my_string_to_symbol(4, "ITECOIN"));
    while (t.begin() != t.end()) {
        t.erase(t.find(t.begin()->id));
    }
    auto t2 = sellorders_t(get_self(), my_string_to_symbol(4, "ITECOIN"));
    while (t2.begin() != t2.end()) {
        t2.erase(t2.find(t2.begin()->id));
    }*/

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

static string symbol_to_string( uint64_t symbol ) {
    symbol >>= 8;
    string z;
    for( uint32_t i = 0; symbol ;  ) {
        z += (char)(symbol & 0xff);
        symbol >>= 8;
    }

    return z;
}

uint64_t pomelo::my_string_to_symbol(uint8_t precision, const char* str) {
    uint32_t len = 0;
    while (str[len]) {
        ++len;
    }
    uint64_t result = 0;
    for (uint32_t i = 0; i < len; ++i) {
        result |= (uint64_t(str[i]) << (8 * (i + 1)));
    }
    result |= uint64_t(precision);
    return result >> 8;
}
/**
 * @brief Check if the currency corresponds to the correct contract
 * 
 * @param sym - currency
 * @param contract - contract
 **/
void pomelo::assert_whitelist(symbol sym, name contract) {
    auto account = get_contract_name_by_symbol(sym);
    eosio_assert(account == contract, "Transfer code does not match the contract in whitelist.");
}

/**
 * @brief Check if the currency corresponds to the correct contract
 * 
 * @param sym_symbol - currency
 * @param contract - contract
 **/
void pomelo::assert_whitelist(string str_symbol, name contract) {
    auto account = get_contract_name_by_symbol(str_symbol);
    eosio_assert(account == contract, "Transfer code does not match the contract in whitelist.");
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

/**
 * @brief Find the corresponding contract in the whitelist by currency
 * 
 * @param sym_symbol - currency
 **/

name pomelo::get_contract_name_by_symbol(symbol sym) {
    auto whitelist = whitelist_index_t(get_self(), sym.raw());
    return name(whitelist.get().contract);
}

/**
 * @brief If the buy operation does not match exactly, add the purchased demand to the table and print the log
 * 
 * @param account - buyer
 * @param bid - Bid (EOS)
 * @param ask - Other currencies required
 **/
void pomelo::publish_buyorder_if_needed(name account, asset bid, asset ask) {
    // Validate bid & ask symbol
    eosio_assert(bid.symbol == EOS_SYMBOL, "Bid must be EOS.");
    eosio_assert(ask.is_valid(), "Ask must be valid");
    eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS");

    if (ask.amount > 0) {
        buyorders_t buy_table(get_self(), ask.symbol.raw());
        buy_table.emplace(get_self(), [&](auto& t) {
            t.id = buy_table.available_primary_key();
            t.account = account.value;
            t.ask = ask;
            t.bid = bid;
            t.unit_price = bid.amount * PRICE_SCALE / ask.amount;
            t.timestamp = current_time();

            action(
                permission_level{ get_self(), "active"_n },
                get_self(), "buyreceipt"_n, t
            ).send();      
        });
    }
}

/**
 * @brief If the sell operation does not match exactly, add the Selling demand to the table and print the log
 * 
 * @param account - seller
 * @param bid - Other currencies required
 * @param ask - ask (EOS)
 **/
void pomelo::publish_sellorder_if_needed(name account, asset bid, asset ask) {
    // Validate bid & ask symbol
    eosio_assert(bid.symbol != EOS_SYMBOL, "Bid must be non-EOS");
    eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS");

    if (ask.amount > 0) {
        sellorders_t sell_table(get_self(), bid.symbol.raw());
        sell_table.emplace(get_self(), [&](auto& t) {
            t.id = sell_table.available_primary_key();
            t.account = account.value;
            t.ask = ask;
            t.bid = bid;
            t.unit_price = ask.amount * PRICE_SCALE / bid.amount;
            t.timestamp = current_time();
            action(
                permission_level{ get_self(), "active"_n },
                get_self(), "sellreceipt"_n, t
           ).send();      
        });
    }
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
void pomelo::buy(name account, asset bid, asset ask) {
    eosio_assert(bid.symbol == EOS_SYMBOL, "Bid must be EOS");                                    // Validate bid symbol
    eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS...");                             // Validate ask symbol
    eosio_assert(is_valid_unit_price(bid.amount, ask.amount), "Bid mod ask must be 0!!!"); // Validate unit price is integer

    uint64_t order_unit_price = bid.amount * PRICE_SCALE / ask.amount; // Calculate unit price  
            
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
        
        bid.amount -= sold_eos;
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
            if (bid.amount == 0 || ask.amount == 0) return;
        }        
        else return;
    }

    // The current order is not fully matched, publish the order
    publish_buyorder_if_needed(account, bid, ask);
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

    // The current order is not fully matched, publish the order
    publish_sellorder_if_needed(account, bid, ask);
}

/**
 * @brief Cancel an existing buy order
 * 
 * @param account - buyer
 * @param str_symbol - Currency name
 * @param id - Order id 
 **/
void pomelo::cancelbuy(name account, string str_symbol, uint64_t id) {
    require_auth(account);

    buyorders_t buy_table(get_self(), symbol(str_symbol, 4).raw());    
    auto itr = buy_table.find(id); 
    
    eosio_assert(itr != buy_table.end(), "Trade id is not found");
    eosio_assert(name(itr->account) == account || account == "kyubeydex.bp"_n, "Account does not match");

    action(
        permission_level{get_self(), "active"_n},
        TOKEN_CONTRACT, "transfer"_n,
        make_tuple(get_self(), name(itr->account), itr->bid,
            std::string("trade cancel successed"))
    ).send();    

    buy_table.erase(itr);
}

/**
 * @brief Cancel an existing sell order
 * 
 * @param account - buyer
 * @param str_symbol - Currency name
 * @param id - Order id 
 **/
void pomelo::cancelsell(name account, string str_symbol, uint64_t id) {
    require_auth(account);
    
    sellorders_t sell_table(get_self(), symbol(str_symbol, 4).raw());  
    auto itr = sell_table.find(id);
    
    eosio_assert(name(itr->account) == account || account == "kyubeydex.bp"_n, "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");

    action(
        permission_level{get_self(), "active"_n},
        get_contract_name_by_symbol(str_symbol), "transfer"_n,
        make_tuple(get_self(), name(itr->account), itr->bid,
            std::string("trade cancel successed"))
    ).send();
    
    sell_table.erase(itr);
}

/**
 * @brief Set the whitelist of currencies
 * 
 * @param str_symbol - Currency name
 * @param issuer - Currency contract address
 **/
void pomelo::setwhitelist(string str_symbol, name issuer) {
    require_auth(get_self());
    whitelist w; w.contract = issuer.value;
    whitelist_index_t whitelist(get_self(), symbol(str_symbol, 4).raw());
    whitelist.set(w, get_self()); 
}

/**
 * @brief Remove currency from whitelist
 * 
 * @param str_symbol - Currency name
 **/
void pomelo::rmwhitelist(string str_symbol) {
    require_auth(get_self());
    whitelist_index_t whitelist(get_self(), symbol(str_symbol, 4).raw());
    whitelist.remove();
}

/**
 * @brief Determining the transfer type by memo
 * 
 * @param from - The originator of the transfer
 * @param to - Transfer address
 * @param bid - Transfer amount
 * @param memo - Use of transfer
 **/
void pomelo::onTransfer( name from, name to, asset bid, string memo ) {
    // x.xxxx KBY price 0.0000000
    // x.xxxx EOS price 0.0000000 
    if (to != get_self()) return;    
    require_auth(from);
    eosio_assert(bid.is_valid(), "invalid token transfer");
    eosio_assert(bid.amount > 0, "must bet a positive amount");

    auto splited_asset = split(memo, ' ');
    eosio_assert( splited_asset.size() == 3, "Memo error");

    asset ask( string_to_amount(splited_asset[0]),
               symbol(splited_asset[1], 4)
             );

    if (bid.symbol == EOS_SYMBOL) {
        //eosio_assert(ask.symbol == S(4, "PXL"), "123");
        //eosio_assert(ask.symbol == string_to_symbol(4, "PXL"), "123");
        // note(minakokojima): S() and string_to_symbol is not equal?

        eosio_assert(ask.symbol != EOS_SYMBOL, "Ask must be non-EOS");
        buy(from, bid, ask);
    }
    else {
        eosio_assert(ask.symbol == EOS_SYMBOL, "Ask must be EOS");
        sell(from, bid, ask);
    }
}