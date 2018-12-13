# eosiocpp -o counter.wast  counter.cpp
# eosiocpp -g counter.abi counter.cpp
# Alias
# https://developers.eos.io/eosio-cpp/docs/aliasing-eosio-components
alias nodeos=/usr/local/eosio/bin/nodeos
alias cleos=/usr/local/eosio/bin/cleos
alias keosd=/usr/local/eosio/bin/keosd
alias eosiocpp=/usr/local/eosio/bin/eosiocpp

eosiocpp -o ./slot_machine.wast ./slot_machine.cpp
eosiocpp -g ./slot_machine.abi ./slot_machine.cpp

# cleos create account eosio happyeosslot EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4 EOS7ijWCBmoXBi3CgtK7DJxentZZeTkeUnaSDvyro9dq7Sd1C3dC4
# cleos set contract happyeosslot ./slot_machine -p happyeosslot@active