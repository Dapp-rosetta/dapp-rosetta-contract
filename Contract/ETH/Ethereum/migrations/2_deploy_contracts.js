// var Counter = artifacts.require("./Counter.sol");
// var DgameMakerToken = artifacts.require("./DgameMakerToken.sol");
// var TradeableToken = artifacts.require("./PonziToken/TradeableToken.sol");
// var TradeableToken2 = artifacts.require("./PonziToken/TradeableToken2.sol");
// var RandomCoin = artifacts.require("./RandomGetter/RandomCoin.sol");
// var ProfitableToken = artifacts.require("./PonziToken/ProfitableToken.sol");
var DeliberativeDemocracy = artifacts.require("./DeliberativeDemocracy.sol");
// var BecToken = artifacts.require("./BecToken/BecToken.sol");

module.exports = function(deployer) {
    // deployer.deploy(BecToken);
    // deployer.deploy(Counter);
    // deployer.deploy(DgameMakerToken);
    // deployer.deploy(TradeableToken);
    // deployer.deploy(TradeableToken2);
    // deployer.deploy(RandomCoin);
    // deployer.deploy(ProfitableToken); 
    deployer.deploy(DeliberativeDemocracy, 0x00);
};
