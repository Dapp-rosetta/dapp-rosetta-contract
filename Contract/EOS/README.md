# Mannul

## Compile
/usr/local/eosio/bin/eosiocpp -g eoscrazytown/eoscrazytown.abi eoscrazytown/eoscrazytown.cpp
/usr/local/eosio/bin/eosiocpp -o eoscrazytown/eoscrazytown.wast eoscrazytown/eoscrazytown.cpp

## Deploy
cleos -u http://api-direct.eosasia.one set contract eosotcbackup eosotcbackup -p eosotcbackup@active


## Query
#cleos -u http://api-direct.eosasia.one get table -l 65536 eosotcbackup dacincubator order
#cleos -u http://api-direct.eosasia.one get table -l 65536 eosotcbackup youyouyouyou order
#cleos -u http://api-direct.eosasia.one get table -l 65536 eosotcbackup eosio.token order
#cleos -u http://api-direct.eosasia.one get table -l 65536 eosotcbackup eosio.token order

