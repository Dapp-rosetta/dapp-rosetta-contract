pragma solidity ^0.4.24;

/**
 * @title SafeMath
 * @dev Math operations with safety checks that revert on error
 */
library SafeMath {

    /**
    * @dev Multiplies two numbers, reverts on overflow.
    */
    function mul(uint256 _a, uint256 _b) internal pure returns (uint256) {
        // Gas optimization: this is cheaper than requiring 'a' not being zero, but the
        // benefit is lost if 'b' is also tested.
        // See: https://github.com/OpenZeppelin/openzeppelin-solidity/pull/522
        if (_a == 0) {
            return 0;
        }

        uint256 c = _a * _b;
        require(c / _a == _b);

        return c;
    }

    /**
    * @dev Integer division of two numbers truncating the quotient, reverts on division by zero.
    */
    function div(uint256 _a, uint256 _b) internal pure returns (uint256) {
        require(_b > 0); // Solidity only automatically asserts when dividing by 0
        uint256 c = _a / _b;
        // assert(_a == _b * c + _a % _b); // There is no case in which this doesn't hold

        return c;
    }

    /**
    * @dev Subtracts two numbers, reverts on overflow (i.e. if subtrahend is greater than minuend).
    */
    function sub(uint256 _a, uint256 _b) internal pure returns (uint256) {
        require(_b <= _a);
        uint256 c = _a - _b;

        return c;
    }

    /**
    * @dev Adds two numbers, reverts on overflow.
    */
    function add(uint256 _a, uint256 _b) internal pure returns (uint256) {
        uint256 c = _a + _b;
        require(c >= _a);

        return c;
    }
}


/**
 * @title AddressUtils
 * @dev Utility library of inline functions on addresses
 */
library AddressUtils {

    /**
     * Returns whether the target address is a contract
     * @dev This function will return false if invoked during the constructor of a contract,
     * as the code is not actually created until after the constructor finishes.
     * @param addr address to check
     * @return whether the target address is a contract
     */
    function isContract(address addr) 
        internal 
        view 
        returns (bool) 
    {
        uint256 size;
        /// @dev XXX Currently there is no better way to check if there is 
        // a contract in an address than to check the size of the code at that address.
        // See https://ethereum.stackexchange.com/a/14016/36603
        // for more details about how this works.
        // TODO Check this again before the Serenity release, because all addresses will be
        // contracts then.
        // solium-disable-next-line security/no-inline-assembly
        assembly { size := extcodesize(addr) }
        return size > 0;
    }
}

/**
 * @title ERC20 interface
 * @dev see https://github.com/ethereum/EIPs/issues/20
 */
contract ERC20 {
    function totalSupply() public view returns (uint256);

    function balanceOf(address _who) public view returns (uint256);

    function allowance(address _owner, address _spender)
        public view returns (uint256);

    function transfer(address _to, uint256 _value) public returns (bool);

    function approve(address _spender, uint256 _value)
        public returns (bool);

    function transferFrom(address _from, address _to, uint256 _value)
        public returns (bool);

    event Transfer(
        address indexed from,
        address indexed to,
        uint256 value
    );

    event Approval(
        address indexed owner,
        address indexed spender,
        uint256 value
    );
}


/**
 * @title Standard ERC20 token
 *
 * @dev Implementation of the basic standard token.
 * https://github.com/ethereum/EIPs/issues/20
 * Based on code by FirstBlood: https://github.com/Firstbloodio/token/blob/master/smart_contract/FirstBloodToken.sol
 */
contract StandardToken is ERC20 {
    using SafeMath for uint256;

    mapping(address => uint256) balances;

    mapping (address => mapping (address => uint256)) internal allowed;

    uint256 totalSupply_;

    /**
    * @dev Total number of tokens in existence
    */
    function totalSupply() public view returns (uint256) {
        return totalSupply_;
    }

    /**
    * @dev Gets the balance of the specified address.
    * @param _owner The address to query the the balance of.
    * @return An uint256 representing the amount owned by the passed address.
    */
    function balanceOf(address _owner) public view returns (uint256) {
        return balances[_owner];
    }

    /**
    * @dev Function to check the amount of tokens that an owner allowed to a spender.
    * @param _owner address The address which owns the funds.
    * @param _spender address The address which will spend the funds.
    * @return A uint256 specifying the amount of tokens still available for the spender.
    */
    function allowance(
        address _owner,
        address _spender
    )
        public
        view
        returns (uint256)
    {
        return allowed[_owner][_spender];
    }

    /**
    * @dev Transfer token for a specified address
    * @param _to The address to transfer to.
    * @param _value The amount to be transferred.
    */
    function transfer(address _to, uint256 _value) public returns (bool) {
        require(_value <= balances[msg.sender]);
        require(_to != address(0));

        balances[msg.sender] = balances[msg.sender].sub(_value);
        balances[_to] = balances[_to].add(_value);
        emit Transfer(msg.sender, _to, _value);
        return true;
    }

    /**
    * @dev Approve the passed address to spend the specified amount of tokens on behalf of msg.sender.
    * Beware that changing an allowance with this method brings the risk that someone may use both the old
    * and the new allowance by unfortunate transaction ordering. One possible solution to mitigate this
    * race condition is to first reduce the spender's allowance to 0 and set the desired value afterwards:
    * https://github.com/ethereum/EIPs/issues/20#issuecomment-263524729
    * @param _spender The address which will spend the funds.
    * @param _value The amount of tokens to be spent.
    */
    function approve(address _spender, uint256 _value) public returns (bool) {
        allowed[msg.sender][_spender] = _value;
        emit Approval(msg.sender, _spender, _value);
        return true;
    }

    /**
    * @dev Transfer tokens from one address to another
    * @param _from address The address which you want to send tokens from
    * @param _to address The address which you want to transfer to
    * @param _value uint256 the amount of tokens to be transferred
    */
    function transferFrom(
        address _from,
        address _to,
        uint256 _value
    )
        public
        returns (bool)
    {
        require(_value <= balances[_from]);
        require(_value <= allowed[_from][msg.sender]);
        require(_to != address(0));

        balances[_from] = balances[_from].sub(_value);
        balances[_to] = balances[_to].add(_value);
        allowed[_from][msg.sender] = allowed[_from][msg.sender].sub(_value);
        emit Transfer(_from, _to, _value);
        return true;
    }

    /**
    * @dev Increase the amount of tokens that an owner allowed to a spender.
    * approve should be called when allowed[_spender] == 0. To increment
    * allowed value is better to use this function to avoid 2 calls (and wait until
    * the first transaction is mined)
    * From MonolithDAO Token.sol
    * @param _spender The address which will spend the funds.
    * @param _addedValue The amount of tokens to increase the allowance by.
    */
    function increaseApproval(
        address _spender,
        uint256 _addedValue
    )
        public
        returns (bool)
    {
        allowed[msg.sender][_spender] = (
        allowed[msg.sender][_spender].add(_addedValue));
        emit Approval(msg.sender, _spender, allowed[msg.sender][_spender]);
        return true;
    }

    /**
    * @dev Decrease the amount of tokens that an owner allowed to a spender.
    * approve should be called when allowed[_spender] == 0. To decrement
    * allowed value is better to use this function to avoid 2 calls (and wait until
    * the first transaction is mined)
    * From MonolithDAO Token.sol
    * @param _spender The address which will spend the funds.
    * @param _subtractedValue The amount of tokens to decrease the allowance by.
    */
    function decreaseApproval(
        address _spender,
        uint256 _subtractedValue
    )
        public
        returns (bool)
    {
        uint256 oldValue = allowed[msg.sender][_spender];
        if (_subtractedValue >= oldValue) {
            allowed[msg.sender][_spender] = 0;
        } else {
            allowed[msg.sender][_spender] = oldValue.sub(_subtractedValue);
        }
        emit Approval(msg.sender, _spender, allowed[msg.sender][_spender]);
        return true;
    }

}


/**
 * ________                                    _____          __                 
 * \______ \    _________    _____   ____     /     \ _____  |  | __ ___________ 
 * |    |  \  / ___\__  \  /     \_/ __ \   /  \ /  \\__  \ |  |/ // __ \_  __ \
 * |    `   \/ /_/  > __ \|  Y Y  \  ___/  /    Y    \/ __ \|    <\  ___/|  | \/
 * /_______  /\___  (____  /__|_|  /\___  > \____|__  (____  /__|_ \\___  >__|   
 *         \//_____/     \/      \/     \/          \/     \/     \/    \/       
 * Powered by Andoromeda 
 */

/**
 * @title Dgame Maker Token
 * @dev Dgame Maker Token which can be trade in the contract.
 * we support buy() and sell() function in a simpilified 50% CW bancor algorithm.
 */
contract PowhToken is StandardToken {
    using AddressUtils for address;

    /*=================================
    =            MODIFIERS            =
    =================================*/
    // only people with tokens
    modifier onlyBagholders(address _addr) {
        require(ownedTokens(_addr) > 0);
        _;
    }
    
    // only people with profits
    modifier onlyStronghands(address _addr) {
        require(ownedDividends(_addr, true) > 0);
        _;
    }
    
    // administrators can:
    // -> change the name of the contract
    // -> change the name of the token
    // -> change the PoS difficulty (How many tokens it costs to hold a masternode, in case it gets crazy high later)
    // they CANNOT:
    // -> take funds
    // -> disable withdrawals
    // -> kill the contract
    // -> change the price of tokens
    modifier onlyAdministrator(){
        address _customerAddress = msg.sender;
        require(administrators[keccak256(abi.encodePacked(_customerAddress))]);
        _;
    }
    
    /*==============================
    =            EVENTS            =
    ==============================*/
    event onTokenPurchase(
        address indexed customerAddress,
        uint256 incomingEthereum,
        uint256 tokensMinted,
        address indexed referredBy
    );
    
    event onTokenSell(
        address indexed customerAddress,
        uint256 tokensBurned,
        uint256 ethereumEarned
    );
    
    event onReinvestment(
        address indexed customerAddress,
        uint256 ethereumReinvested,
        uint256 tokensMinted
    );
    
    event onWithdraw(
        address indexed customerAddress,
        uint256 ethereumWithdrawn
    );

    event ChangeDividendFee(
        uint256 before,
        uint now_
    );

    event ChangeCommunityFee(
        uint256 before,
        uint256 now_
    );

    event ChangeIncremental(
        uint256 before,
        uint256 now_
    );

    event ChangeContractAddr(
        address _addr
    );

    // 锁仓期间只许买，也允许转账，不许卖
    event Lock(
        uint256 beginTime,
        uint256 endTime
    );

    // 修改额度
    event ChangeLockAmount(
        uint256 before,
        uint256 now_
    );
    
      
    /*=====================================
    =            CONFIGURABLES            =
    =====================================*/
    string public name = "Dgame Maker";
    string public symbol = "DGM";
    uint256 constant public decimals = 18;
    uint256 internal dividendFee_ = 20;
    uint256 internal communityFee_ = 50;    
    uint256 constant internal tokenPriceInitial_ = 0.0000001 ether;
    uint256 internal tokenPriceIncremental_ = 0.00000001 ether;
    uint256 constant internal magnitude = 2**64;
    
    
   /*================================
    =            DATASETS            =
    ================================*/
    // amount of shares for each address (scaled number)
    mapping(address => uint256) internal referralBalance_;
    mapping(address => int256) internal payoutsTo_;
    uint256 internal profitPerShare_;
    uint256 internal communityFeeTo_ = 0; // admins can withdraw amount;
    
    // administrator list (see above on what they can do)
    mapping(bytes32 => bool) public administrators;
    bool public isLocked;
    uint256 endTime;

    // 用户当前锁定了多少token
    mapping(address => uint256) public lockToken;
    // 一次提案需要锁定多少token
    uint256 lockAmount;
    

    address public god;


    /*=======================================
    =            PUBLIC FUNCTIONS            =
    =======================================*/
    /*
    * -- APPLICATION ENTRY POINTS --  
    */
    constructor(uint256 _dayLock)
        public
    {
        // add administrators here
        administrators[keccak256(abi.encodePacked(msg.sender))] = true;
        uint256 _lastTime = _dayLock * 1 days;
        endTime = now + _lastTime;
        isLocked = true; 
        totalSupply_ = 0;
        lockAmount = 100 ether;

        emit Lock(now, endTime);
    }
    
     
    /**
     * Converts all incoming ethereum to tokens for the caller, and passes down the referral addy (if any)
     */
    function buy(address _referredBy)
        public
        payable
        returns(uint256)
    {
        purchaseTokens(msg.value, _referredBy);
    }
    
    /**
     * Fallback function to handle ethereum that was send straight to the contract
     * Unfortunately we cannot use a referral address this way.
     */
    function()
        payable
        public
    {
        purchaseTokens(msg.value, 0x0);
    }
    
    /**
     * Converts all of caller's dividends to tokens.
     */
    function reinvest()
        onlyStronghands(msg.sender)
        public
    {
        // fetch dividends
        uint256 _dividends = ownedDividends(msg.sender, false); // retrieve ref. bonus later in the code
        
        // pay out the dividends virtually
        address _customerAddress = msg.sender;
        payoutsTo_[_customerAddress] +=  (int256) (_dividends * magnitude);
        
        // retrieve ref. bonus
        _dividends += referralBalance_[_customerAddress];
        referralBalance_[_customerAddress] = 0;
        
        // dispatch a buy order with the virtualized "withdrawn dividends"
        uint256 _tokens = purchaseTokens(_dividends, 0x0);
        
        // fire event
        emit onReinvestment(_customerAddress, _dividends, _tokens);
    }
    
    /**
     * Alias of sell() and withdraw().
     */
    function exit()
        public
    {
        // get token count for caller & sell them all
        address _customerAddress = msg.sender;
        uint256 _tokens = balances[_customerAddress].sub(lockToken[_customerAddress]);
        if(_tokens > 0) sell(_tokens);
        
        // lambo delivery service
        withdraw();
    }

    function withdraw ()
        public
    {
        require(isLocked == false);

        withdrawBasic(msg.sender);
    }

    /**
     * Withdraws all of the callers earnings.
     */
    function withdrawBasic(address _addr)
        onlyStronghands(_addr)
        internal
    {
        // setup data
        address _customerAddress = _addr;
        uint256 _dividends = ownedDividends(_customerAddress, false); // get ref. bonus later in the code
        
        // update dividend tracker
        payoutsTo_[_customerAddress] +=  (int256) (_dividends * magnitude);
        
        // add ref. bonus
        _dividends += referralBalance_[_customerAddress];
        referralBalance_[_customerAddress] = 0;
        
        // lambo delivery service
        _customerAddress.transfer(_dividends);
        
        // fire event
        emit onWithdraw(_customerAddress, _dividends);
    }
    

    /**
     * @dev withdraw communityFee_
     */
    function withdrawCommunity(uint256 _amount)
        public
        onlyAdministrator
    {
        require(_amount <= communityFeeTo_);
        require(isLocked == false);

        msg.sender.transfer(_amount);
        communityFeeTo_ = communityFeeTo_.sub(_amount);
    }


    /**
     * Liquifies tokens to ethereum.
     */
    function sell(uint256 _amountOfTokens)
        onlyBagholders(msg.sender)
        public
    {
        require(isLocked == false);
        // setup data
        address _customerAddress = msg.sender;
        // russian hackers BTFO
        require(_amountOfTokens <= balances[_customerAddress].sub(lockToken[_customerAddress]));
        uint256 _tokens = _amountOfTokens;
        uint256 _ethereum = tokensToEthereum_(_tokens);
        uint256 _dividends = SafeMath.div(_ethereum, dividendFee_);
        uint256 _taxedEthereum = SafeMath.sub(_ethereum, _dividends);
        
        // burn the sold tokens
        totalSupply_ = SafeMath.sub(totalSupply_, _tokens);
        balances[_customerAddress] = SafeMath.sub(balances[_customerAddress], _tokens);
        
        communityFeeTo_ = SafeMath.add(communityFeeTo_, _dividends);

        // update dividends tracker
        int256 _updatedPayouts = (int256) (profitPerShare_ * _tokens + (_taxedEthereum * magnitude));
        payoutsTo_[_customerAddress] -= _updatedPayouts;       
        
        // dividing by zero is a bad idea
        /*
        if (totalSupply_ > 0) {
            // update the amount of dividends per token
            profitPerShare_ = SafeMath.add(profitPerShare_, (_dividends * magnitude) / totalSupply_);
        }
        */
        
        // fire event
        emit onTokenSell(_customerAddress, _tokens, _taxedEthereum);
    }
    
    
    /**
     * Transfer tokens from the caller to a new holder.
     * Remember, there's a 10% fee here as well.
     */
    function transfer(address _toAddress, uint256 _amountOfTokens)
        onlyBagholders(msg.sender)
        public
        returns(bool)
    {        
        // setup
        address _customerAddress = msg.sender;

        require(balanceOf(_customerAddress).sub(lockToken[_customerAddress]) > _amountOfTokens);
        
        // withdraw all outstanding dividends first
        if(ownedDividends(msg.sender, true) > 0) withdraw();
        
        // liquify 10% of the tokens that are transfered
        // these are dispersed to shareholders
        uint256 _tokenFee = SafeMath.div(_amountOfTokens, dividendFee_);
        uint256 _taxedTokens = SafeMath.sub(_amountOfTokens, _tokenFee);
        uint256 _dividends = tokensToEthereum_(_tokenFee);
  
        // burn the fee tokens
        totalSupply_ = SafeMath.sub(totalSupply_, _tokenFee);

        // exchange tokens
        super.transfer(_toAddress, _amountOfTokens);

        // update dividend trackers
        payoutsTo_[_customerAddress] -= (int256) (profitPerShare_ * _amountOfTokens);
        payoutsTo_[_toAddress] += (int256) (profitPerShare_ * _taxedTokens);
        
        // disperse dividends among holders
        profitPerShare_ = SafeMath.add(profitPerShare_, (_dividends * magnitude) / totalSupply_);
        
        // fire event
        emit Transfer(_customerAddress, _toAddress, _taxedTokens);
        
        // ERC20
        return true;
       
    }

    /**
     * Transfer tokens from the holder to a new holder.
     * Remember, there's a 10% fee here as well.
     */
    function transferFrom(address _fromAddress, address _toAddress, uint256 _amountOfTokens)
        onlyBagholders(msg.sender)
        public
        returns(bool)
    {        
        // setup
        address _customerAddress = _fromAddress;
        require(balanceOf(_customerAddress).sub(lockToken[_customerAddress]) > _amountOfTokens);

        
        // withdraw all outstanding dividends first
        if(ownedDividends(_customerAddress, true) > 0) withdrawBasic(_customerAddress);
        
        // liquify 10% of the tokens that are transfered
        // these are dispersed to shareholders
        uint256 _tokenFee = SafeMath.div(_amountOfTokens, dividendFee_);
        uint256 _taxedTokens = SafeMath.sub(_amountOfTokens, _tokenFee);
        uint256 _dividends = tokensToEthereum_(_tokenFee);
  
        // burn the fee tokens
        totalSupply_ = SafeMath.sub(totalSupply_, _tokenFee);

        // exchange tokens
        super.transferFrom(_fromAddress, _toAddress, _amountOfTokens);

        // update dividend trackers
        payoutsTo_[_customerAddress] -= (int256) (profitPerShare_ * _amountOfTokens);
        payoutsTo_[_toAddress] += (int256) (profitPerShare_ * _taxedTokens);
        
        // disperse dividends among holders
        profitPerShare_ = SafeMath.add(profitPerShare_, (_dividends * magnitude) / totalSupply_);
        
        // fire event
        emit Transfer(_customerAddress, _toAddress, _taxedTokens);
        
        // ERC20
        return true;
       
    }

    function unLock() 
        public
    {
        require(endTime < now);

        isLocked = false;
        endTime = 0;
    }
    
    /*----------  ADMINISTRATOR ONLY FUNCTIONS  ----------*/  
    /**
     * In case one of us dies, we need to replace ourselves.
     */
    function setAdministrator(bytes32 _identifier, bool _status)
        onlyAdministrator()
        public
    {
        administrators[_identifier] = _status;
    }
    
    
    /**
     * If we want to rebrand, we can.
     */
    function setName(string _name)
        onlyAdministrator()
        public
    {
        name = _name;
    }
    
    /**
     * If we want to rebrand, we can.
     */
    function setSymbol(string _symbol)
        onlyAdministrator()
        public
    {
        symbol = _symbol;
    }

    function setContractAddr(address _addr)
        public
        onlyAdministrator
    {
        require(_addr.isContract());

        god = _addr;
        emit ChangeContractAddr(_addr);
    }

    function setDividendFee(uint256 _dividendFee) 
        public
    {
        require(msg.sender == god);

        uint256 before = dividendFee_;
        dividendFee_ = _dividendFee;

        emit ChangeDividendFee(before, dividendFee_);
    }

    function setCommunityFee(uint256 _communityFee)
        public
    {
        require(msg.sender == god);

        uint256 before = communityFee_;
        communityFee_ = _communityFee;

        emit ChangeCommunityFee(before, communityFee_);
    }

    function setIncremental(uint256 _incremental) 
        public
    {
        require(msg.sender == god);

        uint256 before = tokenPriceIncremental_;
        tokenPriceIncremental_ = _incremental;

        emit ChangeIncremental(before, tokenPriceIncremental_);
    }

    function setLockAmount(uint256 _amount)
        public
    {
        require(msg.sender == god);

        uint256 before = lockAmount;
        lockAmount = _amount;

        emit ChangeLockAmount(before, lockAmount);
    }

    function lock(uint256 _lastTime)
        public
    {
        require(isLocked == false);
        require(msg.sender == god);
        
        endTime = now + _lastTime * 1 days;
        isLocked = true;

        emit Lock(now, endTime);
    }

    function lockTokenAdd(address _addr)
        public
    {
        require(msg.sender == god);
        require(balanceOf(_addr) >= lockToken[_addr].add(lockAmount));

        lockToken[_addr] = lockToken[_addr].add(lockAmount);
    }
        
    function lockTokenSub(address _addr)
        public
    {
        require(msg.sender == god);
        require(lockToken[_addr] >= lockAmount);

        lockToken[_addr] = lockToken[_addr].sub(lockAmount);
    }

    /*----------  HELPERS AND CALCULATORS  ----------*/
    /**
     * Method to view the current Ethereum stored in the contract
     * Example: totalEthereumBalance()
     */
    function totalEthereumBalance()
        public
        view
        returns(uint)
    {
        return address(this).balance;
    }
    
    /**
     * Retrieve the tokens owned by the caller.
     */
    function ownedTokens(address _addr)
        public
        view
        returns(uint256)
    {
        address _customerAddress = _addr;
        return balanceOf(_customerAddress);
    }
    
    function lockTokens(address _addr)
        public
        view
        returns(uint256)
    {
        return lockToken[_addr];
    }

    function lockAmount_() 
        public
        view
        returns(uint256)
    {
        return lockAmount;
    }


    /**
     * Retrieve the dividends owned by the caller.
     * If `_includeReferralBonus` is to to 1/true, the referral bonus will be included in the calculations.
     * The reason for this, is that in the frontend, we will want to get the total divs (global + ref)
     * But in the internal calculations, we want them separate. 
     */ 
    function ownedDividends(address _addr, bool _includeReferralBonus) 
        public 
        view 
        returns(uint256)
    {
        address _customerAddress = _addr;
        return _includeReferralBonus ? dividendsOf(_customerAddress) + referralBalance_[_customerAddress] : dividendsOf(_customerAddress) ;
    }
    
    /**
     * Retrieve the dividend balance of any single address.
     */
    function dividendsOf(address _customerAddress)
        view
        public
        returns(uint256)
    {
        return (uint256) ((int256)(profitPerShare_ * balances[_customerAddress]) - payoutsTo_[_customerAddress]) / magnitude;
    }
    
    /**
     * Return the buy price of 1 individual token.
     */
    function sellPrice() 
        public 
        view 
        returns(uint256)
    {
        // our calculation relies on the token supply, so we need supply. Doh.
        if(totalSupply_ == 0){
            return tokenPriceInitial_ - tokenPriceIncremental_;
        } else {
            uint256 _ethereum = tokensToEthereum_(1e18);
            uint256 _dividends = SafeMath.div(_ethereum, dividendFee_  );
            uint256 _taxedEthereum = SafeMath.sub(_ethereum, _dividends);
            return _taxedEthereum;
        }
    }
    
    /**
     * Return the sell price of 1 individual token.
     */
    function buyPrice() 
        public 
        view 
        returns(uint256)
    {
        // our calculation relies on the token supply, so we need supply. Doh.
        if(totalSupply_ == 0){
            return tokenPriceInitial_ + tokenPriceIncremental_;
        } else {
            uint256 _ethereum = tokensToEthereum_(1e18);
            uint256 _dividends = SafeMath.div(_ethereum, dividendFee_  );
            uint256 _taxedEthereum = SafeMath.add(_ethereum, _dividends);
            return _taxedEthereum;
        }
    }
    
    /**
     * Function for the frontend to dynamically retrieve the price scaling of buy orders.
     */
    function calculateTokensReceived(uint256 _ethereumToSpend) 
        public 
        view 
        returns(uint256)
    {
        uint256 _dividends = SafeMath.div(_ethereumToSpend, dividendFee_);
        uint256 _taxedEthereum = SafeMath.sub(_ethereumToSpend, _dividends);
        uint256 _amountOfTokens = ethereumToTokens_(_taxedEthereum);
        
        return _amountOfTokens;
    }
    
    /**
     * Function for the frontend to dynamically retrieve the price scaling of sell orders.
     */
    function calculateEthereumReceived(uint256 _tokensToSell) 
        public 
        view 
        returns(uint256)
    {
        require(_tokensToSell <= totalSupply_);
        uint256 _ethereum = tokensToEthereum_(_tokensToSell);
        uint256 _dividends = SafeMath.div(_ethereum, dividendFee_);
        uint256 _taxedEthereum = SafeMath.sub(_ethereum, _dividends);
        return _taxedEthereum;
    }
    
    
    /*==========================================
    =            INTERNAL FUNCTIONS            =
    ==========================================*/
    function purchaseTokens(uint256 _incomingEthereum, address _referredBy)
        internal
        returns(uint256)
    {
        // data setup
        address _customerAddress = msg.sender;
        uint256 _undividedDividends = SafeMath.div(_incomingEthereum, dividendFee_);
        uint256 _referralBonus = SafeMath.div(_undividedDividends, 3);
        uint256 _dividends = SafeMath.sub(_undividedDividends, _referralBonus);
        uint256 _taxedEthereum = SafeMath.sub(_incomingEthereum, _undividedDividends);
        uint256 _amountOfTokens = ethereumToTokens_(_taxedEthereum);
        uint256 _fee = _dividends * magnitude;
 
        // no point in continuing execution if OP is a poorfag russian hacker
        // prevents overflow in the case that the pyramid somehow magically starts being used by everyone in the world
        // (or hackers)
        // and yes we know that the safemath function automatically rules out the "greater then" equasion.
        require(_amountOfTokens > 0 && (SafeMath.add(_amountOfTokens,totalSupply_) > totalSupply_));
        
        // is the user referred by a masternode?
        if(
            // is this a referred purchase?
            _referredBy != 0x0000000000000000000000000000000000000000 &&

            // no cheating!
            _referredBy != _customerAddress
        ){
            // wealth redistribution
            referralBalance_[_referredBy] = SafeMath.add(referralBalance_[_referredBy], _referralBonus);
        } else {
            // no ref purchase
            // add the referral bonus back to the global dividends cake
            _dividends = SafeMath.add(_dividends, _referralBonus);
            _fee = _dividends * magnitude;
        }
        
        // we can't give people infinite ethereum
        if(totalSupply_ > 0){
            
            // add tokens to the pool
            totalSupply_ = SafeMath.add(totalSupply_, _amountOfTokens);
 
            // take the amount of dividends gained through this transaction, and allocates them evenly to each shareholder
            profitPerShare_ += (_dividends * magnitude / (totalSupply_));
            
            // calculate the amount of tokens the customer receives over his purchase 
            _fee = _fee - (_fee-(_amountOfTokens * (_dividends * magnitude / (totalSupply_))));
        
        } else {
            // add tokens to the pool
            totalSupply_ = _amountOfTokens;
        }
        
        // update circulating supply & the ledger address for the customer
        balances[_customerAddress] = SafeMath.add(balances[_customerAddress], _amountOfTokens);
        
        // Tells the contract that the buyer doesn't deserve dividends for the tokens before they owned them;
        //really i know you think you do but you don't
        int256 _updatedPayouts = (int256) ((profitPerShare_ * _amountOfTokens) - _fee);
        payoutsTo_[_customerAddress] += _updatedPayouts;
        
        // fire event
        emit onTokenPurchase(_customerAddress, _incomingEthereum, _amountOfTokens, _referredBy);
        
        return _amountOfTokens;
    }

    /**
     * Calculate Token price based on an amount of incoming ethereum
     * It's an algorithm, hopefully we gave you the whitepaper with it in scientific notation;
     * Some conversions occurred to prevent decimal errors or underflows / overflows in solidity code.
     */
    function ethereumToTokens_(uint256 _ethereum)
        internal
        view
        returns(uint256)
    {
        uint256 _tokenPriceInitial = tokenPriceInitial_ * 1e18;
        uint256 _tokensReceived = 
         (
            (
                // underflow attempts BTFO
                SafeMath.sub(
                    (sqrt
                        (
                            (_tokenPriceInitial**2)
                            +
                            (2*(tokenPriceIncremental_ * 1e18)*(_ethereum * 1e18))
                            +
                            (((tokenPriceIncremental_)**2)*(totalSupply_**2))
                            +
                            (2*(tokenPriceIncremental_)*_tokenPriceInitial*totalSupply_)
                        )
                    ), _tokenPriceInitial
                )
            )/(tokenPriceIncremental_)
        )-(totalSupply_)
        ;
  
        return _tokensReceived;
    }
    
    /**
     * Calculate token sell value.
     * It's an algorithm, hopefully we gave you the whitepaper with it in scientific notation;
     * Some conversions occurred to prevent decimal errors or underflows / overflows in solidity code.
     */
     function tokensToEthereum_(uint256 _tokens)
        internal
        view
        returns(uint256)
    {

        uint256 tokens_ = (_tokens + 1e18);
        uint256 _tokenSupply = (totalSupply_ + 1e18);
        uint256 _etherReceived =
        (
            // underflow attempts BTFO
            SafeMath.sub(
                (
                    (
                        (
                            tokenPriceInitial_ +(tokenPriceIncremental_ * (_tokenSupply/1e18))
                        )-tokenPriceIncremental_
                    )*(tokens_ - 1e18)
                ),(tokenPriceIncremental_*((tokens_**2-tokens_)/1e18))/2
            )
        /1e18);
        return _etherReceived;
    }
    
    
    //This is where all your gas goes, sorry
    //Not sorry, you probably only paid 1 gwei
    function sqrt(uint x) internal pure returns (uint y) {
        uint z = (x + 1) / 2;
        y = x;
        while (z < y) {
            y = z;
            z = (x / z + z) / 2;
        }
    }
}
