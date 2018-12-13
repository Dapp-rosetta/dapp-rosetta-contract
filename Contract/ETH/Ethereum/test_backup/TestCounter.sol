pragma solidity ^0.4.24;

import "truffle/Assert.sol";
import "truffle/DeployedAddresses.sol";
import "../contracts/Counter.sol";

contract TestCounter {
    function test() public {
        Counter counter = new Counter();
        Assert.equal(counter.get(), expected, "Counter should equal to 0 at the beginning");
        uint expected = 2;
        counter.inc();
        counter.inc();
        Assert.equal(counter.get(), expected, "Counter should equal to 2 after inc");
    } 
}
