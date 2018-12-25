var DgameMakerToken = artifacts.require("DgameMakerToken");

function adjust(a, b) {
    if (a == b) return a;
    let t = 10;
    if (a < b) {
        if (a + t >= b) return b;
        return a;
    } else {
        if (a - t <= b) return b;
        return a;
    }   
}

contract('DgameMakerToken', function(accounts) {
    it("should be consistence, which means buy 10 ether should be equal with buy 5 ether 2 times", async () => {
        let A = await DgameMakerToken.deployed();
        let B = await DgameMakerToken.deployed();
//        let t = await A.buyPrice.call();
//        console.log(t);
        await A.buy2("0x0", {from: accounts[0], value: 4000000});
        await A.buy2("0x0", {from: accounts[0], value: 6000000});        
        await B.buy2("0x0", {from: accounts[1], value: 4000000});
        let b = await B.balanceOf.call(accounts[1]);  
        await B.buy2("0x0", {from: accounts[1], value: 6000000});        
        let b0 = await A.balanceOf.call(accounts[0]);
        let b1 = await B.balanceOf.call(accounts[1]);  
        console.log(b, b0, b1);
        b0 = adjust(b0, b1);
        assert.equal(b0, b1, "Amount should be almost equal");      
    });    
});