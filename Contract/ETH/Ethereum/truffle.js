/*
 * NB: since truffle-hdwallet-provider 0.0.5 you must wrap HDWallet providers in a 
 * function when declaring them. Failure to do so will cause commands to hang. ex:
 * ```
 * mainnet: {
 *     provider: function() { 
 *       return new HDWalletProvider(mnemonic, 'https://mainnet.infura.io/<infura-key>') 
 *     },
 *     network_id: '1',
 *     gas: 4500000,
 *     gasPrice: 10000000000,
 *   },
 */

var HDWalletProvider = require("truffle-hdwallet-provider");
var mnemonic = "rookie ginger butter initial seat economy gentle detect amount hockey dumb asthma";

module.exports = {
  // See <http://truffleframework.com/docs/advanced/configuration>
  // to customize your Truffle configuration!
  networks: {
    /*
    development: {
      host: "127.0.0.1",
      port: 8545,
      network_id: "*" // Match any network id
    },*/
    ropsten: {
      provider: function() {
        return new HDWalletProvider(mnemonic, "https://ropsten.infura.io/v3/252df3eadab74e84a9e5635f8a401e11252df3eadab74e84a9e5635f8a401e11")
      },
      network_id: 3,
      gas: 4500000,
      gasPrice: 10000000000,    
    }   
  } 
};