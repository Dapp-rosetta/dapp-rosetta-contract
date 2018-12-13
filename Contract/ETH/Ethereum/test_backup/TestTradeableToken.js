





var TradeableToken = artifacts.require("TradeableToken");
var TradeableToken2 = artifacts.require("TradeableToken2");

function adjust(a, b) {
    let t = 1;
    if (a < b) {
        if (a + t >= b) return b;
        return a;
    } else {
        if (a - t <= b) return b;
        return a;
    }   
}

contract('TradeableToken', function(accounts) {    
/*
    it("testzs", function(ccounts) {
        let A;
        let B;
        let b0;
        let b1;
        return TradeableToken.deployed().then(function(instance) {
            A = instance
            return TradeableToken.deployed();
        }).then(function(instance) {
            B = instance
            return A.buy({from: accounts[0], value: 50000, gas: "220009"});
        }).then(function() {
            return B.buy({from: accounts[1], value: 50000, gas: "220009"});
            return 0;
        }).then(function() {
            return A.balanceOf.call(accounts[0]);
        }).then(function(t) {
            b0 = t;
            return B.balanceOf.call(accounts[1]);
        }).then(function(t) {
            b1 = t;
            assert.equal(b0, b1, "Amount should be almost equal");   
        });
      });     */

    it("should be consistence, which means buy 10 ether should be equal with buy 5 ether 2 times", async () => {
        let A = await TradeableToken.deployed();
        let B = await TradeableToken2.deployed();
        
/*        let p0 = await A.buyPrice.call();          
        let p1 = await A.sellPrice.call();  
        console.log(p0, p1);      */

/*        await A.buy({from: accounts[0], value: 1});
        await B.buy({from: accounts[1], value: 1});
        p0 = await A.buyPrice.call();          
        p1 = await A.sellPrice.call();  
        console.log(p0, p1);      */
        await A.buy({from: accounts[0], value: 1e19});
        await B.buy({from: accounts[1], value: 5e18});
        await B.buy({from: accounts[1], value: 5e18});
//        p0 = await A.buyPrice.call();          
  //      p1 = await A.sellPrice.call();  
//        console.log(p0, p1);     

  //      await A.buy({from: accounts[0], value: 1e18});
    //    await B.buy({from: accounts[1], value: 1e18});        
       // let b = await B.balanceOf.call(accounts[1]);  
//        await B.buy({from: accounts[1], value: 500});        
        let b0 = await A.balanceOf.call(accounts[0]);
        let b1 = await B.balanceOf.call(accounts[1]);  
        console.log(b0, b1);
        //b0 = adjust(b0, b1);
        assert.equal(b0, b1, "Amount should be almost equal");       
    });
   
    /*
    it("should be almost equal after buy and sell", async () => {

        let tradeableToken = await TradeableToken.deployed();

        let contractEther = await web3.eth.getBalance(tradeableToken.address).toNumber();        

        let beginEther = await web3.eth.getBalance(accounts[0]).toNumber();
        let usedEther = beginEther / 2;
        let unusedEther = beginEther - usedEther;

        await tradeableToken.buy({from: accounts[0], value: usedEther});
        let tokenBalance = await tradeableToken.balanceOf.call(accounts[0]);
        // tokenBalance += 1;

        await tradeableToken.sell(tokenBalance, {from: accounts[0]});        
        let finalEther = await web3.eth.getBalance(accounts[0]).toNumber();
        let refundEther = finalEther - unusedEther;
        let consumingEther = usedEther - refundEther;

        let contractEther2 = await web3.eth.getBalance(tradeableToken.address).toNumber();        
    //    console.log(tradeableToken);

        console.log(beginEther);
        console.log(usedEther);
        console.log(refundEther);        
        console.log(unusedEther);        
        console.log(finalEther);
        console.log(consumingEther);

        console.log(contractEther);
        console.log(contractEther2);

        assert.isAtMost(refundEther, usedEther, "Amount should be almost equal");      
    });*/
});