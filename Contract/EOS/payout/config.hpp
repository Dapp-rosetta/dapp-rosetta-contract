#pragma once
#include <eosiolib/asset.hpp>
const auto EOS_CONTRACT = "eosio.token"_n;
const auto EOS_SYMBOL = eosio::symbol("EOS", 4);
const auto TOKEN_CONTRACT = "dacinbator"_n;
const auto TOKEN_SYMBOL = eosio::symbol("CMU", 4);
const auto REFUND_DELAY = 60;
typedef unsigned __int128 uint128_t;

const uint128_t MAGNITUDE = 1ll<<32;