pragma solidity ^0.4.24;

import "openzeppelin-solidity/contracts/lifecycle/Pausable.sol";

contract DeliberativeDemocracy is Pausable  {

    struct Proposal {
        address advocate; // 发起人
        uint256 act; 
        uint256 startTime;
        uint256 endTime;
    }

    address public clientContractAddress;

    // 既然一token一票，某一个时刻只能投一个节点，那就直接用balances[]
    // mapping(address => uint256) public unused_balance; // 未使用的投票
    // mapping(address => uint256) public used_balance; // 使用的投票  
    mapping(address => bool) public isUsed; // 用户是否已经投票
    mapping(address => address) public candidate; // 我的候选人
    mapping(address => uint256) public ticket; // 得票数
    mapping(address => uint256) public lastVoteTime; // 上次投票时间
    address[21] mps;

    mapping(uint256 => address) public senate; // 议会 
    mapping(address => bool) public isMP;
    // mapping(uint256 => Proposal) public proposals; // 提案
    Proposal[] proposals;

    constructor(address client) public {
        clientContractAddress = client;
    }

    function setClientAddress(address _client) public onlyAdmins {
        clientContractAddress = _client;
    }

    function revote() public {
        if(isUsed[msg.sender]) {
            Client _client = Client(clientContractAddress);

            ticket[candidate[msg.sender]] -= _client.balanceOf(msg.sender);
            isUsed[msg.sender] = false;   
        }
    }

    function vote(address _candidate) public {
        require(isMP[_candidate]);
        revote();
        candidate[msg.sender] = _candidate;

        Client _client = Client(clientContractAddress);
        ticket[_candidate] += _client.balanceOf(msg.sender);

        isUsed[msg.sender] = true;

        // 维护21个议员
        
    }

    /*
    function challenge(address _voter) public {

    }

    function run() public {

    }

    function proposal() public {

    }

    function examine() public {

    }
    */
    function run() public {
        isMP[msg.sender] = true;
    }

    function execute(string method, uint256 para) public returns(bool){
        bytes4 methodId = bytes4(keccak256(method));
        return clientContractAddress.call(methodId, para);
        //if (!clientContractAddress.call(method, para)) {
        //    revert();
        //}
    }

    function withdraw() public {
       // Issuer issuer = Issuer(IssuerContractAddress);
       // issuer.transfer(msg.sender, unused_balance[msg.sender]);
    }

    function recharge(uint256 _value) public {
       // Issuer issuer = Issuer(IssuerContractAddress);
       // issuer.transferFrom(msg.sender, address(this), _value);
       // unused_balance[msg.sender] += _value;        
    }
}

interface Client {
    function transferFrom(address _from, address _to, uint256 _value) external returns(bool);  
    function transfer(address _to, uint256 _value) external returns(bool);
    function balanceOf(address _owner) external view returns (uint256);
}
