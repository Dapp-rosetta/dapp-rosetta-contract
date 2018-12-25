var Counter = artifacts.require("RandomCoin");

contract('Counter', async function(accounts) {
    it("should equal to 0 at the beginning", async function() {
        var instance =  await Counter.deployed() 
        var counter = await instance.settleBet.call(accounts[0]);
        console.log("ccccc"+counter);
        // assert.equal(counter.valueOf(), 0, "not equal to 0 at the beginning");
    });   
});