pragma solidity ^0.4.24;

import "./TradeableToken.sol";


/**
 * @title Tradeable Token
 * @dev Standard Token which can be trade within the contract.
 * we support buy() and sell() function in a simpilified bancor algorithm which CW = 50%.
 */
contract ReferableToken is TradeableToken {

    uint256 public fee = 10;

    /**
    * @dev Buy some token
    */
    function buy() public payable {
//        uint256 fee = msg.value * fee / 100;
  //      _buy(msg.value);
    }
}
