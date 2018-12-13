var Counter = artifacts.require("Counter.sol");

contract('Counter', async function(accounts) {
    it("should equal to 0 at the beginning", async function() {
        var instance =  await Counter.deployed() 
        var counter = await instance.get.call();
        assert.equal(counter.valueOf(), 0, "not equal to 0 at the beginning");
    });   
    it("should equal to 2 after 2 inc() operations", async function() {
        var instance = await Counter.deployed()
        await instance.inc();
        await instance.inc();
        var counter = await instance.get.call();
        assert.equal(counter.toNumber(), 2, "should equal to 2 after 2 inc() operations which is" + counter.valueOf());
    });
});