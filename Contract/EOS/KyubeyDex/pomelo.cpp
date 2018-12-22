#include "pomelo.hpp"

void pomelo::clean(string symbol) {
    require_auth(_self);
/*
    while (buy_table.begin() != buy_table.end()) {
        buy_table.erase(buy_table.begin());
    }*/

/*
    auto t = buyorders(_self, my_string_to_symbol(4, "ITECOIN"));
    while (t.begin() != t.end()) {
        t.erase(t.find(t.begin()->id));
    }
    auto t2 = sellorders(_self, my_string_to_symbol(4, "ITECOIN"));
    while (t2.begin() != t2.end()) {
        t2.erase(t2.find(t2.begin()->id));
    }    */

    auto t  = buyorders(_self, my_string_to_symbol(4, symbol.c_str()));
    while (t.begin() != t.end()) {    
        auto itr = t.begin();        
        action(
            permission_level{_self, N(active)},
            N(eosio.token), N(transfer),
            make_tuple(_self, itr->account, itr->bid,
                std::string("trade cancel successed."))
        ).send();   
        t.erase(itr);
    }
    auto t2  = sellorders(_self, my_string_to_symbol(4, symbol.c_str()));
    while (t2.begin() != t2.end()) {    
        auto itr = t2.begin();        
        action(
            permission_level{_self, N(active)},
            get_contract_name_by_symbol(itr->bid.symbol), N(transfer),
            make_tuple(_self, itr->account, itr->bid,
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
    while (str[len]) 
    {
        ++len;
    }
    uint64_t result = 0;
    for (uint32_t i = 0; i < len; ++i) {
        result |= (uint64_t(str[i]) << (8 * (i + 1)));
    }
    result |= uint64_t(precision);
    return result >> 8;
}

void pomelo::assert_whitelist(string symbol, account_name contract)
{
    auto account = get_contract_name_by_symbol(symbol);
    eosio_assert(account == contract, "Transfer code does not match the contract in whitelist.");
}

void pomelo::assert_whitelist(symbol_type symbol, account_name contract)
{
    auto account = get_contract_name_by_symbol(symbol);
    eosio_assert(account == contract, "Transfer code does not match the contract in whitelist.");
}

bool pomelo::is_valid_unit_price(uint64_t eos, uint64_t non_eos) 
{
    return eos * PRICE_SCALE % non_eos == 0;
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

vector<string> pomelo::split(string src, char c) 
{
    vector<string> z;
    string t;
    for (int i = 0; i < src.size(); ++i){
        if (src[i] == c) 
        {
            z.push_back(t);
            t.clear();
        } 
        else 
        {
            t += src[i];
        }
  }
  if (!t.empty()) z.push_back(t);
  return z;
}

account_name pomelo::get_contract_name_by_symbol(string symbol)
{
//    return get_contract_name_by_symbol(my_string_to_symbol(4, symbol.c_str()));
    auto whitelist = whitelist_index(_self, my_string_to_symbol(4, symbol.c_str()));
    return whitelist.get().contract;
}

account_name pomelo::get_contract_name_by_symbol(symbol_type symbol) {
    auto whitelist = whitelist_index(_self, symbol.name());
    return whitelist.get().contract;
}

void pomelo::publish_buyorder_if_needed(account_name account, asset bid, asset ask)
{

    // Validate bid symbol
    eosio_assert(bid.symbol == EOS, "Bid must be EOS.");

    // Validate ask symbol
    eosio_assert(ask.is_valid(), "Ask must be valid");
    // Validate ask symbol
    eosio_assert(ask.symbol != EOS, "Ask must be non-EOS");

    if (ask.amount > 0)
    {
        auto buy_table = buyorders(_self, ask.symbol.name());
        
        auto id = buy_table.available_primary_key();

        buy_table.emplace(_self, [&](auto& t) {
            t.id = id;
            t.account = account;
            t.ask = ask;
            t.bid = bid;
            t.unit_price = bid.amount * PRICE_SCALE / ask.amount;
            t.timestamp = current_time();

            action(
                permission_level{ _self, N(active) },
                _self, N(buyreceipt), t
            ).send();      
        });

     
    }
}


void pomelo::publish_sellorder_if_needed(account_name account, asset bid, asset ask) {
    // Validate bid symbol
    eosio_assert(bid.symbol != EOS, "Bid must be non-EOS");

    // Validate ask symbol
    eosio_assert(ask.symbol == EOS, "Ask must be EOS");

    if (ask.amount > 0) {

        auto sell_table = sellorders(_self, bid.symbol.name());
        auto id = sell_table.available_primary_key();

        sell_table.emplace(_self, [&](auto& t) {
            t.id = id;
            t.account = account;
            t.ask = ask;
            t.bid = bid;
            t.unit_price = ask.amount * PRICE_SCALE / bid.amount;
            t.timestamp = current_time();
            action(
                permission_level{ _self, N(active) },
                _self, N(sellreceipt), t
           ).send();      
        });
    }
}

void pomelo::buy(account_name account, asset bid, asset ask) 
{
    
    eosio_assert(bid.symbol == EOS, "Bid must be EOS");                                    // Validate bid symbol
    eosio_assert(ask.symbol != EOS, "Ask must be non-EOS...");                             // Validate ask symbol
    eosio_assert(is_valid_unit_price(bid.amount, ask.amount), "Bid mod ask must be 0!!!"); // Validate unit price is integer

    uint64_t order_unit_price = bid.amount * PRICE_SCALE / ask.amount; // Calculate unit price  
            
    // Retrive the sell table for current token
    auto sell_table = sellorders(_self, ask.symbol.name());

    // Get unit price index
    auto unit_price_index = sell_table.get_index<N(byprice)>();    
    
    // Visit sell orders table
    for (auto itr = unit_price_index.begin(); itr != unit_price_index.end();)
    {
        if (itr->unit_price > order_unit_price) {
            break;
        }
        uint64_t sold_token = ask.amount <= itr->bid.amount ? ask.amount : itr->bid.amount;
        uint64_t sold_eos = sold_token * itr->unit_price / PRICE_SCALE;

        // Modify sell order record
        unit_price_index.modify(itr, 0, [&](auto& t) {
            t.bid.amount -= sold_token;
            t.ask.amount -= sold_eos;
        });
        
        bid.amount -= sold_eos;
        ask.amount -= sold_eos * PRICE_SCALE / order_unit_price;        
        
        // Retrive issue contract of this token
        auto token_contract = get_contract_name_by_symbol(ask.symbol);

        match_record m{
            .id = itr->id,
            .bidder = account,    
            .asker = itr->account,
            .bid = asset(sold_eos, EOS),
            .ask = asset(sold_token, ask.symbol),
            .unit_price = itr->unit_price,
            .timestamp = now(),
        };
        action(
            permission_level{ _self, N(active) },
            _self, N(buymatch), m
        ).send();
            
        // Transfer EOS to seller  
             
        action(
            permission_level{ _self, N(active) },
            TOKEN_CONTRACT, N(transfer),
            make_tuple(_self, itr->account, asset(sold_eos, EOS), string("transfer")))
        .send();
    

        // Transfer Token to buyer       
    
        action(
            permission_level{ _self, N(active) },
            token_contract, N(transfer),
            make_tuple(_self, account, asset(sold_token, ask.symbol), string("transfer")))
        .send();
                

        // Erase the sell order from sell order table if the order has been took.
        if (itr->ask.amount == 0 || itr->bid.amount == 0)
        {
            itr = unit_price_index.erase(itr);
            if (bid.amount == 0 || ask.amount == 0) {
                return;
            }
        }        
        else {
            return;
        }
    }

    // The current order is not fully matched, publish the order
    publish_buyorder_if_needed(account, bid, ask);
}

void pomelo::sell(account_name account, asset bid, asset ask) {
    // Validate bid symbol
    eosio_assert(bid.symbol != EOS, "Bid must be non-EOS");

    // Validate ask symbol
    eosio_assert(ask.symbol == EOS, "Ask must be EOS..");

    // Validate unit price is integer
    eosio_assert(is_valid_unit_price(ask.amount, bid.amount), "Ask mod bid must be 0");

    // Retrive the buy table for current token
    auto buy_table = buyorders(_self, bid.symbol.name());

    // Calculate unit price
    auto order_unit_price = ask.amount * PRICE_SCALE / bid.amount;

    // Get unit price index
    auto unit_price_index = buy_table.get_index<N(byprice)>();
    
    // Visit buy orders table    
    for (auto itr = unit_price_index.begin(); itr != unit_price_index.end(); ) {    

        if (itr->unit_price < order_unit_price) {
            break;
        }

        uint64_t sold_token = bid.amount <= itr->ask.amount ? bid.amount : itr->ask.amount;
        uint64_t sold_eos = sold_token * itr->unit_price / PRICE_SCALE;

        // Modify sell order record

        unit_price_index.modify(itr, 0, [&](auto& t) {
            t.bid.amount -= sold_eos;
            t.ask.amount -= sold_token;
        });

        bid.amount -= sold_token;
        ask.amount -= sold_token * order_unit_price / PRICE_SCALE;
        
        // Retrive issue contract of this token
        auto token_contract = get_contract_name_by_symbol(bid.symbol);

        match_record m;
        m.id = itr->id;
        m.bidder = itr->account;
        m.asker = account;
        m.bid = asset(sold_token, bid.symbol);
        m.ask = asset(sold_eos, EOS);
        m.unit_price = itr->unit_price;
        m.timestamp = now();

        action(
            permission_level{ _self, N(active) },
            _self, N(sellmatch), m
        ).send();  
            
        // Transfer EOS to seller
        
        action(
            permission_level{ _self, N(active) },
            TOKEN_CONTRACT, N(transfer),
            make_tuple(_self, account, asset(sold_eos, EOS), string("transfer")))
        .send();
    
            
        // Transfer Token to buyer
        
        action(
            permission_level{ _self, N(active) },
            token_contract, N(transfer),
            make_tuple(_self, itr->account, asset(sold_token, bid.symbol), string("transfer")))
        .send();
                
        // Erase the buy order from buy order table if the order has been took.
        if (itr->ask.amount == 0 || itr->bid.amount == 0) {
            itr = unit_price_index.erase(itr);
            if (bid.amount == 0 || ask.amount == 0) {
                return;
            }
        }        
        else {
            return;
        }
    }

    // The current order is not fully matched, publish the order
    publish_sellorder_if_needed(account, bid, ask);
}

void pomelo::cancelbuy(account_name account, string symbol, uint64_t id) {
    require_auth(account);

    auto buy_table = buyorders(_self, my_string_to_symbol(4, symbol.c_str()));    
    auto itr = buy_table.find(id); 
    
    eosio_assert(itr != buy_table.end(), "Trade id is not found");
    eosio_assert(itr->account == account || account == N(kyubeydex.bp), "Account does not match");

    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        make_tuple(_self, itr->account, itr->bid,
            std::string("trade cancel successed"))
    ).send();    

    buy_table.erase(itr);
}

void pomelo::cancelsell(account_name account, string symbol, uint64_t id) {
    require_auth(account);
    
    auto sell_table = sellorders(_self, my_string_to_symbol(4, symbol.c_str()));  
    auto itr = sell_table.find(id);
    
    eosio_assert(itr->account == account || account == N(kyubeydex.bp), "Account does not match");
    eosio_assert(itr->id == id, "Trade id is not found");

    action(
        permission_level{_self, N(active)},
        get_contract_name_by_symbol(symbol), N(transfer),
        make_tuple(_self, itr->account, itr->bid,
            std::string("trade cancel successed"))
    ).send();
    
    sell_table.erase(itr);
}



void pomelo::onTransfer(account_name from, account_name to, asset bid, std::string memo) 
{        
    
    if (to != _self) return;    
    require_auth(from);
    eosio_assert(bid.is_valid(), "invalid token transfer");
    eosio_assert(bid.amount > 0, "must bet a positive amount");

    auto splited_asset = split(memo, ' ');
    if (splited_asset.size() != 2) {
        return;
    }
    asset ask;
    ask.amount = string_to_amount(splited_asset[0]);
    ask.symbol = string_to_symbol(4, splited_asset[1].c_str());

    if (bid.symbol == EOS) {
        //eosio_assert(ask.symbol == S(4, "PXL"), "123");
        //eosio_assert(ask.symbol == string_to_symbol(4, "PXL"), "123");
        // note(minakokojima): S() and string_to_symbol is not equal?

        eosio_assert(ask.symbol != EOS, "Ask must be non-EOS");
        buy(from, bid, ask);
    }
    else
    {
        eosio_assert(ask.symbol == EOS, "Ask must be EOS");
        sell(from, bid, ask);
    }
}


void pomelo::transfer(account_name from, account_name to, asset bid, std::string memo) { 
    return;
}

void pomelo::setwhitelist(string symbol, account_name issuer) {
    require_auth(_self);
    whitelist w; w.contract = issuer;
    whitelist_index whitelist(_self, my_string_to_symbol(4, symbol.c_str()));
    whitelist.set(w, _self); 
}

void pomelo::rmwhitelist(string symbol) {
    require_auth(_self);
    whitelist_index whitelist(_self, my_string_to_symbol(4, symbol.c_str()));
    whitelist.remove();
}