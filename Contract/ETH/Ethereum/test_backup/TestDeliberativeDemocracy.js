var PowhToken = artifacts.require("PowhToken");
var DeliberativeDemocracy = artifacts.require("DeliberativeDemocracy");

contract('PowhToken', function(accounts) {
    contract('DeliberativeDemocracy', function(accounts) {        
        it("Let the hunt begin.", async () => {
            let powhToken = await PowhToken.deployed();           
            let powhTokenAddress = await powhToken.address;
            let deliberativeDemocracy = await DeliberativeDemocracy.deployed();
            await deliberativeDemocracy.setClientAddress(powhTokenAddress,  {from: accounts[0]});
            
            let deliberativeDemocracyAddress = await deliberativeDemocracy.address;
            console.log(deliberativeDemocracyAddress);

            let minReferrerBonus = await powhToken.minReferrerBonus.call();
            let maxReferrerBonus = await powhToken.maxReferrerBonus.call();

            console.log(minReferrerBonus);
            console.log(maxReferrerBonus);            
            
            console.log(await powhToken.administrators.call(deliberativeDemocracyAddress));
            await powhToken.setAdministrator(deliberativeDemocracyAddress, true);
            console.log(await powhToken.administrators.call(deliberativeDemocracyAddress));
            console.log(await deliberativeDemocracy.clientContractAddress.call());
            console.log(await deliberativeDemocracy.execute("setMaxReferrerBonus(uint256)", 25));
            maxReferrerBonus = await powhToken.maxReferrerBonus.call();
            console.log(maxReferrerBonus);
        });
    })
});