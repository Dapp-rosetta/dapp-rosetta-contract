pragma solidity ^0.4.24;
/// @author MinakoKojima (https://github.com/lychees)

import "openzeppelin-solidity/contracts/ownership/Ownable.sol";
import "openzeppelin-solidity/contracts/lifecycle/Pausable.sol";

contract Dex is Ownable {

  address public owner;
  mapping (address => bool) public admins;
  
  struct Order {
    address owner;
    uint256 price;
    address issuer;
    uint256 tokenId;      
  }  
  Order[] private orderBook;
  uint256 private orderBookSize;

  function DecentralizedExchange() public {
    owner = msg.sender;
    admins[owner] = true;    
  }

  /* Modifiers */
  modifier onlyOwner() {
    require(owner == msg.sender);
    _;
  }

  modifier onlyAdmins() {
    require(admins[msg.sender]);
    _;
  }

  /* Owner */
  function setOwner (address _owner) onlyOwner() public {
    owner = _owner;
  }

  function addAdmin (address _admin) onlyOwner() public {
    admins[_admin] = true;
  }

  function removeAdmin (address _admin) onlyOwner() public {
    delete admins[_admin];
  }

  /* Withdraw */
  function withdrawAll () onlyAdmins() public {
   msg.sender.transfer(address(this).balance);
  }

  function withdrawAmount (uint256 _amount) onlyAdmins() public {
    msg.sender.transfer(_amount);
  }

  /* ERC721 */

  function name() public pure returns (string _name) {
    return "dapdap.io";
  }
  
  /* Read */
  function isAdmin(address _admin) public view returns (bool _isAdmin) {
    return admins[_admin];
  }
  function totalOrder() public view returns (uint256 _totalOrder) {
    return orderBookSize;
  }
  function getOrder(uint256 _id) public view returns (address _owner, uint256 _price, address _issuer, uint256 _tokenId) {
    return (orderBook[_id].owner, orderBook[_id].price, orderBook[_id].issuer, orderBook[_id].tokenId);
  }
  
  /* Util */
  function isContract(address addr) internal view returns (bool) {
    uint size;
    assembly { size := extcodesize(addr) } // solium-disable-line
    return size > 0;
  }  
  
  /* Buy */
  
  function put(address _issuer, uint256 _tokenId, uint256 _price) public {
    Issuer issuer = Issuer(_issuer);
    require(issuer.ownerOf(_tokenId) == msg.sender);
    issuer.transferFrom(msg.sender, address(this), _tokenId);
    if (orderBookSize == orderBook.length) {
      orderBook.push(Order(msg.sender, _price, _issuer, _tokenId));
    } else {
      orderBook[orderBookSize] = Order(msg.sender, _price, _issuer, _tokenId);
    }
    orderBookSize += 1;
  }
  function buy(uint256 _id) public payable{
    require(_id < orderBookSize);
    require(msg.value >= orderBook[_id].price);
    require(!isContract(msg.sender));    
    orderBook[_id].owner.transfer(orderBook[_id].price*97/100); // 3% Cut-off
    if (msg.value > orderBook[_id].price) {
        msg.sender.transfer(msg.value - orderBook[_id].price);
    }
    Issuer issuer = Issuer(orderBook[_id].issuer);
    issuer.transfer(msg.sender, orderBook[_id].tokenId);    
    orderBook[_id] = orderBook[orderBookSize-1];
    orderBookSize -= 1;    
  }
  function revoke(uint256 _id) public {
    require(msg.sender == orderBook[_id].owner);
    Issuer issuer = Issuer(orderBook[_id].issuer);
    issuer.transfer(msg.sender, orderBook[_id].tokenId);    
    orderBook[_id] = orderBook[orderBookSize-1];
    orderBookSize -= 1;
  }
}

interface Issuer {
  function transferFrom(address _from, address _to, uint256 _tokenId) external;  
  function transfer(address _to, uint256 _tokenId) external;
  function ownerOf (uint256 _tokenId) external view returns (address _owner);
}